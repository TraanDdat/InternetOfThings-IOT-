#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <Arduino.h>
 
// Giá trị cảm biến (do sensor task ghi)
extern float glob_temperature;
extern float glob_humidity;
 
// Thông tin AP riêng của ESP32
extern String ssid;
extern String password;
 
// Thông tin router từ trang /settings
extern String wifi_ssid;
extern String wifi_password;
 
// Trạng thái mạng dùng chung giữa các task
extern bool isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
#endif