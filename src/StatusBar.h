#ifndef _STATUS_BAR_H
#define _STATUS_BAR_H

#include <TFT_eSPI.h>
#include <WiFi.h>
#include "RTClib.h"

#include "Helpers/Color.h"
#include "Helpers/Sensors.h"

extern TFT_eSPI tft;
extern SensorManager sensors;

// Cấu hình
#define BAR_HEIGHT      12      
#define BAR_WIDTH       160    
#define BAR_BG_COLOR    BLACK
#define BAR_TEXT_COLOR  WHITE

// Chân đo pin (Bạn cần kiểm tra lại sơ đồ mạch của mình)
// ESP32 thường dùng GPIO 34, 35 qua cầu phân áp
#define BATTERY_PIN     34 
#define MAX_VOLTAGE     4.2
#define MIN_VOLTAGE     3.3

class StatusBar {
private:
    TFT_eSprite spr = TFT_eSprite(&tft);
    
    // Trạng thái nội tại
    int lastBatteryLevel = -1;
    bool lastWifiState = false;
    String lastTimeStr = "";
    bool isBleConnected = false; // Biến cờ cho Bluetooth

    unsigned long lastUpdate = 0;
    const int UPDATE_INTERVAL = 1000; // Cập nhật mỗi 1 giây

public:
    void init() {
        spr.setColorDepth(16);
        spr.createSprite(BAR_WIDTH, BAR_HEIGHT);
        
        // Cấu hình chân đo pin
        // pinMode(BATTERY_PIN, INPUT);
        
        draw(); // Vẽ lần đầu
    }

    // Hàm cập nhật (gọi trong loop hoặc task)
    void update() {
        if (millis() - lastUpdate > UPDATE_INTERVAL) {
            draw();
            lastUpdate = millis();
        }
    }

    // Set trạng thái Bluetooth từ bên ngoài
    void setBluetoothStatus(bool connected) {
        if (isBleConnected != connected) {
            isBleConnected = connected;
            draw(); // Vẽ lại ngay lập tức
        }
    }

    void forceRedraw() {
        draw(); 
    }

private:
    // --- CÁC HÀM VẼ ICON ---
    void drawBattery(int x, int y, int percentage) {
        // Vẽ vỏ pin
        spr.drawRect(x, y + 2, 16, 8, TFT_WHITE);
        spr.fillRect(x + 16, y + 4, 2, 4, TFT_WHITE); // Cực dương

        // Vẽ dung lượng
        uint16_t color = TFT_GREEN;
        if (percentage < 20) color = TFT_RED;
        else if (percentage < 50) color = TFT_YELLOW;

        int w = map(percentage, 0, 100, 0, 12);
        if (w > 12) w = 12;
        if (w < 0) w = 0;
        
        spr.fillRect(x + 2, y + 4, w, 4, color);
    }

    void drawWifi(int x, int y, bool connected) {
        if (!connected) {
            // Vẽ dấu X hoặc mờ
            spr.setTextColor(TFT_DARKGREY);
            spr.setCursor(x, y+2);
            spr.print("x");
            return;
        }
        
        // Vẽ 3 vạch sóng
        spr.fillRect(x, y + 6, 2, 2, TFT_WHITE);     // Chấm nhỏ
        spr.drawLine(x + 3, y + 4, x + 3, y + 8, TFT_WHITE); // Vạch vừa
        spr.drawLine(x + 6, y + 2, x + 6, y + 8, TFT_WHITE); // Vạch lớn
    }

    void drawBluetooth(int x, int y, bool connected) {
        uint16_t color = connected ? TFT_BLUE : TFT_DARKGREY;
        // Vẽ biểu tượng BT đơn giản (chữ B cách điệu)
        spr.drawLine(x+2, y, x+2, y+10, color);     // Cột dọc
        spr.drawLine(x+2, y+2, x+5, y+5, color);    // Chéo xuống
        spr.drawLine(x+5, y+5, x+2, y+8, color);    // Chéo vào
        spr.drawLine(x+2, y+8, x+5, y+5, color); // (Vẽ lại cho đậm hoặc thêm chi tiết)
        
        // Hoặc đơn giản là chữ B
        spr.setTextColor(color);
        spr.setTextSize(1);
        spr.setCursor(x, y+2);
        spr.print("B");
    }

    int getBatteryPercentage() {
        // Đây là hàm giả lập. Thực tế bạn cần đọc analogRead(BATTERY_PIN)
        // và map theo điện áp cầu phân áp của mạch bạn.
        // int raw = analogRead(BATTERY_PIN);
        // return map(raw, 2500, 4095, 0, 100); 
        return 85; // Giả sử 85%
    }

    void drawSettingsIcon(int x, int y) {
        // Vẽ bánh răng đơn giản (Màu xám hoặc trắng)
        spr.setTextColor(LIGHTGREY);
        
        // Vẽ vòng tròn ngoài
        spr.drawCircle(x + 4, y + 4, 3, LIGHTGREY);
        // Vẽ vòng tròn trong (lỗ trục)
        spr.drawPixel(x + 4, y + 4, TFT_BLACK);
        
        // Vẽ các răng cưa (dấu + và dấu x)
        spr.drawLine(x + 1, y + 4, x + 7, y + 4, LIGHTGREY); // Ngang
        spr.drawLine(x + 4, y + 1, x + 4, y + 7, LIGHTGREY); // Dọc
        spr.drawLine(x + 2, y + 2, x + 6, y + 6, LIGHTGREY); // Chéo 1
        spr.drawLine(x + 6, y + 2, x + 2, y + 6, LIGHTGREY); // Chéo 2
    }

    String getCurrentTime() {
        return sensors.getTimeStr();
    }

    // --- HÀM VẼ CHÍNH ---
    void draw() {
        spr.fillSprite(BAR_BG_COLOR); // Xóa nền
        
        // 1. Vẽ Thời gian (Bên trái)
        spr.setTextColor(BAR_TEXT_COLOR, BAR_BG_COLOR);
        spr.setTextSize(1);
        spr.setCursor(2, 2);
        spr.print(getCurrentTime());

        // 2. Vẽ các Icon kết nối 
        int rightMargin = BAR_WIDTH - 20; 

        // Settings
        drawSettingsIcon(rightMargin + 10, 1);

        // Battery 
        drawBattery(rightMargin - 10, 0, getBatteryPercentage());
        
        // Wifi 
        drawWifi(rightMargin - 20, 0, WiFi.status() == WL_CONNECTED);

        // Bluetooth 
        drawBluetooth(rightMargin - 30, 0, isBleConnected);

        // Đường kẻ phân cách với Game (nếu thích)
        spr.drawLine(0, BAR_HEIGHT-1, BAR_WIDTH, BAR_HEIGHT-1, TFT_DARKGREY);

        spr.pushSprite(0, 0); // Đẩy lên vị trí trên cùng màn hình
    }
};

#endif