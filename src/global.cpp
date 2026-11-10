#include "global.h"
 
float glob_temperature = 0;
float glob_humidity    = 0;
 
String ssid     = "ESP32-Webserver";  // tên AP riêng!
String password = "12345678";      // >= 8 ký tự
 
String wifi_ssid     = "";         // gán bởi /connect
String wifi_password = "";
 
bool isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = NULL;
