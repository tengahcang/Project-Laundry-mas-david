#include "http_manager.h"
#include "config.h"
#include "wifi_manager.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
// #include <ETH.h>
// #include <WebServer_ESP32_W5500.h>
// extern ESP32_W5500 ETH;

// #include <ETH.h>


bool testKey()
{
    // 1. Perbaikan Logika: JIKA tidak ada Ethernet DAN tidak ada WiFi, BARU gagal.
    if (!checkEthernet() && WiFi.status() != WL_CONNECTED) {
        Serial.println("❌ Tidak ada koneksi internet (LAN/WiFi)");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure(); // Karena pakai HTTPS 443

    Serial.println("📤 Menghubungkan ke: " + SERVER_HOST);

    // Tambahkan timeout agar tidak gantung jika server down
    client.setTimeout(5000); 

    if (!client.connect(SERVER_HOST.c_str(), 443)) {
        Serial.println("❌ Koneksi ke Server Gagal (Handshake error/Timeout)");
        return false;
    }

    // Buat Payload JSON
    StaticJsonDocument<200> doc;
    doc["key"] = DEVICE_KEY;
    doc["perusahaan"] = COMPANY_KEY;
    doc["code"] = CODE_KEY;
    doc["machine"] = lastSavedString;

    String payload;
    serializeJson(doc, payload);

    // 2. Kirim Request
    client.println("POST " + SERVER_PATH + " HTTP/1.1");
    client.println("Host: " + SERVER_HOST);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println("Connection: close");
    client.println();
    client.println(payload);

    // 3. Baca Response secara lebih teliti
    String responseBody = "";
    unsigned long timeout = millis();
    
    // Tunggu sampai ada data yang masuk (max 5 detik)
    while (client.connected() && !client.available()) {
        if (millis() - timeout > 5000) break;
        delay(10);
    }

    // Lewati Header, ambil Body
    while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") { // Header selesai ditandai baris kosong
            responseBody = client.readString(); // Ambil sisanya (Body JSON)
            break;
        }
    }
    client.stop();

    Serial.println("📥 Raw Response: " + responseBody);

    // 4. Proses JSON Response
    if (responseBody != "") {
        StaticJsonDocument<1024> resDoc; // Perbesar sedikit buffer untuk respon panjang
        DeserializationError error = deserializeJson(resDoc, responseBody);
        
        if (!error) {
            String respondKey = resDoc["key"] | "";
            String actionData = resDoc["action"] | "";
            
            if (respondKey == DEVICE_KEY) {
                if (actionData != "" && actionData != "null") {
                    processAction(actionData);
                }
                return true;
            } else {
                Serial.println("⚠️ Key dari server tidak cocok");
            }
        } else {
            Serial.print("❌ JSON Parse Error: ");
            Serial.println(error.c_str());
        }
    }
    return false;
}

void processAction(String action) {
    if (action == "null" || action == "") return;

    int count = 0;
    int start = 0;
    int end = action.indexOf(';');

    while (end != -1 && count < MAX_MACHINES) {
        String val = action.substring(start, end);
        int durationMinutes = val.toInt();

        if (durationMinutes > 0) {
            laundryRoom[count].remainingTime += (durationMinutes * 60);
            laundryRoom[count].isActive = true;

            Serial.printf("➕ Mesin %d Ditambah: %d m. Total: %d s\n",
                          count + 1, durationMinutes, laundryRoom[count].remainingTime);

            // Kita tetap simpan per mesin di sini agar data order terbaru tidak hilang
            pref.begin("laundry", false);
            char key[10];
            sprintf(key, "m%d", count);
            pref.putInt(key, laundryRoom[count].remainingTime);
            pref.end();
        }
        
        count++;
        start = end + 1;
        end = action.indexOf(';', start);
    }

    // --- DI SINI TEMPATNYA ---
    activeMachineCount = count; // Update jumlah mesin yang terdeteksi

    // Hitung jumlah IC: jika 1-8 mesin = 1 IC, 9-16 = 2 IC, dst.
    numShiftRegisters = (activeMachineCount + 7) / 8;
    
    // Safety check: Minimal harus ada 1 IC yang didefinisikan
    if (numShiftRegisters < 1) numShiftRegisters = 1; 

    Serial.printf("📊 Total Mesin: %d | IC Shift Register: %d\n", activeMachineCount, numShiftRegisters);
}