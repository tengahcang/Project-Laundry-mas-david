#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

// --- PINOUT SHIFT REGISTER ---
#define SHIFT_DATA_PIN  25
#define SHIFT_CLOCK_PIN 33
#define SHIFT_LATCH_PIN 32
#define SHIFT_OE_PIN    26

// --- PINOUT ETHERNET W5500 ---
#define ETH_MOSI 23
#define ETH_MISO 19
#define ETH_SCK  18
#define ETH_CS   5
#define ETH_INT  4
#define ETH_RST  2

// Variabel Global
extern String WIFI_SSID;
extern String WIFI_PASS;
extern String SERVER_HOST;
extern String SERVER_PATH;
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
extern int activeMachineCount;
extern String lastSavedString;
extern int numShiftRegisters;
extern bool isEthernetConnected;
bool checkEthernet();

#endif