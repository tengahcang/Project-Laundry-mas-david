#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

#define SHIFT_DATA_PIN  25
#define SHIFT_CLOCK_PIN 33
#define SHIFT_LATCH_PIN 32
#define SHIFT_OE_PIN    26

// Ethernet W5500 Pins
#define ETH_MOSI 23
#define ETH_MISO 19
#define ETH_SCK  18
#define ETH_CS   5
#define ETH_INT  4
#define ETH_RST  2

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String SERVER_URL;
extern String DEVICE_KEY;
extern String COMPANY_KEY;
extern String CODE_KEY;

const int MAX_MACHINES = 20;
struct Machine {
    int remainingTime; 
    bool isActive;
};

extern Machine laundryRoom[MAX_MACHINES];
extern Preferences pref;
extern int activeMachineCount; // TAMBAHKAN BARIS INI
extern String lastSavedString; // Tambahkan ini sebagai variabel global
extern int numShiftRegisters; // Tambahkan ini

extern bool isEthernetConnected;

#endif