#include <Arduino.h>
#include "config.h"
#include <WebServer_ESP32_W5500.h>
#include <WiFi.h>
#include "wifi_manager.h"
#include "http_manager.h"
#include "timer_manager.h"



unsigned long lastPollingTime = 0;
const unsigned long pollingInterval = 5000;

bool checkEthernet() {
    return (ETH.localIP()[0] != 0);
}

void setup()
{
    Serial.begin(115200);
    delay(500); // Beri waktu Serial stabil

    Serial.println("\n--- SYSTEM STARTING ---");

    // --- 1. RESET HARDWARE W5500 (PENTING!) ---
    pinMode(ETH_RST, OUTPUT);
    digitalWrite(ETH_RST, LOW);  // Reset aktif
    delay(200);                  // Tunggu sebentar
    digitalWrite(ETH_RST, HIGH); // Jalankan kembali
    delay(1000);                 // Beri napas W5500 untuk inisialisasi internal

    // --- 2. MULAI ETHERNET ---
    uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01};
    ETH.begin(ETH_MISO, ETH_MOSI, ETH_SCK, ETH_CS, ETH_INT, 25, 1, mac); 

    // --- 3. TUNGGU IP ETHERNET (LEBIH SABAR) ---
    Serial.print("⏳ Mencari IP Ethernet");
    unsigned long startAttempt = millis();
    while (ETH.localIP()[0] == 0 && millis() - startAttempt < 8000) { // Tunggu sampai 8 detik
        delay(500);
        Serial.print(".");
        yield(); // Beri waktu untuk sistem internal ESP32 (mencegah WDT reset)
    }
    Serial.println("");

    // --- 4. HUBUNGKAN INTERNET (HYBRID LAN/WIFI) ---
    connectToInternet(); 

    Serial.println("⏳ Menunggu stabilitas koneksi (3 detik)...");
    delay(3000); 

    // --- 5. TEST KEY KE SERVER ---
    bool ok = testKey();

    if(!ok) {
        Serial.println("❌ Server test gagal");
    } else {
        Serial.println("✅ Server Berhasil Merespon!");
    }
    
    // --- 6. LANJUT PROGRAM UTAMA ---
    pinMode(SHIFT_DATA_PIN, OUTPUT);
    pinMode(SHIFT_CLOCK_PIN, OUTPUT);
    pinMode(SHIFT_LATCH_PIN, OUTPUT);
    pinMode(SHIFT_OE_PIN, OUTPUT);
    digitalWrite(SHIFT_OE_PIN, LOW);

    loadSavedTimes();
    startTimerTask();
    Serial.println("🚀 PROGRAM UTAMA BERJALAN");
}

void loop()
{
    unsigned long currentTime = millis();

    // 1. Jalankan pemanggilan API setiap interval (misal 5 detik)
    if (currentTime - lastPollingTime >= pollingInterval) {
        lastPollingTime = currentTime;
        
        // --- LOGIKA FAILBACK ---
        // Jika sedang pakai WiFi, iseng cek apakah Ethernet dicolok lagi
        if (!isEthernetConnected && checkEthernet()) {
            connectToInternet(); // Ini akan memicu pindah ke Ethernet
        }

        if (checkEthernet() || WiFi.status() == WL_CONNECTED) {
            Serial.println("\n📡 [POLLING] Mengambil data order...");
            testKey(); 
        } else {
            Serial.println("⚠️ Koneksi Putus Total! Mencoba Reconnect...");
            connectToInternet(); 
        }
    }

    // 2. Monitor status mesin di serial setiap 5 detik
    static unsigned long lastMonitor = 0;
    if (currentTime - lastMonitor > 5000)
    {
        lastMonitor = currentTime;
        bool anyActive = false;
        
        for (int i = 0; i < activeMachineCount; i++)
        {
            if (laundryRoom[i].isActive)
            {
                if (!anyActive) Serial.println("\n--- STATUS WAKTU MESIN ---");
                Serial.printf("M%d: %d s | ", i + 1, laundryRoom[i].remainingTime);
                anyActive = true;
            }
        }
        
        if (anyActive) {
            Serial.println("\n--------------------------");
        }
    }

    // Kasih napas sedikit untuk sistem ESP32 agar tidak overheat/WDT reset
    yield();
}