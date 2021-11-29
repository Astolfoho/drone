#include <Arduino.h>
#include "wifi_config.h"



ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void logd(const char *str)
{
  Serial.println(str);
}

void setup()
{
  Serial.begin(115200);
  init_wifi();
  logd("awsome!!!");

  server.begin();	
 // WiFiServer server(80);
}

void loop()
{

  

}

