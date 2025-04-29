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

bool enableShow = DISABLE;

#define SAD    0
#define NORMAL 1
#define HAPPY  2
int warningTempState = SAD;
int warningHumiState = NORMAL;
int warningRainState = HAPPY;


bool autoWarning = DISABLE;
// --------------------- Cảm biến DHT11 ---------------------
#include "DHT.h"
#define DHT11_PIN         26
#define DHTTYPE DHT11
DHT dht(DHT11_PIN, DHTTYPE);
float tempValue = 30;
float humiValue   = 60;
SimpleKalmanFilter tempfilter(2, 2, 0.001);
SimpleKalmanFilter humifilter(2, 2, 0.001);
bool dht11ReadOK = true;
// Khai bao LED
#define LED           33
// Khai báo BUZZER
#define BUZZER        2
uint32_t timeCountBuzzerWarning = 0;
#define TIME_BUZZER_WARNING     300  //thời gian cảnh báo bằng còi (đơn vị giây)
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
TaskHandle_t TaskDHT11_handle = NULL;
TaskHandle_t TaskAutoWarning_handle = NULL;

// --------------------------- Khai báo cảm biến mưa ----------------------------------------
#define rainSensor    15
#define RAIN_ON   0
#define RAIN_OFF  1
int rainCountPulse = 0;                      //  biến đếm xung cb mưa
int  R_Funnel = 0;                           // đơn vị mm, đây là kích thước bán kính của phễu hứng nước mưa, cần tùy chỉnh theo đúng mô hình thực tế
int  V_AmountOfWater = 0 ;                   // đơn vị ml, đây là lượng nước vừa đủ để bập bênh nước lật được, cần căn chỉnh theo thực tế
int  S_Funnel = 0 ;                          // Diện tích mặc đón nước của phễu = pi*r^2
float onePulseValue = 0;                     // với 6ml rót vào ô chứa thì nước sẽ đổ (1 xung)=> tương đương với lượng mưa 0.63135mm, tham khảo cách tính ở đây https://smartsolutions4home.com/ss4h-rg-rain-gauge/
uint32_t timeMillisRain   = 0; 
uint32_t timeCalulateRain = 0; 

float rainValue = 0;                // cường độ mưa mm/h, ở đây chúng ta đo lượng mưa tức thời để biết được hiện tại đang mưa to hay mưa nhỏ
float rainValueMax = 0;
                                     // 1 kiểu đo khác là đo tổng lượng mưa, ví dụ như tổng lượng mưa trong 1 giờ, 12h, 24h

/*
* Tính toán 1 chút để có được kết quả cường độ mưa
* Ta có mô hình với các thông số như sau:
  Bán kính phễu :55mm
  Lượng nước mưa cần thiết để gàu đo lật : 6ml
* Với mô hình như trên, diện tích của mô hình là S_Funnel = 3.14*55*55= 9498.5 mm2
* 1m2 = 1000000mm2 => tỉ lệ  k= 1000000/9498.5 = 105.28
* Như vậy 1 xung tương đương với A = k*V_AmountOfWater = 105.28*6 = 631.68 ml/m2 = 0.63168 l/m2 
* Đơn vị đo mưa được tính bằng mm có nghĩa là trên 1 đơn vị diện tích có 1 lít nước mưa rơi xuống hoặc trên đơn vị diện tích đó lớp nước mưa có bề dày 1mm. 
* Khi ta nghe bản tin dự báo thời tiết có phần lượng mưa tại một nơi nào đó là 1.0mm thì có nghĩa là ở nơi đó trên 1m2 diện tích có 1l mưa rơi xuống.
* Như vậy mỗi khi có 6ml nước chảy vào phễu, tương đương với việc ta đọc được 1 xung, cũng tương đương với việc lượng mưa đo được là  0.63168 l/m2 = 0.63168mm
*
* Ví dụ ta tiến hành đo trong 10 phút
* Số xung đo được : 20
* => Lượng nước đo được = 20 * A = 20 * 0.63168 = 12.63 (mm/10 phút) = 12.63*6 = 75.78 mm/h
*/

//-------------------------------- Khai báo cảm biến gió ------------------------------------
#define windSensor        13
#define WIND_ON   0
#define WIND_OFF  1
int windCountPulse = 0;
uint32_t timeMillisWind   = 0; 
uint32_t timeCalulateWind = 0; 
float D_Anemometer = 0;             // Đường kính của bộ đo mô hình, đơn vị mét (tùy thuộc vào mô hình, cần chỉnh sửa đúng thực tế)
#define PI  3.14
float C_Anemometer = 0;              // Chu vi của mô hình, đơn vị mét
float windValue = 0;                // Vận tốc của gió m/s
float windValueMax = 0;
bool trigWindBlynk = 0;
/*
* Tính toán 1 chút để tính được tốc độ gió, nếu cơ cấu quay của mô hình đủ nhẹ, tốc độ của trục quay sẽ xấp xỉ tốc độ gió
* Ta có mô hình như sau
  Đường kính trục quay : 205mm = 0.205 m
* Ví dụ với thời gian đo là 1s, ta đo được 5 xung
* 5 xung tương đương với bộ đo đã quay được 5 vòng, vậy quãng đường đã xoay được S = 5 * C_Anemometer = 5 * 0.205 * 3.14 = 3.2185 m
* Với công thức S = v * t => v = S / t = 3.2185/1 = 3.22 m/s
* Như vậy ta đã đo được tốc độ gió tức thời 3.22m/s
* Tuy nhiên ta nên đo thời gian dài hơn để có kết quả chính xác hơn, ví dụ như đo trong 5s, 10s xem được bao nhiêu xung, sau đó chia tỷ lệ để có kết quả chính xác hơn
* Kết quả đo còn phụ thuộc vào phần cứng mô hình, nếu như cánh quay mô hình quá nặng thì kết quả sẽ k được chính xác, cần phải chỉnh theo thực tế
*/
bool trigRainBlynk = 0;
bool rainSensorTrig = 0;
// Hàm ngắt ngoài cảm biến mưa
void IRAM_ATTR rainSensorISR() {
    rainSensorTrig = 1;
}
bool windSensorTrig = 0;
// Hàm ngắt ngoài cảm biến gió
void IRAM_ATTR windSensorISR() {
    windSensorTrig = 1;
}
// Khởi tạo cảm biến mưa
void rainInit(){
  V_AmountOfWater = ErainAmountOfWater;
  S_Funnel = ErainSFunnel;
  onePulseValue = ((1000000/(float)S_Funnel)*(float)V_AmountOfWater)/1000 ;   

  Serial.print("V_AmountOfWater : ");Serial.println(V_AmountOfWater); 
  Serial.print("S_Funnel : ");Serial.println(S_Funnel); 
  Serial.print("onePulseValue : ");Serial.println(onePulseValue); 

  pinMode(rainSensor, INPUT_PULLUP);
  attachInterrupt(rainSensor, rainSensorISR, FALLING);
}
// Khởi tạo cảm biến gió
void windInit(){
  D_Anemometer = (float)EwindDAnemometer/1000;
  C_Anemometer = (float)PI*D_Anemometer ;

  Serial.print("D_Anemometer : ");Serial.println(D_Anemometer);
  Serial.print("C_Anemometer : ");Serial.println(C_Anemometer);

  pinMode(windSensor, INPUT_PULLUP);
  attachInterrupt(windSensor, windSensorISR, FALLING);
}

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
  // Khởi tạo DHT11
  dht.begin();
  // Khởi tạo rainSensor
  rainInit();
  // Khởi tạo windSensor
  windInit();


  // Khởi tạo nút nhấn
  pinMode(BUTTON_SET_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  button_init(&buttonSET, BUTTON_SET_PIN, BUTTON1_ID);
  button_init(&buttonUP, BUTTON_UP_PIN, BUTTON2_ID);
  button_init(&buttonDOWN,   BUTTON_DOWN_PIN,   BUTTON3_ID);
  button_pressshort_set_callback((void *)button_press_short_callback);
  button_presslong_set_callback((void *)button_press_long_callback);

  xTaskCreatePinnedToCore(TaskButton,          "TaskButton" ,          1024*10 ,  NULL,  20 ,  &TaskButton_handle       , 1);
  xTaskCreatePinnedToCore(TaskOLEDDisplay,     "TaskOLEDDisplay" ,     1024*16 ,  NULL,  20 ,  &TaskOLEDDisplay_handle  , 1);
  xTaskCreatePinnedToCore(TaskDHT11,           "TaskDHT11" ,           1024*10 ,  NULL,  20 ,  &TaskDHT11_handle  , 1);
  xTaskCreatePinnedToCore(TaskWindSensor,      "TaskWindSensor" ,      1024*10 ,  NULL,  10 ,  NULL ,  1);
  xTaskCreatePinnedToCore(TaskRainSensor,      "TaskRainSensor" ,      1024*10 ,  NULL,  10 ,  NULL ,  1);
  xTaskCreatePinnedToCore(TaskAutoWarning,     "TaskAutoWarning" ,     1024*10 ,  NULL,  10 , &TaskAutoWarning_handle ,  1);

  // Kết nối wifi
  connectSTA();
}

void loop() {
  vTaskDelete(NULL);
}

//---------------------------Task đo cảm biến mưa---------------------------
void TaskRainSensor(void *pvParameters) {
    while(1) {
      //rainValue = 5;
      if(rainSensorTrig == 1) {
        delay(150);
        rainSensorTrig = 0;
        rainCountPulse++;
        Serial.print("rainCountPulse : ");
        Serial.println(rainCountPulse);
      }
      if(millis() - timeMillisRain > 1000) {
        timeCalulateRain ++;
        timeMillisRain = millis();
        Serial.print("timeCalulateRain : ");
        Serial.println(timeCalulateRain);
      }
      if(timeCalulateRain >= ErainTimeSample*60) {
        rainValue = (rainCountPulse*onePulseValue)*(3600/(ErainTimeSample*60));
        rainCountPulse = 0;
        timeCalulateRain = 0;
        Serial.print("rainValue : ");
        Serial.print(rainValue);
        Serial.println("mm/h");
        trigRainBlynk = 1;
       
        if(rainValue > rainValueMax) rainValueMax = rainValue;
      }
      delay(100);
    }
}

//---------------------------Task đo cảm biến gió---------------------------
void TaskWindSensor(void *pvParameters) {
    while(1) {
      //windValue = 2.2;
      if(windSensorTrig == 1) { 
        delay(100);
        windSensorTrig = 0;
        windCountPulse++;
        Serial.print("windCountPulse : ");
        Serial.println(windCountPulse);
      }

      if(millis() - timeMillisWind> 1000) {
        timeCalulateWind ++;
        timeMillisWind = millis();
        //  Serial.print("timeCalulateWind : ");
        //  Serial.println(timeCalulateWind); 
      }
      if(timeCalulateWind >= EwindTimeSample) {
        windValue = (windCountPulse * C_Anemometer) / EwindTimeSample;
       
        windCountPulse = 0;
        timeCalulateWind = 0;
        Serial.print("windValue : ");
        Serial.print(windValue);
        Serial.println("m/s");
        trigWindBlynk = 1;
        if(windValue > windValueMax) windValueMax = windValue;
      }
      delay(100);
    }
}

//--------------------Task đo DHT11 ---------------
void TaskDHT11(void *pvParameters) { 
    //delay(10000);
    while(1) {
      float humi =  dht.readHumidity();
      float temp =  dht.readTemperature();
      if (isnan(humi) || isnan(temp) ) {
          Serial.println(F("Failed to read from DHT sensor!"));
          dht11ReadOK = false;
      }
      else if(humi <= 100 && temp < 100) {
          dht11ReadOK = true;
          // humiValue = humifilter.updateEstimate(humi);
          // tempValue = tempfilter.updateEstimate(temp);
          humiValue = humi;
          tempValue = temp;

          Serial.print(F("Humidity: "));
          Serial.print(humiValue);
          Serial.print(F("%  Temperature: "));
          Serial.print(tempValue);
          Serial.print(F("°C "));
          Serial.println();

          if(tempValue < EtempThreshold1 || tempValue > EtempThreshold2) 
            warningTempState = SAD;
          else
            warningTempState = HAPPY;
          if(humiValue < EhumiThreshold1 || tempValue > EhumiThreshold2) 
            warningHumiState = SAD;
          else
            warningHumiState = HAPPY;
      }
      delay(3000);
    }
}


// Xóa 1 ô hình chữ nhật từ tọa độ (x1,y1) đến (x2,y2)
void clearRectangle(int x1, int y1, int x2, int y2) {
   for(int i = y1; i < y2; i++) {
     oled.drawLine(x1, i, x2, i, 0);
   }
}

void clearOLED(){
  oled.clearDisplay();
  oled.display();
}

int countSCREEN9 = 0;
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
        case SCREEN1:   // Hiển thị nhiệt độ , độ ẩm
          for(int j = 0; j < 2 && enableShow == ENABLE; j++) {

          oled.clearDisplay();
          oled.setTextSize(1);
          oled.setCursor(0, 0);
          oled.print("Nhiet do: ");
          oled.setTextSize(2);
          oled.setCursor(0, 12);
          oled.print(tempValue,1); oled.drawCircle(52, 12, 3,SH110X_WHITE); oled.print(" C"); 
         
          oled.setTextSize(1);
          oled.setCursor(0, 34);
          oled.print("Do am: ");
          oled.setTextSize(2);
          oled.setCursor(0, 46);
          oled.print(humiValue,1); oled.print(" %"); 
          
            for(int i = 0; i < FRAME_COUNT_face1OLED && enableShow == ENABLE; i++) {
                clearRectangle(96, 0, 128, 64);
                if(warningTempState == SAD)
                  oled.drawBitmap(96, 0, face1OLED[i], 32, 32, 1);
                else if(warningTempState == NORMAL)
                  oled.drawBitmap(96, 0, face2OLED[i], 32, 32, 1);
                else if(warningTempState == HAPPY)
                  oled.drawBitmap(96, 0, face3OLED[i], 32, 32, 1);
                if(warningHumiState == SAD)
                  oled.drawBitmap(96, 32, face1OLED[i], 32, 32, 1);
                else if(warningHumiState == NORMAL)
                  oled.drawBitmap(96, 32, face2OLED[i], 32, 32, 1);
                else if(warningHumiState == HAPPY)
                  oled.drawBitmap(96, 32, face3OLED[i], 32, 32, 1);
                oled.display();
                delay(FRAME_DELAY);
            }
            oled.display();
            delay(100);
          }
          if( enableShow == ENABLE)
            screenOLED = SCREEN3;
          break;
        case SCREEN2:  
          
          break;
        case SCREEN3:  // Hiển thị mưa, gió
          for(int j = 0; j < 2 && enableShow == ENABLE; j++) {

            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(0, 0);
            oled.print("Cam bien mua:");
            oled.setTextSize(2);
            oled.setCursor(0, 12);
            if(rainValue >= 1000)
              oled.print(rainValue,0); 
            else
             oled.print(rainValue,1);  
            oled.setTextSize(1);
            oled.print(" mm/h"); 
          
            oled.setTextSize(1);
            oled.setCursor(0, 34);
            oled.print("Cam bien gio: ");
            oled.setTextSize(2);
            oled.setCursor(0, 46);
            oled.print(windValue,1); 
            oled.setTextSize(1);
            oled.print(" m/s"); 

            for(int i = 0; i < FRAME_COUNT_windOLED && enableShow == ENABLE; i++) {
              clearRectangle(96, 0, 128, 64);
              if(windValue > 0 )
                oled.drawBitmap(96, 32, windOLED[i], 32, 32, 1);
              else
                oled.drawBitmap(96, 32, windOLED[0], 32, 32, 1);

               if(rainValue == 0) 
                 oled.drawBitmap(96, 0, blynkOLED[0], 32, 32, 1);
               else if(rainValue > 0 && rainValue <= ErainThreshold1) 
                 oled.drawBitmap(96, 0, smallRainOLED[i], 32, 32, 1);
               else if(rainValue > ErainThreshold1 && rainValue <= ErainThreshold2) 
                 oled.drawBitmap(96, 0, normalRainOLED[i], 32, 32, 1);
               else 
                 oled.drawBitmap(96, 0, bigRainOLED[i], 32, 32, 1);
              oled.display();
              delay(FRAME_DELAY);  
            }
            oled.display();
            delay(100);
          }
          if( enableShow == ENABLE)
            screenOLED = SCREEN1;
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
            screenOLED = SCREEN3;
            enableShow = ENABLE;
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
              enableShow = ENABLE;
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
            enableShow = ENABLE;
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
            enableShow = ENABLE;
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
            enableShow = ENABLE;
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
      enableShow = DISABLE;
      if ( Essid.length() > 1 ) {  
      Serial.println(Essid);        //Print SSID
      Serial.println(Epass);        //Print Password
      Serial.println(Etoken);        //Print token
      Etoken = Etoken.c_str();
      WiFi.begin(Essid.c_str(), Epass.c_str());   //c_str()
      int countConnect = 0;
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);   
          if(countConnect++  == 7) {
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
            enableShow = ENABLE;
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
    enableShow = DISABLE;
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

  if(myObject.hasOwnProperty("tempThreshold1"))
    EtempThreshold1 = (int) myObject["tempThreshold1"];
  if(myObject.hasOwnProperty("tempThreshold2")) 
    EtempThreshold2 = (int) myObject["tempThreshold2"];

  if(myObject.hasOwnProperty("humiThreshold1")) 
    EhumiThreshold1 = (int) myObject["humiThreshold1"];
  if(myObject.hasOwnProperty("humiThreshold2")) 
    EhumiThreshold2 = (int) myObject["humiThreshold2"];

  if(myObject.hasOwnProperty("rainThreshold1")) 
    ErainThreshold1 = (int) myObject["rainThreshold1"];
  if(myObject.hasOwnProperty("rainThreshold2")) 
    ErainThreshold2 = (int) myObject["rainThreshold2"];

  if(myObject.hasOwnProperty("windThreshold1")) 
    EwindThreshold1 = (int) myObject["windThreshold1"];
  if(myObject.hasOwnProperty("windThreshold2")) 
    EwindThreshold2 = (int) myObject["windThreshold2"];

  if(myObject.hasOwnProperty("rainTimeSample")) 
    ErainTimeSample = (int) myObject["rainTimeSample"];
  if(myObject.hasOwnProperty("windTimeSample")) 
    EwindTimeSample = (int) myObject["windTimeSample"];

  if(myObject.hasOwnProperty("rainSFunnel")) 
    ErainSFunnel = (int) myObject["rainSFunnel"];

  if(myObject.hasOwnProperty("rainAmountOfWater")) 
    ErainAmountOfWater = (int) myObject["rainAmountOfWater"];

  if(myObject.hasOwnProperty("windDAnemometer")) 
    EwindDAnemometer = (int) myObject["windDAnemometer"];
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

    Serial.print("tempThreshold1 = ");
    Serial.println(EtempThreshold1);

    Serial.print("tempThreshold2 = ");
    Serial.println(EtempThreshold2);

    Serial.print("humiThreshold1 = ");
    Serial.println(EhumiThreshold1);

    Serial.print("humiThreshold2 = ");
    Serial.println(EhumiThreshold2);

    Serial.print("rainThreshold1 = ");
    Serial.println(ErainThreshold1);
    Serial.print("rainThreshold2 = ");
    Serial.println(ErainThreshold2);

    Serial.print("windThreshold1 = ");
    Serial.println(EwindThreshold1);
    Serial.print("windThreshold2 = ");
    Serial.println(EwindThreshold2);

    Serial.print("rainTimeSample = ");
    Serial.println(ErainTimeSample);

    Serial.print("windTimeSample = ");
    Serial.println(EwindTimeSample);

    Serial.print("rainSFunnel = ");
    Serial.println(ErainSFunnel);

    Serial.print("rainAmountOfWater = ");
    Serial.println(ErainAmountOfWater);

    Serial.print("windDAnemometer = ");
    Serial.println(EwindDAnemometer);



    Serial.print("autoWarning = ");
    Serial.println(autoWarning);
}

//-------- Hàm tạo biến JSON để gửi đi khi có request HTTP_GET "/" --------
String getJsonData() {
  JSONVar myObject;
  myObject["ssid"]  = Essid;
  myObject["pass"]  = Epass;
  myObject["token"] = Etoken;
  myObject["tempThreshold1"] = EtempThreshold1;
  myObject["tempThreshold2"] = EtempThreshold2;
  myObject["humiThreshold1"] = EhumiThreshold1;
  myObject["humiThreshold2"] = EhumiThreshold2;
  myObject["rainThreshold1"] = ErainThreshold1;
  myObject["rainThreshold2"] = ErainThreshold2;
  myObject["windThreshold1"] = EwindThreshold1;
  myObject["windThreshold2"] = EwindThreshold2;
  myObject["rainTimeSample"] = ErainTimeSample;
  myObject["windTimeSample"] = EwindTimeSample;
  myObject["rainSFunnel"]       = ErainSFunnel;
  myObject["rainAmountOfWater"] = ErainAmountOfWater;
  myObject["windDAnemometer"]   = EwindDAnemometer;
  
  String jsonData = JSON.stringify(myObject);
  return jsonData;
}

//-------------------------------------------------------------------------------
//--------------------------------Task Blynk-------------------------------------

//----------------------------- Task auto Warning--------------------------------
void TaskAutoWarning(void *pvParameters)  {
    delay(20000);
    while(1) {
      if(autoWarning == 1) {
          check_data_and_send_to_blynk(tempValue, humiValue, rainValue, windValue);
      }
      delay(10000);
    }
}

//----------------------- Send send Data value to Blynk every 2 seconds--------

void myTimer() {
    Blynk.virtualWrite(V0, tempValue);  
    Blynk.virtualWrite(V1, humiValue);
    if(trigRainBlynk == 1)  {
      Blynk.virtualWrite(V2, rainValue);
      trigRainBlynk = 0;
    }
    if(trigWindBlynk == 1)  {
      Blynk.virtualWrite(V3, windValue);
      trigWindBlynk = 0;
    }
    
    Blynk.virtualWrite(V4, autoWarning); 
}
//--------------Read button from BLYNK and send notification back to Blynk-----------------------
int checkAirQuality = 0;
BLYNK_WRITE(V3) {
    enableShow = DISABLE;
    checkAirQuality = param.asInt();
    if(checkAirQuality == 1) {
      buzzerBeep(1);
      check_data_and_send_to_blynk(tempValue, humiValue, rainValue, windValue);
      screenOLED = SCREEN12;
    } 
}

//------------------------- check autoWarning from BLYNK  -----------------------
BLYNK_WRITE(V4) {
    enableShow = DISABLE;
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

    if(Essid.length() == 0) Essid = "BLK";
    EtempThreshold1 = EEPROM.read(200);
    EtempThreshold2 = EEPROM.read(201);

    EhumiThreshold1 = EEPROM.read(202);
    EhumiThreshold2 = EEPROM.read(203);

    ErainThreshold1 = EEPROM.read(204) * 100 + EEPROM.read(205);
    ErainThreshold2 = EEPROM.read(206) * 100 + EEPROM.read(207);  

    EwindThreshold1 = EEPROM.read(211) * 100 + EEPROM.read(212);
    EwindThreshold2 = EEPROM.read(213) * 100 + EEPROM.read(214);  

    int ErainTimeSampleTemp = EEPROM.read(215) * 100 + EEPROM.read(216); 
    if(ErainTimeSampleTemp > 0) ErainTimeSample = ErainTimeSampleTemp;

    int EwindTimeSampleTemp = EEPROM.read(217) * 100 + EEPROM.read(218); 
    if(EwindTimeSampleTemp > 0) EwindTimeSample = EwindTimeSampleTemp;

    int ErainSFunnelTemp    = EEPROM.read(219) * 10000  + EEPROM.read(220) * 100 + EEPROM.read(221); 
    if(ErainSFunnelTemp > 0) ErainSFunnel = ErainSFunnelTemp;
    
    int ErainAmountOfWaterTemp = EEPROM.read(222);
    if(ErainAmountOfWaterTemp > 0) ErainAmountOfWater = ErainAmountOfWaterTemp;

    int EwindDAnemometerTemp = EEPROM.read(223) * 100 + EEPROM.read(224); 
    if(EwindDAnemometerTemp > 0) EwindDAnemometer = EwindDAnemometerTemp;

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

    EEPROM.write(200, EtempThreshold1);          // lưu ngưỡng nhiệt độ 1
    EEPROM.write(201, EtempThreshold2);          // lưu ngưỡng nhiệt độ 2

    EEPROM.write(202, EhumiThreshold1);          // lưu ngưỡng độ ẩm 1
    EEPROM.write(203, EhumiThreshold2);          // lưu ngưỡng độ ẩm 2

    EEPROM.write(204, ErainThreshold1 / 100);      // lưu hàng nghìn + trăm mưa 1
    EEPROM.write(205, ErainThreshold1 % 100);      // lưu hàng chục + đơn vị mưa 1

    EEPROM.write(206, ErainThreshold2 / 100);      // lưu hàng nghìn + trăm mưa 2
    EEPROM.write(207, ErainThreshold2 % 100);      // lưu hàng chục + đơn vị mưa 2

    EEPROM.write(211, EwindThreshold1 / 100);      // lưu hàng nghìn + trăm gió 1
    EEPROM.write(212, EwindThreshold1 % 100);      // lưu hàng chục + đơn vị gió 1

    EEPROM.write(213, EwindThreshold2 / 100);      // lưu hàng nghìn + trăm gió 1
    EEPROM.write(214, EwindThreshold2 % 100);      // lưu hàng chục + đơn vị gió 2

    EEPROM.write(215, ErainTimeSample / 100);      // lưu hàng nghìn + trăm thời gian lấy mẫu mưa
    EEPROM.write(216, ErainTimeSample % 100);      // lưu hàng chục + đơn vị thời gian lấy mẫu mưa

    EEPROM.write(217, EwindTimeSample / 100);      // lưu hàng nghìn + trăm thời gian lấy mẫu gió
    EEPROM.write(218, EwindTimeSample % 100);      // lưu hàng chục + đơn vị thời gian lấy mẫu gió

    EEPROM.write(219, (ErainSFunnel / 100)/100);   // lưu chục nghìn + trăm nghìn diện tích phễu hứng mưa
    EEPROM.write(220, (ErainSFunnel / 100)%100);   // lưu nghìn + trăm diện tích phễu hứng mưa
    EEPROM.write(221, (ErainSFunnel / 100)%100);   // lưu hàng chục + đơn vị diện tích phễu hứng mưa

    EEPROM.write(222, ErainAmountOfWater);         // lưu lượng nước mưa vừa đủ để phễu đo lật

    EEPROM.write(223, EwindDAnemometer / 100);      // lưu hàng nghìn + trăm đường kính trục cánh quạt quay hứng gió
    EEPROM.write(224, EwindDAnemometer % 100);      // lưu hàng chục + đơn vị đường kính trục cánh quạt quay hứng gió

    
    EEPROM.commit();

    Serial.println("write eeprom");
    delay(500);
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
//-----------------Hàm xử lí nút nhấn nhả ----------------------
void button_press_short_callback(uint8_t button_id) {
    switch(button_id) {
      case BUTTON1_ID :  
        buzzerBeep(1);
        Serial.println("btSET press short");
        break;
      case BUTTON2_ID :
        buzzerBeep(1);
        Serial.println("btUP press short");
        break;
      case BUTTON3_ID :
        buzzerBeep(1);
        Serial.println("btDOWN press short");
        enableShow = DISABLE;
        check_data_and_send_to_blynk(tempValue, humiValue, rainValue, windValue);
        screenOLED = SCREEN12;
        break;  
    } 
} 
//-----------------Hàm xử lí nút nhấn giữ ----------------------
void button_press_long_callback(uint8_t button_id) {
  switch(button_id) {
    case BUTTON1_ID :
      buzzerBeep(2);  
      enableShow = DISABLE;
      Serial.println("btSET press long");
      screenOLED = SCREEN9;
      clearOLED();
      connectAPMode(); 
      break;
    case BUTTON2_ID :
      buzzerBeep(2);
      Serial.println("btUP press short");
      break;
    case BUTTON3_ID :
      buzzerBeep(2);
      Serial.println("btDOWN press short");
      enableShow = DISABLE;
      autoWarning = 1 - autoWarning;
      EEPROM.write(210, autoWarning);  EEPROM.commit();
      Blynk.virtualWrite(V4, autoWarning); 
      if(autoWarning == 0) screenOLED = SCREEN11;
      else screenOLED = SCREEN10;
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

/**
 * @brief Kiểm tra chất lượng không khí và gửi lên BLYNK
 *
 * @param autoWarning auto Warning
 * @param temp Nhiệt độ hiện tại    *C
 * @param humi Độ ẩm hiện tại        %
 * @param rain Mưa hiện tại   mm/h
 * @param wind Gió hiện tại   m/s
 */
void check_data_and_send_to_blynk(int temp, int humi, int rain, int wind) {
  String notifications = "";
  int tempIndex = 0;
  int rainIndex = 0;
  int windIndex = 0;
  int humiIndex = 0;
  if(dht11ReadOK ==  true) {
    if(temp < EtempThreshold1 )tempIndex = 1;
    else if(temp >= EtempThreshold1 && temp <= EtempThreshold2)  tempIndex = 0;
    else tempIndex = 3;
    

    if(humi < EhumiThreshold1 ) humiIndex = 1;
    else if(humi >= EhumiThreshold1 && humi <= EhumiThreshold2)   humiIndex = 0;
    else humiIndex = 3;

    if(rain == 0 ) rainIndex = 0;
    else if(rain > 0 && rain <= ErainThreshold1)   rainIndex = 2;
    else if(rain > ErainThreshold1 && rain <= ErainThreshold2)   rainIndex = 3;
    else rainIndex = 4;

    if(wind == 0 ) windIndex = 0;
    else if(wind > 0 && wind <= EwindThreshold1) windIndex = 0;
    else if(wind >= EwindThreshold1 && wind <= EwindThreshold2)   windIndex = 3;
    else windIndex = 4;

    if(tempIndex == 0 && humiIndex == 0 && rainIndex == 0)
      notifications = "";
    else {
      if(tempIndex != 0) notifications = notifications + snTemp[tempIndex] + String(temp) + "*C . ";
      if(humiIndex != 0) notifications = notifications + snHumi[humiIndex] + String(humi) + "% . " ;
      if(rainIndex != 0) notifications = notifications + snRain[rainIndex] + String(rain) + "mm/h . " ;
      if(windIndex != 0) notifications = notifications + snWind[windIndex] + String(wind) + "m/s . " ;
      Blynk.logEvent("auto_warning",notifications);
    }
    Serial.println(notifications);
  }
}
