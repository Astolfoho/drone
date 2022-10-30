#include <Arduino.h>
#include "wifi_config.h"
#include "AsyncWebSocket.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include "esp32-hal-ledc.h"

#include "MPU9250.h"
//#include "eeprom_utils.h"
#include <ESP32Servo.h>

ESP32PWM pwm;

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

int servo1Pin = 15;
int servo2Pin = 27;
int servo3Pin = 5;
int servo4Pin = 32;

MPU9250 mpu;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int speed = 0;

int minUs = 0;
int maxUs = 2500;

// void logd(const char *str)
// {
//   Serial.println(str);
// }

void logd(char *fmt, ...)
{
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 128, fmt, args);
  va_end(args);
  Serial.println(buf);
}

void process_command(char *command)
{
  // ws.textAll("Comando Recebido");
  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, command);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  int type = doc["type"];
  logd("tipo do comando: %i", type);
  if (type == 1)
  {
    const char *message = doc["message"];
    logd("mensagem do comando: %s", message);
  }
  else if (type == 2)
  {
    speed = doc["speed"];
  }

  ws.textAll("Comando Recebido");
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    //client connected
    logd("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    //client disconnected
    logd("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    //error was received from the other end
    logd("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    //pong message was received (in response to a ping request maybe)
    logd("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA)
  {
    //data packet
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      //the whole message is in a single frame and we got all of it's data
      logd("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
      if (info->opcode == WS_TEXT)
      {
        data[len] = 0;
        process_command((char *)data);
        //logd("%s\n", (char *)data);
      }
      else
      {
        for (size_t i = 0; i < info->len; i++)
        {
          logd("%02x ", data[i]);
        }
        logd("\n");
      }
      if (info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    }
    // else
    // {
    //   //message is comprised of multiple frames or the frame is split into multiple packets
    //   if (info->index == 0)
    //   {
    //     if (info->num == 0)
    //       logd("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
    //     logd("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
    //   }

    //   logd("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);
    //   if (info->message_opcode == WS_TEXT)
    //   {
    //     data[len] = 0;
    //     logd("%s\n", (char *)data);
    //   }
    //   else
    //   {
    //     for (size_t i = 0; i < len; i++)
    //     {
    //       logd("%02x ", data[i]);
    //     }
    //     logd("\n");
    //   }

    //   if ((info->index + len) == info->len)
    //   {
    //     logd("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
    //     if (info->final)
    //     {
    //       logd("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
    //       if (info->message_opcode == WS_TEXT)
    //         client->text("I got your text message");
    //       else
    //         client->binary("I got your binary message");
    //     }
    //   }
    // }
  }
}

void print_calibration()
{
  Serial.println("< calibration parameters >");
  Serial.println("accel bias [g]: ");
  Serial.print(mpu.getAccBiasX() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
  Serial.print(", ");
  Serial.print(mpu.getAccBiasY() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
  Serial.print(", ");
  Serial.print(mpu.getAccBiasZ() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
  Serial.println();
  Serial.println("gyro bias [deg/s]: ");
  Serial.print(mpu.getGyroBiasX() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
  Serial.print(", ");
  Serial.print(mpu.getGyroBiasY() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
  Serial.print(", ");
  Serial.print(mpu.getGyroBiasZ() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
  Serial.println();
  Serial.println("mag bias [mG]: ");
  Serial.print(mpu.getMagBiasX());
  Serial.print(", ");
  Serial.print(mpu.getMagBiasY());
  Serial.print(", ");
  Serial.print(mpu.getMagBiasZ());
  Serial.println();
  Serial.println("mag scale []: ");
  Serial.print(mpu.getMagScaleX());
  Serial.print(", ");
  Serial.print(mpu.getMagScaleY());
  Serial.print(", ");
  Serial.print(mpu.getMagScaleZ());
  Serial.println();
}

void setup()
{

  // pinMode(26, INPUT_PULLDOWN); // configure the acelerometer

  //Wire.begin(27, 13);
  Wire.begin();

  Serial.begin(115200);
  delay(2000);
  init_wifi();
  logd("awsome!!!");

  bool mpuOk = false;
  while (!mpuOk)
  {
    mpuOk = mpu.setup(0x68);

    if (!mpuOk)
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
    else
      break;

    delay(2000);
  }

  mpu.setMagneticDeclination(-21.34);
  mpu.setFilterIterations(10);

  // #if defined(ESP_PLATFORM) || defined(ESP8266)
  //     EEPROM.begin(0x80);
  // #endif

  //server.begin();
  // WiFiServer server(80);
  // setup ......
  // events.onConnect([](AsyncEventSourceClient *client)
  //                  {
  //                    if (client->lastId())
  //                    {
  //                      Serial.logd("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
  //                    }
  //                    //send event with message "hello!", id current millis
  //                    // and set reconnect delay to 1 second
  //                    client->send("hello!", NULL, millis(), 1000);
  //                  });
  // //HTTP Basic authentication
  // //events.setAuthentication("user", "pass");
  // server.addHandler(&events);

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();

  Serial.println("Accel Gyro calibration will start in 5sec.");
  Serial.println("Please leave the device still on the flat plane.");
  mpu.verbose(true);
  //delay(2000);
  //mpu.calibrateAccelGyro();

  Serial.println("Mag calibration will start in 5sec.");
  Serial.println("Please Wave device in a figure eight until done.");
  //delay(2000);
  //mpu.calibrateMag();

  print_calibration();
  mpu.verbose(false);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servo1.setPeriodHertz(60);
  servo2.setPeriodHertz(60);
  servo3.setPeriodHertz(60);
  servo4.setPeriodHertz(60);

  servo1.attach(servo1Pin, 0, maxUs);
  servo2.attach(servo2Pin, 0, maxUs);
  servo3.attach(servo3Pin, 0, maxUs);
  servo4.attach(servo4Pin, 0, maxUs);

  delay(1000);

  servo1.detach();
  servo2.detach();
  servo3.detach();
  servo4.detach();

  delay(5000);

  servo1.attach(servo1Pin, 0, maxUs);
  servo2.attach(servo2Pin, 0, maxUs);
  servo3.attach(servo3Pin, 0, maxUs);
  servo4.attach(servo4Pin, 0, maxUs);

  servo1.writeMicroseconds(maxUs);
  servo2.writeMicroseconds(maxUs);
  servo3.writeMicroseconds(maxUs);
  servo4.writeMicroseconds(maxUs);

  delay(3000);

  servo1.writeMicroseconds(500);
  servo2.writeMicroseconds(500);
  servo3.writeMicroseconds(500);
  servo4.writeMicroseconds(500);

  delay(2000);

  servo1.writeMicroseconds(0);
  servo2.writeMicroseconds(0);
  servo3.writeMicroseconds(0);
  servo4.writeMicroseconds(0);

  // save to eeprom
  // saveCalibration();

  // load from eeprom
  //  loadCalibration();
}

struct Quat
{
  float x;
  float y;
  float z;
  float w;
} quat;
struct Euler
{
  float x;
  float y;
  float z;
} euler;

struct RPY
{
  float r;
  float p;
  float y;
} rpy;

void print_roll_pitch_yaw()
{
  Serial.print("Yaw, Pitch, Roll: ");
  Serial.print(mpu.getYaw(), 2);
  Serial.print(", ");
  Serial.print(mpu.getPitch(), 2);
  Serial.print(", ");
  Serial.print(mpu.getRoll(), 2);
  Serial.print("  ");
  Serial.print("Mag : ");
  Serial.print(mpu.getMagX(), 2);
  Serial.print(", ");
  Serial.print(mpu.getMagY(), 2);
  Serial.print(", ");
  Serial.print(mpu.getMagZ(), 2);
  Serial.print(", ");
  Serial.print("lin_acc = ");
  Serial.print(mpu.getLinearAccX(), 2);
  Serial.print(", ");
  Serial.print(mpu.getLinearAccY(), 2);
  Serial.print(", ");
  Serial.println(mpu.getLinearAccZ(), 2);
}

void loop()
{
  if (mpu.update())
  {
    static uint32_t prev_ms = millis();
    if (millis() > prev_ms + 500)
    {
      //print_roll_pitch_yaw();

      quat.x = mpu.getQuaternionX();
      quat.y = mpu.getQuaternionY();
      quat.z = mpu.getQuaternionZ();
      quat.w = mpu.getQuaternionW();
      euler.x = mpu.getEulerX();
      euler.y = mpu.getEulerY();
      euler.z = mpu.getEulerY();
      rpy.r = mpu.getRoll();
      rpy.p = mpu.getPitch();
      rpy.y = mpu.getYaw();

      StaticJsonDocument<200> doc;
      doc["q_x"] = quat.x;
      doc["q_y"] = quat.y;
      doc["q_z"] = quat.z;
      doc["q_w"] = quat.w;

      doc["e_x"] = euler.x;
      doc["e_y"] = euler.y;
      doc["e_z"] = euler.z;

      doc["roll"] = rpy.r;
      doc["pitch"] = rpy.p;
      doc["yaw"] = rpy.y;

      char output[1024];
      serializeJson(doc, output);

      ws.textAll(output);

      // Serial.print(" quaternion x: ");
      // Serial.print(quat.x);
      // Serial.print(" quaternion y: ");
      // Serial.print(quat.y);
      // Serial.print(" quaternion z: ");
      // Serial.print(quat.z);
      // Serial.print(" quaternion w: ");
      // Serial.print(quat.w);
      // Serial.println();

      // Serial.print(" euler x: ");
      // Serial.print(euler.x);
      // Serial.print(" euler y: ");
      // Serial.print(euler.y);
      // Serial.print(" euler z: ");
      // Serial.print(euler.z);
      // Serial.println();

      prev_ms = millis();
    }
  }

  // for (int i = 500; i < 2500; ++i)
  // {

  //   // Write microseconds
  //   servo1.writeMicroseconds(i);
  //   servo2.writeMicroseconds(i);
  //   servo3.writeMicroseconds(i);
  //   servo4.writeMicroseconds(i);

  //   //wait for 10ms just for fun :)
  //   // changes take effect every REFRESH_INTERVAL (defined in SimpleESC.h)
  //   delay(10);
  //   Serial.println(i);
  // }

  // // Disable motor/stop generating PWM pulse
  // delay(2000);

  // servo1.writeMicroseconds(500);
  // servo2.writeMicroseconds(500);
  // servo3.writeMicroseconds(500);
  // servo4.writeMicroseconds(500);
  // delay(5000);

  // servo1.writeMicroseconds(2500);
  // servo2.writeMicroseconds(2500);
  // servo3.writeMicroseconds(2500);
  // servo4.writeMicroseconds(2500);
  // delay(1000);

  // servo1.writeMicroseconds(0);
  // servo2.writeMicroseconds(0);
  // servo3.writeMicroseconds(0);
  // servo4.writeMicroseconds(0);

  // servo1.detach();
  // servo2.detach();
  // servo3.detach();
  // servo4.detach();

  servo1.writeMicroseconds(speed);
  servo2.writeMicroseconds(speed);
  servo3.writeMicroseconds(speed);
  servo4.writeMicroseconds(speed);
  logd("speed %i", speed);

  // OscWiFi.update();
}
