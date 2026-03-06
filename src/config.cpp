#include "config.h"

// Masukkan data kamu di sini
String WIFI_SSID = "TP-Link_Lantai 2_2.4 GHz";
String WIFI_PASS = "Mbs1234!";
String SERVER_URL = "https://mesinlaundryindonesia.co.id/lds/index.php/iot/index_post";
String DEVICE_KEY = "FFAAFFEE";
String COMPANY_KEY = "NGA";
String CODE_KEY = "M";

Preferences pref;
Machine laundryRoom[MAX_MACHINES];
int activeMachineCount = 0;
String lastSavedString = ""; // Fisik variabel global ada di sini

int numShiftRegisters = 1; // Default awal 1

bool isEthernetConnected = false;