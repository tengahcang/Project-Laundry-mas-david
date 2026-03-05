#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

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

#endif