#ifndef _SETTINGS_APP_H
#define _SETTINGS_APP_H

#include "AppInterface.h"
#include "StatusBar.h"
#include "Settings/SettingPage.h"

#include "Helpers/Color.h"
#include "Helpers/Theme.h"

#include "Settings/Page_Audio.h"
#include "Settings/Page_Wifi.h"
#include "Settings/Page_Storage.h"
#include "Settings/Page_Display.h"
#include "Settings/Page_Input.h"
#include "Settings/Page_Bluetooth.h"
#include "Settings/Page_RTC.h"
#include "Settings/Page_SystemInfor.h"

extern int width;
extern int height;
extern ThemeManager theme;

enum FocusState {
    FOCUS_MENU_LEFT,
    FOCUS_CONTENT_RIGHT
};

// --- QUẢN LÝ CÁC TRANG ---
Page_Audio pageAudio;
Page_Wifi pageWifi;
Page_Storage pageStorage;
Page_Display pageDisplay;
Page_Input pageInput;
Page_Bluetooth pageBluetooth;
Page_RTC pageRTC;
Page_SystemInfor pageSystemInfor;

// Danh sách con trỏ đến các trang để dễ duyệt vòng lặp
SettingPage* pages[] = { 
    &pageSystemInfor,
    &pageWifi, 
    &pageBluetooth,
    &pageAudio,
    &pageDisplay,
    &pageStorage,
    &pageInput,
    &pageRTC
}; 
const int pageCount = sizeof(pages) / sizeof(pages[0]);
int selectedIndex = 0;

class SettingsApp : public AppInterface {
private:
    TFT_eSprite img = TFT_eSprite(&tft);

    FocusState currentFocus = FOCUS_MENU_LEFT;
    const int SPLIT_X = 55; // Vị trí đường kẻ dọc (Menu bên trái rộng 55px)

public:
    const char* getName() override { 
        return "SETTINGS"; 
    }

    uint16_t getIconColor() override { 
        return TFT_LIGHTGREY; 
    }

    bool showStatusBar() override { 
        return true; 
    }

    void start() override {
        img.setColorDepth(16);
        img.createSprite(width, height - BAR_HEIGHT); 
        
        // Gọi init cho các trang con nếu cần
        for(int i=0; i<pageCount; i++) pages[i]->init();
        currentFocus = FOCUS_MENU_LEFT;
        drawUI();
    }

    void stop() override {
        img.deleteSprite();
    }

    void update() override {
        pages[selectedIndex]->update();
        if (currentFocus == FOCUS_CONTENT_RIGHT) {
            drawUI(); 
        }
    }

    void handleInput(char key) override {
        bool needRedraw = false;

        // Menu bên trái
        if (currentFocus == FOCUS_MENU_LEFT) {
            if (key == 'U') { // UP
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = pageCount - 1;
                needRedraw = true;
            }
            else if (key == 'D') { // DOWN
                selectedIndex++;
                if (selectedIndex >= pageCount) selectedIndex = 0;
                needRedraw = true;
            }
            // Các phím còn lại: Truyền sang cho trang bên phải xử lý
            else if (key == 'O' || key == 'R') {
                currentFocus = FOCUS_CONTENT_RIGHT;
                needRedraw = true; 
            }
        }

        // Nội dung bên phải
        else if (currentFocus == FOCUS_CONTENT_RIGHT) {
            bool keepFocus = pages[selectedIndex]->handleInput(key);
            // Phím quay lại menu bên trái
            if (!keepFocus) { 
                currentFocus = FOCUS_MENU_LEFT;
                needRedraw = true;
            }
            else needRedraw = true;
        }

        if (needRedraw) drawUI();
    }

private:
    void drawUI() {
        img.fillSprite(theme.color.bg);

        // VẼ MENU BÊN TRÁI
        // Nền menu
        img.fillRect(0, 0, SPLIT_X, height - BAR_HEIGHT, theme.color.subBg); 
        
        // Vẽ danh sách trang
        int startY = 5;         // Y bắt đầu vẽ mục đầu tiên
        int listLimit = 6;      // Số mục tối đa hiển thị cùng lúc
        int indexOffset = 0;    // Dịch chuyển index để cuộn danh sách

        if(selectedIndex >= listLimit) {
            indexOffset = selectedIndex - listLimit + 1;
        }

        for (int i = 0; i < listLimit && (i + indexOffset < pageCount); i++ ) {
            int idx = i +indexOffset;
            int lineY = startY + (i * 20);

            if (idx == selectedIndex) {
                // Nếu đang Focus bên trái 
                if (currentFocus == FOCUS_MENU_LEFT) {
                    img.fillRect(0, lineY - 3, SPLIT_X, 14, WHITE);
                    img.setTextColor(BLACK);
                } 
                // Nếu đang Focus bên phải -> Màu Xám sáng 
                else {
                    img.fillRect(0, lineY - 3, SPLIT_X, 14, LIGHTGREY);
                    img.setTextColor(BLACK);
                }
            } 
            else {
                img.setTextColor(theme.color.text); 
            }

            img.setCursor(5, lineY);
            img.print(pages[idx]->getName());
        }

        // VẼ CHI TIẾT BÊN PHẢI 
        pages[selectedIndex]->drawDetails(&img, SPLIT_X + 2, 0, 160 - SPLIT_X - 2, height - BAR_HEIGHT, (currentFocus == FOCUS_CONTENT_RIGHT));

        img.pushSprite(0, BAR_HEIGHT);
    }

};

#endif