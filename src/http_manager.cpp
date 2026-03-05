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
    payload += "\"code\":\"" + CODE_KEY + "\"";
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

            pref.begin("laundry", false);
            char key[10];
            sprintf(key, "m%d", count);
            pref.putInt(key, laundryRoom[count].remainingTime);
            pref.end();
        }
        
        // PINDAHKAN INI KE LUAR blok IF (durationMinutes > 0)
        count++;
        start = end + 1;
        end = action.indexOf(';', start);
    }
    activeMachineCount = count; // Update jumlah mesin yang terdeteksi
}