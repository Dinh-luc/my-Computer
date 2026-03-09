#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <Preferences.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include <WiFi.h>

#include "Helpers/Keypad_4x4.h"
#include "Helpers/Storage.h"
#include "Helpers/Sensors.h"
#include "Helpers/Theme.h"

#include "AppRegistry.h"
#include "AppManager.h"

// ================= Define Pins TFT_eSPI =================
#define TFT_MOSI  46  // Chân SDA / DIN trên màn hình
#define TFT_SCLK  3   // Chân SCK / CLK trên màn hình
#define TFT_CS    1   // Chân CS (Chip Select)
#define TFT_DC    42  // Chân DC / RS (Data/Command)
#define TFT_RST   2   // Chân RES / RST (Reset)

int width = 160;
int height = 128;

TFT_eSPI tft = TFT_eSPI(); 
Keypad_4x4 keypad;
Storage storage;
SensorManager sensors;
ThemeManager theme;

AppManager manager(myApps, APP_COUNT);
SemaphoreHandle_t tftMutex;
SemaphoreHandle_t sysMutex;

RTC_DS3231 rtc;

// ================= HELPERS =================
// Hàm đồng bộ giờ từ NTP về DS3231
void syncTime() {
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  
  Serial.print("Đang đồng bộ giờ");
  int retry = 0;
  while(!getLocalTime(&timeinfo) && retry < 10) {
    Serial.print(".");
    delay(500);
    retry++;
  }
  
  if (retry < 10) {
    Serial.println("\nĐã lấy được giờ NTP!");
    // Ghi vào DS3231
    sensors.setTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.println("Đã cập nhật DS3231.");
  } else {
    Serial.println("\nKhông lấy được giờ NTP.");
  }
}

//================================== Tasks ==================================
// Task xử lý phím bấm
void taskInput(void* pvParameters) {
  while(true) {
    char key = keypad.getKey();
    if (key) {
      if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
        Serial.print("Key pressed in task: ");
        Serial.println(key);
        manager.handleInput(key);
        xSemaphoreGive(tftMutex);
      }
    }
    vTaskDelay(10); // Giữ cho task nhẹ nhàng
  }
}

// Task vòng lặp game
void taskGameLoop(void* pvParameters) {
  while(true) {
    if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE) {
        manager.update(); 
        xSemaphoreGive(tftMutex); 
    }
    vTaskDelay(5); // Delay nhỏ để tránh treo task
  }
}

// Task xử lý Wifi và đồng bộ giờ chạy ngầm (chạy 1 lần rồi xóa hoặc định kỳ)
void taskSystem(void* pvParameters) {
  Preferences prefs;
  prefs.begin("wifi_creds", true); // True = Read only
  String savedSSID = prefs.getString("ssid", "");
  String savedPass = prefs.getString("password", "");
  prefs.end();
  
  if (savedSSID.length() > 0) {
    Serial.print("📡 Auto-connecting to: ");
    Serial.println(savedSSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());

    // Chờ kết nối 
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      vTaskDelay(500); 
      Serial.print(".");
      retry++;
    }

    // Nếu kết nối thành công -> Đồng bộ giờ
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ WiFi Connected!");
      
      // Lock mutex trước khi truy cập I2C (RTC)
      if (xSemaphoreTake(sysMutex, portMAX_DELAY) == pdTRUE) {
        syncTime();
        xSemaphoreGive(sysMutex);
      }
    } else {
      Serial.println("\n❌ Auto-connect failed.");
    }
  } else {
    Serial.println("⚠️ No saved WiFi credentials.");
  }
  
  // Tự hủy task sau khi hoàn thành nhiệm vụ để giải phóng RAM
  vTaskDelete(NULL);
}

// ================= Setup =================
void setup() {
  Serial.begin(115200);
  keypad.begin();
  randomSeed(analogRead(9));
  
  tftMutex = xSemaphoreCreateMutex();
  sysMutex = xSemaphoreCreateMutex();

  sensors.init();
  theme.begin();

  if(psramFound()) Serial.printf("✅ PSRAM: %d bytes\n", ESP.getPsramSize());

  if(!storage.initSD()) {
    Serial.println("❌ Không thể khởi tạo SD Card");
  }
  else {
    Serial.println("✅ SD Card đã sẵn sàng");
  }

  tft.init(); 
  tft.setRotation(3); 
  tft.fillScreen(TFT_BLACK);

  manager.init();

  xTaskCreatePinnedToCore(taskInput, "Input", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskGameLoop, "GameLoop", 8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskSystem, "SysTask", 8192, NULL, 1, NULL, 0);

  Serial.println("✅ Khởi tạo thành công!");
}

// ================= Loop =================
void loop() {
  vTaskDelay(1); 
}
