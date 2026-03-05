#include "timer_manager.h"
#include "config.h"


void timerTask(void *pvParameters) {
    for (;;) {
        bool adaPerubahan = false;

        for (int i = 0; i < activeMachineCount; i++) {
            if (laundryRoom[i].isActive && laundryRoom[i].remainingTime > 0) {
                laundryRoom[i].remainingTime--;
                adaPerubahan = true;

                if (laundryRoom[i].remainingTime <= 0) {
                    laundryRoom[i].isActive = false;
                    Serial.printf("⏹️ Mesin %d Selesai!\n", i + 1);
                }
            }
        }

        // Simpan ke Flash setiap 10 detik sekali saja agar Flash tidak cepat rusak (wear leveling)
        static int saveCounter = 0;
        if (adaPerubahan && ++saveCounter >= 10) { 
            String dataToSave = "";
            
            // Rakit string format "09;00;00;..."
            for (int i = 0; i < activeMachineCount; i++) {
                int menit = (laundryRoom[i].remainingTime + 59) / 60; // Simpan dalam menit agar rapi
                if (menit < 10) dataToSave += "0"; // Padding nol didepan (optional)
                dataToSave += String(menit);
                dataToSave += ";";
            }

            pref.begin("laundry", false);
            pref.putString("actionStr", dataToSave); // Simpan sebagai SATU string
            pref.end();
            
            saveCounter = 0;
            Serial.print("💾 Flash Saved: ");
            Serial.println(dataToSave);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Fungsi untuk ambil data lama saat baru nyala
void loadSavedTimes() {
    pref.begin("laundry", true);
    String savedData = pref.getString("actionStr", ""); // Ambil string actionStr
    pref.end();

    if (savedData != "") {
        Serial.println("📂 Recovery Data: " + savedData);
        
        int count = 0;
        int start = 0;
        int end = savedData.indexOf(';');

        while (end != -1 && count < MAX_MACHINES) {
            String val = savedData.substring(start, end);
            int durationMinutes = val.toInt();

            if (durationMinutes > 0) {
                laundryRoom[count].remainingTime = durationMinutes * 60;
                laundryRoom[count].isActive = true;
            }
            
            count++;
            start = end + 1;
            end = savedData.indexOf(';', start);
        }
        activeMachineCount = count;
    }
}

void startTimerTask() {
    xTaskCreatePinnedToCore(
        timerTask,
        "TimerTask",
        4096,
        NULL,
        1,
        NULL,
        0
    );
}