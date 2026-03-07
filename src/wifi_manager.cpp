#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
// Tambahkan ini agar compiler kenal ETH milik Mas Aldo
// #include <WebServer_ESP32_W5500.h>
// #include <ETH.h>

// extern ESP32_W5500 ETH;

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

void connectToInternet() {
    // 1. Cek apakah Ethernet sekarang dicolok/aktif
    if (checkEthernet()) {
        if (!isEthernetConnected) {
            Serial.println("\n🌐 [NETWORK] Kabel LAN Terdeteksi! Beralih ke Ethernet...");
            isEthernetConnected = true;
            
            // Matikan WiFi agar tidak boros daya dan tidak bentrok IP
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("📡 Mematikan WiFi cadangan...");
                WiFi.disconnect(true); 
                WiFi.mode(WIFI_OFF);
            }
        }
        return; // Sudah pakai Ethernet, keluar dari fungsi
    }

    // 2. Jika Ethernet tidak ada (dicabut), dan WiFi belum konek
    if (WiFi.status() != WL_CONNECTED) {
        if (isEthernetConnected) {
            Serial.println("\n⚠️ Ethernet Terputus! Mencari WiFi...");
            isEthernetConnected = false;
        }
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
        
        // Tunggu sebentar saja, jangan pakai while() agar tidak macet
        int retry = 0;
        while (WiFi.status() != WL_CONNECTED && retry < 10) {
            delay(500);
            Serial.print(".");
            retry++;
        }
    }
}