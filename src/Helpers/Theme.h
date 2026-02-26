#ifndef _THEME_H
#define _THEME_H

#include <Arduino.h>
#include <Preferences.h>
#include <TJpg_Decoder.h>

#include "Helpers/Color.h"

// Định nghĩa chân đèn nền màn hình 
#define TFT_BL_PIN 21 

enum ThemeMode {
    THEME_DARK,
    THEME_LIGHT,
    THEME_CUSTOM
};

struct ThemeColor {
    uint16_t bg;        // Nền
    uint16_t subBg;     // Nền phụ
    uint16_t text;      // Chữ chính
    uint16_t textSub;   // Chữ phụ/mờ
    uint16_t highlight; // Màu chọn/Active
    uint16_t border;    // Viền
    uint16_t alert;     // Thông báo lỗi
};

class ThemeManager {
private:
    Preferences prefs;
    
public:
    ThemeMode currentMode = THEME_DARK;
    ThemeColor color;
    uint16_t customPrimaryColor = ORANGE;
    
    // Cấu hình màn hình
    // int brightness = 200; // 0-255
    String wallpaperPath = ""; // Nếu rỗng dùng màu nền, nếu có path dùng ảnh
    
    // Cấu hình Screensaver
    bool screensaverEnabled = true;
    int screensaverTimeout = 30; // Giây
    int screensaverType = 0; // 0: Clock, 1: Image Slide, 2: Black

    void setCustomTheme(uint16_t primary) {
        customPrimaryColor = primary;
        currentMode = THEME_CUSTOM;
        
        // 1. Nền: Xám rất tối (để nội dung nổi bật)
        // 2. Highlight: Màu chủ đạo user chọn
        // 3. Border: Màu chủ đạo nhưng tối hơn 50% (giả lập bằng cách shift bit)
        uint16_t dimmedColor = (primary >> 1) & 0x7BEF; 

        color = {
            DARKGREY_2,      // BG
            DARKGREY,        // Sub BG
            WHITE,           // Text
            LIGHTGREY,       // Sub Text
            primary,         // Highlight
            dimmedColor,     // Border
            RED              // Alert
        };
        saveSettings();
    }

    void begin() {
        // pinMode(TFT_BL_PIN, OUTPUT);
        // Cấu hình PWM cho độ sáng 
        // ledcAttach(TFT_BL_PIN, 5000, 8); // Freq: 5KHz, Res: 8bit

        loadSettings();
        applyTheme(currentMode);
        // applyBrightness(brightness);
    }

    // void setBrightness(int val) {
    //     if (val < 5) val = 5; // Tránh tắt hẳn tối thui
    //     if (val > 255) val = 255;
    //     brightness = val;
    //     ledcWrite(TFT_BL_PIN, brightness);
    // }

    void applyTheme(ThemeMode mode) {
        currentMode = mode;
        if (mode == THEME_DARK) {
            color = {BLACK, DARKGREY, WHITE, LIGHTGREY, BLUE, DARKGREY, RED};
        } 
        else if (mode == THEME_LIGHT) {
            color = {WHITE, LIGHTGREY, BLACK, DARKGREY, ORANGE, LIGHTGREY, RED};
        }
        else if (mode == THEME_CUSTOM) {
            // Load từ SD card hoặc Preferences nếu muốn custom sâu
            setCustomTheme(customPrimaryColor);
        }
    }

    void saveSettings() {
        prefs.begin("display_cfg", false);                  
        prefs.putInt("mode", (int)currentMode);             // Lưu kiểu theme
        // prefs.putInt("bri", brightness);                 // Lưu độ sáng
        prefs.putString("wall", wallpaperPath);             // Lưu hình nền
        prefs.putUShort("cust_col", customPrimaryColor);    // Lưu màu custom
        prefs.putBool("ss_en", screensaverEnabled);         // Lưu cấu hình screensaver
        prefs.putInt("ss_type", screensaverType);           // Lưu kiểu screensaver
        prefs.putInt("ss_time", screensaverTimeout);        // Lưu timeout screensaver
        prefs.end();
    }

    void loadSettings() {
        prefs.begin("display_cfg", true);
        currentMode = (ThemeMode)prefs.getInt("mode", 0);
        // brightness = prefs.getInt("bri", 200);
        wallpaperPath = prefs.getString("wall", "");
        customPrimaryColor = prefs.getUShort("cust_col", ORANGE);
        screensaverEnabled = prefs.getBool("ss_en", true);
        screensaverType = prefs.getInt("ss_type", 0);
        screensaverTimeout = prefs.getInt("ss_time", 30);
        prefs.end();

        applyTheme(currentMode);   
    }
};

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    // Nếu bạn vẽ lên Sprite (img) trong AppManager
    // Bạn cần access được biến 'img' ở đây. 
    // Cách đơn giản nhất cho người mới: Vẽ trực tiếp lên màn hình TFT, sau đó vẽ Menu đè lên (nhưng Menu phải transparent).

    // Tuy nhiên, cách tốt nhất với cấu trúc của bạn là vẽ vào Sprite 'img'.
    // Giả sử biến 'img' (TFT_eSprite) là public hoặc có thể truy cập:
    extern TFT_eSprite img; // Khai báo extern để trỏ tới biến img bên AppManager
    img.pushImage(x, y, w, h, bitmap);
    return 1;   
}

// Khai báo global để dùng mọi nơi
extern ThemeManager theme;

#endif