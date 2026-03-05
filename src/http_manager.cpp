#include "http_manager.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

bool testKey()
{
    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"key\":\"" + DEVICE_KEY + "\",";
    payload += "\"perusahaan\":\"" + COMPANY_KEY + "\",";
    payload += "\"code\":\"" + CODE_KEY + "\",";
    payload += "\"machine\":\"" + lastSavedString + "\""; // Langsung pakai variabel global
    payload += "}";

    Serial.println(payload);

    Serial.println("📤 Sending key test...");

    int httpCode = http.POST(payload);

    if (httpCode <= 0)
    {
        Serial.println("❌ HTTP Request gagal");
        http.end();
        return false;
    }

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    String response = http.getString();
    Serial.println("Response:");
    Serial.println(response);

    http.end();

    if (httpCode == 200) {
        JsonDocument doc; // Gunakan JsonDocument untuk ArduinoJson V7
        deserializeJson(doc, response);

        String respond = doc["key"] | "";
        String actionData = doc["action"] | ""; // Ambil data action dari JSON

        if (respond == DEVICE_KEY) {
            processAction(actionData); // WAJIB panggil ini agar data diproses
            return true;
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