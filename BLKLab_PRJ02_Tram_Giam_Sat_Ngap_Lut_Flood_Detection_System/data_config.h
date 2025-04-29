#ifndef _MY_CONFIG_H_
#define _MY_CONFIG_H_

const char* ssidAP     = "ESP32_IOT";     // Tên wifi AP Mode
const char* passwordAP = "";          // Mật khẩu AP Mode

String  Essid   = "";                 // EEPROM tên wifi nhà bạn
String  Epass   = "";                 // EEPROM mật khẩu wifi nhà bạn
String  Etoken = "";                  // EEPROM mã token blynk
int  EheightInstallSensor = 0;          // chiều cao lắp cảm biến
int  EthresholdWarning = 0;             // ngưỡng cảnh báo

#endif