#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

void connectWifi()
{
    Serial.println("🔄 Connecting WiFi...");
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n✅ WiFi Connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}