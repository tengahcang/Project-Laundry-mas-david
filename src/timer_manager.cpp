#include "timer_manager.h"
#include "config.h"

// Variabel untuk mencatat data terakhir yang sukses disimpan ke memori
// String lastSavedString = "";

// Tambahkan variabel global di timer_manager.cpp untuk mengingat status relay terakhir
byte lastRelayStates[3] = {0, 0, 0};

void timerTask(void *pvParameters) {
    for (;;) {
        bool adaPerubahan = false;
        String currentActionStr = "";

        // 1. Hitung mundur dan rakit string kondisi saat ini
        for (int i = 0; i < activeMachineCount; i++) {
            if (laundryRoom[i].isActive && laundryRoom[i].remainingTime > 0) {
                laundryRoom[i].remainingTime--;
                adaPerubahan = true;

                if (laundryRoom[i].remainingTime <= 0) {
                    laundryRoom[i].isActive = false;
                    Serial.printf("⏹️ Mesin %d Selesai!\n", i + 1);
                }
            }

            // Hitung pembulatan ke atas untuk string
            int menit = (laundryRoom[i].remainingTime + 59) / 60;
            if (menit < 10) currentActionStr += "0";
            currentActionStr += String(menit);
            currentActionStr += ";";
        }

        // 2. LOGIKA OPSI 4: Bandingkan data saat ini dengan data terakhir di Flash
        // Kita cek setiap 1 detik, tapi HANYA menulis jika string-nya berubah (ganti menit)
        if (currentActionStr != lastSavedString) {
            pref.begin("laundry", false);
            pref.putString("actionStr", currentActionStr);
            pref.end();

            lastSavedString = currentActionStr; // Update catatan terakhir
            Serial.print("💾 Flash Updated (Minute Change): ");
            Serial.println(currentActionStr);
        }

        updateRelays();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Fungsi untuk ambil data lama saat baru nyala
void loadSavedTimes() {
    pref.begin("laundry", true);
    String savedData = pref.getString("actionStr", ""); 
    pref.end();

    if (savedData != "") {
        // --- TAMBAHKAN BARIS INI ---
        lastSavedString = savedData; 
        // ---------------------------
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

void updateRelays() {
    byte currentRelayStates[3] = {0, 0, 0};
    byte pulseData[3] = {0, 0, 0};

    // 1. Tentukan status mesin yang SEHARUSNYA aktif
    for (int i = 0; i < activeMachineCount; i++) {
        if (laundryRoom[i].isActive) {
            int byteIdx = i / 8; 
            int bitIdx = i % 8;  
            bitSet(currentRelayStates[byteIdx], bitIdx);
        }
    }

    // 2. LOGIKA HANYA NYALA: Bandingkan status baru dengan status lama
    bool adaYangPerluDinyalakan = false;
    for (int j = 0; j < numShiftRegisters; j++) {
        // Logika (A & ~B): Cari bit yang di status baru adalah 1, tapi di status lama adalah 0
        pulseData[j] = currentRelayStates[j] & ~lastRelayStates[j]; 
        
        if (pulseData[j] > 0) adaYangPerluDinyalakan = true;
    }

    // 3. Jika ada mesin yang baru saja aktif (pindah dari 0 ke 1)
    if (adaYangPerluDinyalakan) {
        Serial.println("⚡ Triggering pulse HANYA untuk mesin yang baru AKTIF...");

        digitalWrite(SHIFT_LATCH_PIN, LOW);
        for (int i = numShiftRegisters - 1; i >= 0; i--) {
            shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, pulseData[i]);
        }
        digitalWrite(SHIFT_LATCH_PIN, HIGH);

        delay(50); // Pulse 50ms sesuai instruksi Mas David

        // Matikan semua output (release coil)
        digitalWrite(SHIFT_LATCH_PIN, LOW);
        for (int i = numShiftRegisters - 1; i >= 0; i--) {
            shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, 0x00);
        }
        digitalWrite(SHIFT_LATCH_PIN, HIGH);
        
        Serial.println("✅ Trigger selesai.");
    }
    
    // PENTING: Selalu update lastRelayStates agar sinkron dengan kondisi mesin saat ini
    // meskipun kita tidak mengirimkan pulse saat mesin mati
    for (int k = 0; k < 3; k++) {
        lastRelayStates[k] = currentRelayStates[k];
    }
}