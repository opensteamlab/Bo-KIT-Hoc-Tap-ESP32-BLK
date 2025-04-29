/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-async-web-server-espasyncwebserver-library/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
#define BLYNK_PRINT Serial
// You should get Auth Token in the Blynk App.
#define BLYNK_TEMPLATE_ID           "BANLINHKIEN"
#define BLYNK_TEMPLATE_NAME         "BANLINHKIEN"
char BLYNK_AUTH_TOKEN[32]   =   "";
// Import required libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SimpleKalmanFilter.h>
#include "index_html.h"
#include "data_config.h"
#include <EEPROM.h>
#include <Arduino_JSON.h>
#include "icon.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//----------------------- Khai báo 1 số biến Blynk -----------------------
bool blynkConnect = true;
BlynkTimer timer; 
// Một số Macro
#define ENABLE    1
#define DISABLE   0
// ---------------------- Khai báo cho OLED 1.3 --------------------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3C //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G oled = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define OLED_SDA      21
#define OLED_SCL      22

typedef enum {
  SCREEN0,
  SCREEN1,
  SCREEN2,
  SCREEN3,
  SCREEN4,
  SCREEN5,
  SCREEN6,
  SCREEN7,
  SCREEN8,
  SCREEN9,
  SCREEN10,
  SCREEN11,
  SCREEN12,
  SCREEN13
}SCREEN;
int screenOLED = SCREEN0;

bool enableShow1 = DISABLE;
bool enableShow2 = ENABLE;
#define SAD    0
#define NORMAL 1
#define HAPPY  2

bool autoWarning = DISABLE;

typedef enum {
  IDLE,
  CALIB,
  THRESHOLD_SETUP,
  WIFI_SETUP
} MODE;
int modeRun = IDLE;
String OLED_STRING1 = "Xin chao";
String OLED_STRING2 = "Hi my friend";
// --------------- Khai báo cảm biến khoảng cách srf04 -----------
#include <HCSR04.h>
#define SRF04_TRIG    19
#define SRF04_ECHO    18
HCSR04 ultrasonicSensor(SRF04_TRIG, SRF04_ECHO, 20, 400);
SimpleKalmanFilter srf04filter(2, 2, 0.1);
int srf04Value = 0;     // biến đo khoảng cách hiện tại
int waterValue = 0;     // biến chiều cao mực nước,   waterValue = EheightInstallSensor - srf04Value;
int EheightInstallSensorTemp = 0;      // biến tạm khoảng cách từ cảm biến đến mặt đất
int valueThresholdTemp = 0;           // biến tạm ngưỡng
bool waterWarning = DISABLE;
// Khai bao LED
#define LED           33
// Khai báo RELAY
#define RELAY         25
// Khai báo BUZZER
#define BUZZER        2
bool buzzerWarning = DISABLE;
//-------------------- Khai báo Button-----------------------
#include "mybutton.h"
#define BUTTON_DOWN_PIN   34
#define BUTTON_UP_PIN     35
#define BUTTON_SET_PIN    32

#define BUTTON1_ID  1
#define BUTTON2_ID  2
#define BUTTON3_ID  3
Button buttonSET;
Button buttonDOWN;
Button buttonUP;
void button_press_short_callback(uint8_t button_id);
void button_press_long_callback(uint8_t button_id);
//------------------------------------------------------------
TaskHandle_t TaskButton_handle      = NULL;
TaskHandle_t TaskOLEDDisplay_handle = NULL;
TaskHandle_t TaskAutoWarning_handle = NULL;
void setup(){
  Serial.begin(115200);
  // Đọc data setup từ eeprom
  EEPROM.begin(512);
  readEEPROM();
    // Khởi tạo LED
  pinMode(LED, OUTPUT);
  // Khởi tạo BUZZER
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, DISABLE);
  // Khởi tạo OLED
  oled.begin(i2c_Address, true);
  oled.setTextSize(2);
  oled.setTextColor(SH110X_WHITE);
  // Khởi tạo SRF04
  ultrasonicSensor.begin();

  // Khởi tạo nút nhấn
  pinMode(BUTTON_SET_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  button_init(&buttonSET, BUTTON_SET_PIN, BUTTON1_ID);
  button_init(&buttonUP, BUTTON_UP_PIN, BUTTON2_ID);
  button_init(&buttonDOWN,   BUTTON_DOWN_PIN,   BUTTON3_ID);
  button_pressshort_set_callback((void *)button_press_short_callback);
  button_presslong_set_callback((void *)button_press_long_callback);
  xTaskCreatePinnedToCore(TaskSRF04Sensor,     "TaskSRF04Sensor",      1024*10 ,  NULL,  10  ,  NULL  , 1 );
  xTaskCreatePinnedToCore(TaskButton,          "TaskButton" ,          1024*10 ,  NULL,  20 ,  &TaskButton_handle       , 1);
  xTaskCreatePinnedToCore(TaskOLEDDisplay,     "TaskOLEDDisplay" ,     1024*16 ,  NULL,  20 ,  &TaskOLEDDisplay_handle  , 1);
  xTaskCreatePinnedToCore(TaskBuzzerWarning,     "TaskBuzzerWarning" ,     1024*16 ,  NULL,  10 ,  NULL  , 1);
  
  // Kết nối wifi
  connectSTA();
}

void loop() {
  vTaskDelete(NULL);
}
void TaskBuzzerWarning(void *pvParameters) {
  while(1) {
    if(waterWarning == ENABLE && buzzerWarning == ENABLE) {
      Blynk.logEvent("auto_warning","Cảnh báo nước cao");
      digitalWrite(BUZZER,ENABLE);
      delay(1000);
      digitalWrite(BUZZER,DISABLE);
      delay(500);
    } else {
      digitalWrite(BUZZER,DISABLE);
      delay(2000);
    }
  }
}
//-------------------- Task đọc cảm biến srf04 ---------------
bool outRange = 0;
void TaskSRF04Sensor(void *pvParameters) {
    while(1) {
      int  distanceMeasure = ultrasonicSensor.getMedianFilterDistance(); //pass 3 measurements through median filter, better result on moving obstacles
      distanceMeasure = srf04filter.updateEstimate(distanceMeasure);
      if (distanceMeasure < 400 &&  distanceMeasure > 20) {
          outRange = 0;
          Serial.print("distanceMeasure: ");
          Serial.print(distanceMeasure, 1);
          Serial.println(F(" cm"));
          srf04Value = distanceMeasure;
          waterValue = EheightInstallSensor - srf04Value;
          if(waterValue < 0) waterValue = 0;
          Serial.print("waterValue: ");
          Serial.print(waterValue);
          Serial.println(F(" cm"));
          if(waterValue < EthresholdWarning) waterWarning = DISABLE;
          else waterWarning = ENABLE;
      }
      else {
        outRange = 1;
        Serial.println(F("SRF04 out of range"));
        waterWarning = ENABLE;
        waterValue = EheightInstallSensor;
      }   
      delay(200);
    }
}

// Xóa 1 ô hình chữ nhật từ tọa độ (x1,y1) đến (x2,y2)
void clearRectangle(int x1, int y1, int x2, int y2) {
   for(int i = y1; i < y2; i++) {
     oled.drawLine(x1, i, x2, i, 0);
   }
}

void clearOLED() {
  oled.clearDisplay();
  oled.display();
}

int countSCREEN9 = 0;
int countSCREEN1 = 0 ;

void drawWaterLevel() {
  oled.drawRect(115, 1, 10, 61, SH110X_WHITE);
  // 57  // 7
  int waterLevel = waterValue;
  if(waterLevel > EthresholdWarning) waterLevel = EthresholdWarning;
  int level = map(waterLevel, 0 , EthresholdWarning, 0, 57);
  oled.fillRect( 117, 60 - level  , 6, level , SH110X_WHITE);
}
// Task hiển thị OLED
void TaskOLEDDisplay(void *pvParameters) {
  while (1) {
      switch(screenOLED) {
        case SCREEN0: // Hiệu ứng khởi động
          for(int j = 0; j < 3; j++) {
            for(int i = 0; i < FRAME_COUNT_loadingOLED; i++) {
              oled.clearDisplay();
              oled.drawBitmap(32, 0, loadingOLED[i], FRAME_WIDTH_64, FRAME_HEIGHT_64, 1);
              oled.display();
              delay(FRAME_DELAY/4);
            }
          }
          screenOLED = SCREEN4;
          break;
        case SCREEN1:   // Hiển thị màn hình chính
          buzzerWarning = ENABLE;
          oled.clearDisplay();
          oled.setTextSize(1);
          oled.setCursor(0, 0);
          oled.print("Muc nuoc: ");
          if(waterWarning == ENABLE) {
            countSCREEN1++;
            if(countSCREEN1 == 2) {
              oled.setTextSize(2);
              oled.setCursor(0, 12);
              oled.print("      "); 
            } else if (countSCREEN1 == 4) {
              if(outRange == 1) {
                oled.setTextSize(2);
                oled.setCursor(0, 12);
                oled.print("NGUY HIEM"); 
               } else {
                 oled.setTextSize(2);
                 oled.setCursor(0, 12);
                 oled.print(waterValue); 
                 oled.print("/"); 
                 oled.print(EheightInstallSensor); 
                 oled.setTextSize(1);
                 oled.print(" cm");  
               }
             
            } else if (countSCREEN1 == 5) {
              countSCREEN1 = 0;
            }
            
          } else {
            
            oled.setTextSize(2);
            oled.setCursor(0, 12);
            oled.print(waterValue); 
            oled.print("/"); 
            oled.print(EheightInstallSensor); 
            oled.setTextSize(1);
            oled.print(" cm"); 
            countSCREEN1 = 0;
          }
          oled.setTextSize(1);
          oled.setCursor(0, 34);
          oled.print("Nguong canh bao: ");
          oled.setTextSize(2);
          oled.setCursor(0, 46);
          oled.print(EthresholdWarning); 
          oled.setTextSize(1);
          oled.print(" cm"); 

          drawWaterLevel();
          oled.display();
          delay(200);
          break;
        case SCREEN2:  
          buzzerWarning = DISABLE;
          oled.clearDisplay();
          oled.setTextSize(1);
          oled.setCursor(40, 20);
          oled.print(OLED_STRING1);
          oled.setCursor(40, 32);
          oled.print(OLED_STRING2);
          oled.drawBitmap(0, 16, setting2OLED[0], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
          oled.display();
          delay(100);
          break;
        case SCREEN3:  
          
          delay(100);
          break; 
        case SCREEN4:    // Đang kết nối Wifi
          oled.clearDisplay();
          oled.setTextSize(1);
          oled.setCursor(40, 5);
          oled.print("WIFI");
          oled.setTextSize(1.5);
          oled.setCursor(40, 17);
          oled.print("Dang ket noi..");
      
          for(int i = 0; i < FRAME_COUNT_wifiOLED; i++) {
            clearRectangle(0, 0, 32, 32);
            oled.drawBitmap(0, 0, wifiOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
            oled.display();
            delay(FRAME_DELAY);
          }
          break;
        case SCREEN5:    // Kết nối wifi thất bại
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 5);
            oled.print("WIFI");
            oled.setTextSize(1.5);
            oled.setCursor(40, 17);
            oled.print("Mat ket noi.");
            oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1 ], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
            oled.drawLine(31, 0 , 0, 31 , 1);
            oled.drawLine(32, 0 , 0, 32 , 1);
            oled.display();
            delay(2000);
            screenOLED = SCREEN9;
          break;
        case SCREEN6:   // Đã kết nối Wifi, đang kết nối Blynk
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 5);
            oled.print("WIFI");
            oled.setTextSize(1.5);
            oled.setCursor(40, 17);
            oled.print("Da ket noi.");
            oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1 ], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);

            oled.setTextSize(1);
            oled.setCursor(40, 34);
            oled.print("BLYNK");
            oled.setTextSize(1.5);
            oled.setCursor(40, 51);
            oled.print("Dang ket noi..");
                        

            for(int i = 0; i < FRAME_COUNT_blynkOLED; i++) {
              clearRectangle(0, 32, 32, 64);
              oled.drawBitmap(0, 32, blynkOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
              oled.display();
              delay(FRAME_DELAY);
            }

          break;
        case SCREEN7:   // Đã kết nối Wifi, Đã kết nối Blynk
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 5);
            oled.print("WIFI");
            oled.setTextSize(1.5);
            oled.setCursor(40, 17);
            oled.print("Da ket noi.");
            oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1 ], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);

            oled.setTextSize(1);
            oled.setCursor(40, 34);
            oled.print("BLYNK");
            oled.setTextSize(1.5);
            oled.setCursor(40, 51);
            oled.print("Da ket noi.");
            oled.drawBitmap(0, 32, blynkOLED[FRAME_COUNT_wifiOLED/2], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
            oled.display();
            delay(2000);
            screenOLED = SCREEN1;
            enableShow1 = ENABLE;
            buzzerWarning = ENABLE;
          break;
        case SCREEN8:   // Đã kết nối Wifi, Mat kết nối Blynk
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 5);
            oled.print("WIFI");
            oled.setTextSize(1.5);
            oled.setCursor(40, 17);
            oled.print("Da ket noi.");
            oled.drawBitmap(0, 0, wifiOLED[FRAME_COUNT_wifiOLED - 1 ], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);

            oled.setTextSize(1);
            oled.setCursor(40, 34);
            oled.print("BLYNK");
            oled.setTextSize(1.5);
            oled.setCursor(40, 51);
            oled.print("Mat ket noi.");
            oled.drawBitmap(0, 32, blynkOLED[FRAME_COUNT_wifiOLED/2], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
            oled.drawLine(31, 32 , 0, 63 , 1);
            oled.drawLine(32, 32 , 0, 64 , 1);
            oled.display();
            delay(2000);
            screenOLED = SCREEN9;
          break;
        case SCREEN9:   // Cai đặt 192.168.4.1
            buzzerWarning = DISABLE;
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 5);
            oled.setTextSize(1);
            oled.print("Ket noi Wifi:");
            oled.setCursor(40, 17);
            oled.setTextSize(1);
            oled.print("ESP32_IOT");

            oled.setCursor(40, 38);
            oled.print("Dia chi IP:");
    
            oled.setCursor(40, 50);
            oled.print("192.168.4.1");

            for(int i = 0; i < FRAME_COUNT_settingOLED; i++) {
              clearRectangle(0, 0, 32, 64);
              oled.drawBitmap(0, 16, settingOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
              oled.display();
              delay(FRAME_DELAY*2);
            }
            countSCREEN9++;
            if(countSCREEN9 > 10) {
              countSCREEN9 = 0;
              screenOLED = SCREEN1;
              enableShow1 = ENABLE;
              buzzerWarning = ENABLE;
            }
 
            break;
          case SCREEN10:    // auto : on
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 20);
            oled.print("Canh bao:");
            oled.setTextSize(2);
            oled.setCursor(40, 32);
            oled.print("DISABLE"); 
            for(int i = 0; i < FRAME_COUNT_autoOnOLED; i++) {
              clearRectangle(0, 0, 32, 64);
              oled.drawBitmap(0, 16, autoOnOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
              oled.display();
              delay(FRAME_DELAY);
            }
            clearRectangle(40, 32, 128, 64);
            oled.setCursor(40, 32);
            oled.print("ENABLE"); 
            oled.display();   
            delay(2000);
            screenOLED = SCREEN1;
            enableShow1 = ENABLE;
            buzzerWarning = DISABLE;
            break;
          case SCREEN11:     // auto : off
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 20);
            oled.print("Canh bao:");
            oled.setTextSize(2);
            oled.setCursor(40, 32);
            oled.print("ENABLE");
            for(int i = 0; i < FRAME_COUNT_autoOffOLED; i++) {
              clearRectangle(0, 0, 32, 64);
              oled.drawBitmap(0, 16, autoOffOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
              oled.display();
              delay(FRAME_DELAY);
            }
            clearRectangle(40, 32, 128, 64);
            oled.setCursor(40, 32);
            oled.print("DISABLE"); 
            oled.display();    
            delay(2000);
            screenOLED = SCREEN1;  
            enableShow1 = ENABLE;
            buzzerWarning = DISABLE;
            break;
          case SCREEN12:  // gui du lieu len blynk
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(40, 20);
            oled.print("Gui du lieu");
            oled.setCursor(40, 32);
            oled.print("den BLYNK"); 
            for(int i = 0; i < FRAME_COUNT_sendDataOLED; i++) {
                clearRectangle(0, 0, 32, 64);
                oled.drawBitmap(0, 16, sendDataOLED[i], FRAME_WIDTH_32, FRAME_HEIGHT_32, 1);
                oled.display();
                delay(FRAME_DELAY);
            } 
            delay(1000);
            screenOLED = SCREEN1; 
            enableShow1 = ENABLE;
            break;
          case SCREEN13:   // khoi dong lai
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(0, 20);
            oled.print("Khoi dong lai");
            oled.setCursor(0, 32);
            oled.print("Vui long doi ..."); 
            oled.display();
            break;
          default : 
            delay(500);
            break;
      } 
      delay(10);
  }
}



//-----------------Kết nối STA wifi, chuyển sang wifi AP nếu kết nối thất bại ----------------------- 
void connectSTA() {
      delay(5000);
      enableShow1 = DISABLE;
      if ( Essid.length() > 1 ) {  
      Serial.println(Essid);        //Print SSID
      Serial.println(Epass);        //Print Password
      Serial.println(Etoken);        //Print token
      Etoken = Etoken.c_str();
      WiFi.begin(Essid.c_str(), Epass.c_str());   //c_str()
      int countConnect = 0;
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);   
          if(countConnect++  == 15) {
            Serial.println("Ket noi Wifi that bai");
            Serial.println("Kiem tra SSID & PASS");
            Serial.println("Ket noi Wifi: ESP32 de cau hinh");
            Serial.println("IP: 192.168.4.1");
            screenOLED = SCREEN5;
            digitalWrite(BUZZER, ENABLE);
            delay(2000);
            digitalWrite(BUZZER, DISABLE);
            delay(3000);
            break;
          }
          // MODE đang kết nối wifi
          screenOLED = SCREEN4;
          delay(2000);
      }
      Serial.println("");
      if(WiFi.status() == WL_CONNECTED) {
        Serial.println("Da ket noi Wifi: ");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP()); 
        Serial.println((char*)Essid.c_str());

       // MODE wifi đã kết nối, đang kết nối blynk
       screenOLED = SCREEN6;
       delay(2000);
        strcpy(BLYNK_AUTH_TOKEN,Etoken.c_str());
        
        Blynk.config(BLYNK_AUTH_TOKEN);
        blynkConnect = Blynk.connect();
        if(blynkConnect == false) {
            screenOLED = SCREEN8;
            delay(2000);
            connectAPMode(); 
        }
        else {
            Serial.println("Da ket noi BLYNK");
            enableShow1 = ENABLE;
            // MODE đã kết nối wifi, đã kết nối blynk
            screenOLED = SCREEN7;
            delay(2000);
            xTaskCreatePinnedToCore(TaskBlynk,            "TaskBlynk" ,           1024*16 ,  NULL,  20  ,  NULL ,  1);
            timer.setInterval(1000L, myTimer);  
            buzzerBeep(5);  
            return; 
        }
      }
      else {
        digitalWrite(BUZZER, ENABLE);
        delay(2000);
        digitalWrite(BUZZER, DISABLE);
        // MODE truy cập vào 192.168.4.1
        screenOLED = SCREEN9;
        connectAPMode(); 
      }
        
    }
}


//--------------------------- switch AP Mode --------------------------- 
void connectAPMode() {

  // Khởi tạo Wifi AP Mode, vui lòng kết nối wifi ESP32, truy cập 192.168.4.1
  WiFi.softAP(ssidAP, passwordAP);  

  // Gửi trang HTML khi client truy cập 192.168.4.1
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Gửi data ban đầu đến clientgetDataFromClient
  server.on("/data_before", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getJsonData();
    request->send(200, "application/json", json);
  });

  // Get data từ client
  server.on("/post_data", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "SUCCESS");
    enableShow1 = DISABLE;
    screenOLED = SCREEN13;
    delay(5000);
    ESP.restart();
  }, NULL, getDataFromClient);

  // Start server
  server.begin();
}

//------------------- Hàm đọc data từ client gửi từ HTTP_POST "/post_data" -------------------
void getDataFromClient(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  Serial.print("get data : ");
  Serial.println((char *)data);
  JSONVar myObject = JSON.parse((char *)data);
  if(myObject.hasOwnProperty("ssid"))
    Essid = (const char*) myObject["ssid"];
  if(myObject.hasOwnProperty("pass"))
    Epass = (const char*)myObject["pass"] ;
  if(myObject.hasOwnProperty("token"))
    Etoken = (const char*)myObject["token"];
  if(myObject.hasOwnProperty("heightInstallSensor"))
    EheightInstallSensor = (int) myObject["heightInstallSensor"];
  if(myObject.hasOwnProperty("thresholdWarning")) 
    EthresholdWarning = (int) myObject["thresholdWarning"];

  writeEEPROM();
  

}

// ------------ Hàm in các giá trị cài đặt ------------
void printValueSetup() {
    Serial.print("ssid = ");
    Serial.println(Essid);
    Serial.print("pass = ");
    Serial.println(Epass);
    Serial.print("token = ");
    Serial.println(Etoken);

    Serial.print("heightInstallSensor = ");
    Serial.println(EheightInstallSensor);

    Serial.print("thresholdWarning = ");
    Serial.println(EthresholdWarning);
   
    Serial.print("autoWarning = ");
    Serial.println(autoWarning);
}

//-------- Hàm tạo biến JSON để gửi đi khi có request HTTP_GET "/" --------
String getJsonData() {
  JSONVar myObject;
  myObject["ssid"]  = Essid;
  myObject["pass"]  = Epass;
  myObject["token"] = Etoken;
  myObject["heightInstallSensor"] = EheightInstallSensor;
  myObject["thresholdWarning"] = EthresholdWarning;


  String jsonData = JSON.stringify(myObject);
  return jsonData;
}

//-------------------------------------------------------------------------------
//--------------------------------Task Blynk-------------------------------------



//----------------------- Send send Data value to Blynk every 2 seconds--------
void myTimer() {
    Blynk.virtualWrite(V0, waterValue);
    Blynk.virtualWrite(V2, autoWarning); 
    Blynk.virtualWrite(V1, EthresholdWarning);
}
BLYNK_WRITE(V1) {
    int thresholdBlynk = param.asInt();
    if(thresholdBlynk < EheightInstallSensor)  {
      EthresholdWarning = thresholdBlynk;
      //ghi vao flash
      writeThresHoldEEPROM(EthresholdWarning);
    }
    else {
      delay(500);
      Blynk.virtualWrite(V1, EthresholdWarning); 
    }
    delay(500);
}
//------------------------- check autoWarning from BLYNK  -----------------------
BLYNK_WRITE(V2) {
    enableShow1 = DISABLE;
    autoWarning = param.asInt();
    buzzerBeep(1);
    EEPROM.write(210, autoWarning);  EEPROM.commit();
    if(autoWarning == 0) screenOLED = SCREEN11;
    else screenOLED = SCREEN10;
}

//---------------------------Task TaskSwitchAPtoSTA---------------------------
void TaskBlynk(void *pvParameters) {
    while(1) {
      Blynk.run();
      timer.run(); 
      delay(10);
    }
}

/*
 * Các hàm liên quan đến lưu dữ liệu cài đặt vào EEPROM
*/
//--------------------------- Read Eeprom  --------------------------------
void readEEPROM() {
    for (int i = 0; i < 32; ++i)       //Reading SSID
        Essid += char(EEPROM.read(i)); 
    for (int i = 32; i < 64; ++i)      //Reading Password
        Epass += char(EEPROM.read(i)); 
    for (int i = 64; i < 96; ++i)      //Reading Password
        Etoken += char(EEPROM.read(i)); 

    EheightInstallSensor = EEPROM.read(200) * 100 + EEPROM.read(201);
    EthresholdWarning    = EEPROM.read(202) * 100 + EEPROM.read(203);  

    autoWarning     = EEPROM.read(210);

    printValueSetup();
}

// ------------------------ Clear Eeprom ------------------------

void clearEeprom() {
    Serial.println("Clearing Eeprom");
    for (int i = 0; i < 250; ++i) 
      EEPROM.write(i, 0);
}

// -------------------- Hàm ghi data vào EEPROM ------------------
void writeEEPROM() {
    clearEeprom();
    for (int i = 0; i < Essid.length(); ++i)
          EEPROM.write(i, Essid[i]);  
    for (int i = 0; i < Epass.length(); ++i)
          EEPROM.write(32+i, Epass[i]);
    for (int i = 0; i < Etoken.length(); ++i)
          EEPROM.write(64+i, Etoken[i]);
    if(Essid.length() == 0) Essid = "BLK";

    EEPROM.write(200, EheightInstallSensor / 100);      // lưu hàng nghìn + trăm chiều cao lắp cảm biến
    EEPROM.write(201, EheightInstallSensor % 100);      // lưu hàng chục + đơn vị chiều cao lắp cảm biến

    EEPROM.write(202, EthresholdWarning / 100);      // lưu hàng nghìn + trăm bụi ngưỡng
    EEPROM.write(203, EthresholdWarning % 100);      // lưu hàng chục + đơn vị ngưỡng
    
    EEPROM.commit();

    Serial.println("write eeprom");
    delay(500);
}

// -------- Hàm lưu threshold vào EEPROM , nếu giá trị waterValue > thresshold thì sẽ báo động -----------------------
void writeThresHoldEEPROM(int thresshold)
{
    int firstTwoDigits = thresshold / 100;  // lấy 2 số hàng nghìn và trăm
    int lastTwoDigits  = thresshold % 100;  // lấy 2 số hàng chục và đơn vị
    EEPROM.write(202, firstTwoDigits);         // lưu 2 số hàng nghìn và trăm vào flash
    EEPROM.write(203, lastTwoDigits);          // lưu 2 số hàng chục và đơn vị vào flash
    EEPROM.commit();
    Serial.print("thresshold");
    Serial.println(thresshold);
}
// ---------Hàm lưu Calib vào EEPROM, đây là giá trị từ cảm biến đến mặt đất -----------------------
void writeCalibEEPROM(int calibValue)
{
    int firstTwoDigits = calibValue / 100;  // lấy 2 số hàng nghìn và trăm
    int lastTwoDigits  = calibValue % 100;  // lấy 2 số hàng chục và đơn vị
    EEPROM.write(200, firstTwoDigits);         // lưu 2 số hàng nghìn và trăm vào flash
    EEPROM.write(201, lastTwoDigits);          // lưu 2 số hàng chục và đơn vị vào flash
    EEPROM.commit();
    Serial.print("calibValue");
    Serial.println(calibValue);
}
//-----------------------Task Task Button ----------
void TaskButton(void *pvParameters) {
    while(1) {
      handle_button(&buttonSET);
      handle_button(&buttonUP);
      handle_button(&buttonDOWN);
      delay(10);
    }
}
void button_press_short_callback(uint8_t button_id) {
    switch(button_id) {
      case BUTTON1_ID :  
        buzzerBeep(1);
        Serial.println("btSET press short");        
        // cai dat threshold
        if(modeRun == IDLE) {
          modeRun = THRESHOLD_SETUP;
          valueThresholdTemp = EthresholdWarning; 
          // SCREEN_SHOW hiển thị thiết lập ngưỡng

          Serial.print("thiet lap nguong");  
          Serial.println(valueThresholdTemp);  
          screenOLED = SCREEN2;
          OLED_STRING1 = "Thiet lap ";
          OLED_STRING2 = "nguong : " + String(valueThresholdTemp);
        }
        else if(modeRun == THRESHOLD_SETUP) {

          EthresholdWarning = valueThresholdTemp;
          writeThresHoldEEPROM(EthresholdWarning);
          Blynk.virtualWrite(V1, EthresholdWarning);
          // SCREEN_SHOW thiết lập ngưỡng thành công

          Serial.println("thiet lap nguong thanh cong");  
          screenOLED = SCREEN2;
          OLED_STRING1 = "Thiet lap";
          OLED_STRING2 = "thanh cong";
          delay(1000);
          modeRun = IDLE;
          screenOLED = SCREEN1;
        }

        break;
      case BUTTON2_ID :
        buzzerBeep(1);
        Serial.println("btUP press short");
        if(modeRun == IDLE) {
          screenOLED = SCREEN1;
        }
        else  if(modeRun == THRESHOLD_SETUP) {
          valueThresholdTemp ++;
          if(valueThresholdTemp >= EheightInstallSensor - 20) valueThresholdTemp = EheightInstallSensor - 20;
          // SCREEN_SHOW Hiển thị giá trị valueThresholdTemp
          Serial.print("nguong : ");  
          Serial.println(valueThresholdTemp); 
          screenOLED = SCREEN2;
          OLED_STRING1 = "Thiet lap ";
          OLED_STRING2 = "nguong : " + String(valueThresholdTemp);
        }
         else if (modeRun == CALIB) {
          if(srf04Value < 50) {
            Serial.println("lap dat sensor qua thap <40cm");
            // SCREEN_SHOW Lắp cảm biến quá thấp
            screenOLED = SCREEN2;
            OLED_STRING1 = "Lap dat sensor";
            OLED_STRING2 = "qua thap < 40cm";
            delay(2000);
          }
          else if (srf04Value >=40 && srf04Value < 400) {
            EheightInstallSensorTemp = srf04Value;
            // SCREEN_SHOW Lắp cảm biến hợp lí

            Serial.println("lap dat sensor hop li");  
            Serial.println(srf04Value);  
            screenOLED = SCREEN2;
            OLED_STRING1 = "Lap dat sensor";
            OLED_STRING2 = "OK : " + String(EheightInstallSensorTemp);
            delay(2000);
          }
          else {
            Serial.println("SRF04 out of range");
            // SCREEN_SHOW Không thể đo được

            Serial.println("khong the do duoc");  
            screenOLED = SCREEN2;
            OLED_STRING1 = "Khong the";
            OLED_STRING2 = "do duoc";
            delay(1000);
            OLED_STRING1 = "Thu lai";
            OLED_STRING2 = "";
            delay(1000);
          }
        }
        break;
      case BUTTON3_ID :
        buzzerBeep(1);
        Serial.println("btDOWN press short");
        if(modeRun == IDLE) {
          screenOLED = SCREEN1;
        }
       
        else  if(modeRun == THRESHOLD_SETUP) {
            valueThresholdTemp --;
            if(valueThresholdTemp <= 0) valueThresholdTemp = 0;
            // SCREEN_SHOW Hiển thị giá trị valueThresholdTemp
            Serial.print("nguong : ");  
            Serial.println(valueThresholdTemp); 
            screenOLED = SCREEN2;
            OLED_STRING1 = "Thiet lap";
            OLED_STRING2 = "nguong : " + String(valueThresholdTemp);
        }
        break;  
    } 
} 

//-----------------Hàm xử lí nút nhấn giữ ----------------------
void button_press_long_callback(uint8_t button_id) {
    switch(button_id) {  
      case BUTTON1_ID :  
        buzzerBeep(2);
        Serial.println("btSET press long");
        if(modeRun == IDLE) {
          buzzerBeep(2);  
            enableShow1 = DISABLE;
            Serial.println("btSET press long");
            screenOLED = SCREEN9;
            clearOLED();
            connectAPMode(); 
        }       
        break;

      case BUTTON2_ID :
        buzzerBeep(2);
        Serial.println("btUp press long");
        if(modeRun == IDLE) { 
          EheightInstallSensorTemp = EheightInstallSensor;
          modeRun = CALIB;
          screenOLED = SCREEN2;
          OLED_STRING1 = "Thiet lap";
          OLED_STRING2 = "khoang cach";
          // SCREEN_SHOW thiết lập khoảng cách
          Serial.println("thiet lap khoang cach"); 
          delay(1000);
          Serial.println("giu cam bien on dinh"); 
          OLED_STRING1 = "Giu cam bien";
          OLED_STRING2 = "on dinh";
          delay(1000);
          screenOLED = SCREEN2;
          OLED_STRING1 = "Chieu cao hien";
          OLED_STRING2 = "tai : " + String(srf04Value);
          // SCREEN_SHOW giữ cảm biến ổn định
        }
        else if(modeRun == CALIB) { 
          writeCalibEEPROM(EheightInstallSensorTemp);
          EheightInstallSensor = EheightInstallSensorTemp;
          // SCREEN_SHOW thiet lap thanh cong
          Serial.println("thiet lap thanh cong"); 
          screenOLED = SCREEN2;
          OLED_STRING1 = "Thiet lap";
          OLED_STRING2 = "thanh cong";
          delay(2000);
          modeRun = IDLE;
          screenOLED = SCREEN1;
        }

         
        break;
      case BUTTON3_ID :
      
        buzzerBeep(2);
        Serial.println("btDown press long");
        if(modeRun == IDLE) {
          enableShow1 = DISABLE;
          autoWarning = 1 - autoWarning;
          EEPROM.write(210, autoWarning);  EEPROM.commit();
          Blynk.virtualWrite(V2, autoWarning); 
          if(autoWarning == 0) screenOLED = SCREEN11;
          else screenOLED = SCREEN10;
        }
        break;  
 
   }   
}

// ---------------------- Hàm điều khiển còi -----------------------------
void buzzerBeep(int numberBeep) {
  for(int i = 0; i < numberBeep; ++i) {
    digitalWrite(BUZZER, ENABLE);
    delay(100);
    digitalWrite(BUZZER, DISABLE);
    delay(100);
  }  
}
// ---------------------- Hàm điều khiển LED -----------------------------
void blinkLED(int numberBlink) {
  for(int i = 0; i < numberBlink; ++i) {
    digitalWrite(LED, DISABLE);
    delay(300);
    digitalWrite(LED, ENABLE);
    delay(300);
  }  
}

