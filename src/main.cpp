#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "wifi_manager.h"
#include "http_manager.h"
#include "timer_manager.h"

unsigned long lastPollingTime = 0;
const unsigned long pollingInterval = 5000;

void setup()
{
    // WAJIB: Tambahkan ini di baris pertama setup
    Serial.begin(115200);
    while (!Serial); // Tunggu Serial siap
    
    Serial.println("\n--- SYSTEM STARTING ---");

    connectWifi();

    bool ok = testKey();

    if(!ok)
        Serial.println("❌ Server test gagal");
    
    Serial.println("🚀 SIAP LANJUT PROGRAM UTAMA");
    loadSavedTimes();
    startTimerTask();

}

void loop()
{
    // Biarkan kosong dulu untuk testing
    unsigned long currentTime = millis();

    // Jalankan pemanggilan API setiap interval tertentu
    if (currentTime - lastPollingTime >= pollingInterval)
    {
        lastPollingTime = currentTime;
        
        if (WiFi.status() == WL_CONNECTED)
        {
            testKey(); // Sekarang memanggil API terus-menerus
        }
        else
        {
            Serial.println("⚠️ WiFi Disconnected, trying to reconnect...");
            WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
        }
    }

    // Monitor status mesin di serial setiap 5 detik (hanya jika ada yang aktif)
    static unsigned long lastMonitor = 0;
    if (currentTime - lastMonitor > 5000)
    {
        bool anyActive = false;
        for (int i = 0; i < activeMachineCount; i++)
        {
            if (laundryRoom[i].isActive)
            {
                if (!anyActive) Serial.println("\n--- Status Mesin Aktif ---");
                Serial.printf("M%d: %d s | ", i + 1, laundryRoom[i].remainingTime);
                anyActive = true;
            }
        }
        if (anyActive) Serial.println("\n--------------------------");
        lastMonitor = currentTime;
    }
}