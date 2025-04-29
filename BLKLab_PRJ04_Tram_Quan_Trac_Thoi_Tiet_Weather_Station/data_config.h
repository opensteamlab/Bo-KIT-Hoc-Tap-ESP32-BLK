#ifndef _MY_CONFIG_H_
#define _MY_CONFIG_H_

const char* ssidAP     = "ESP32_IOT";     // Tên wifi AP Mode
const char* passwordAP = "";          // Mật khẩu AP Mode

String  Essid   = "";          // EEPROM tên wifi nhà bạn
String  Epass   = "";                     // EEPROM mật khẩu wifi nhà bạn
String  Etoken  = "";                  // EEPROM mã token blynk
int  EtempThreshold1 = 0;             // ngưỡng nhiệt độ 1   
int  EtempThreshold2 = 0;             // ngưỡng nhiệt độ 2 

int  EhumiThreshold1 = 0;             // ngưỡng độ ẩm 1
int  EhumiThreshold2 = 0;             // ngưỡng độ ẩm 2

int  ErainThreshold1 = 0;             // ngưỡng mưa 1  
int  ErainThreshold2 = 0;             // ngưỡng mưa 2

int  EwindThreshold1 = 0;             // ngưỡng gió 1
int  EwindThreshold2 = 0;             // ngưỡng gió 2

int  ErainTimeSample = 5;             // thời gian lấy mẫu cảm biến mưa
int  EwindTimeSample = 5;             // thời gian lấy mẫu cảm biến gió

int  ErainSFunnel       = 9498;           // Đường kính phễu hứng nước mưa
int  ErainAmountOfWater = 6;           // Lượng nước mưa vừa đủ để phễu đo lật
int  EwindDAnemometer   = 208;           // Đường kính trục cánh quạt quay hứng gió
     


String snTemp[4] = {"", "Nhiệt độ thấp ","Nhiệt độ phù hợp ","Nhiệt độ cao "};
String snHumi[4] = {"", "Độ ẩm thấp ","Độ ẩm phù hợp ","Độ ẩm cao "};
String snWind[5] = {"", "Không có gió ","Gió nhẹ ","Gió vừa ","Gió lớn "};
String snRain[5] = {"", "Không mưa ","Mưa nhỏ ","Mưa vừa ","Mưa lớn "};
#endif