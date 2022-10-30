#include <WiFi.h>


const char *ssid = "BATCAVERNA";
const char *password = "39761984";

void init_wifi()
{
    WiFi.mode(WIFI_MODE_STA);

    Serial.println();
    Serial.print("Conectando-se a ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    Serial.println("WiFi conectada.");
    Serial.println("Endereço de IP: ");
    Serial.println(WiFi.localIP());


    


}
