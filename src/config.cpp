#include "config.h"

// Pengaturan Koneksi WiFi
String WIFI_SSID = "TP-Link_Lantai 2_2.4 GHz";
String WIFI_PASS = "Mbs1234!";

// Pengaturan Server (Dipisah Host & Path)
String SERVER_HOST = "www.mesinlaundryindonesia.co.id";
String SERVER_PATH = "/lds/index.php/iot/index_post";

// Kredensial IOT
String DEVICE_KEY = "FFAAFFEE";
String COMPANY_KEY = "NGA";
String CODE_KEY = "M";


// Status Sistem & Memori
Preferences pref;
Machine laundryRoom[MAX_MACHINES];
int activeMachineCount = 0;
String lastSavedString = ""; // Fisik variabel global ada di sini

// Hardware Output
int numShiftRegisters = 1; // Default awal 1

// Status Jaringan
bool isEthernetConnected = false;