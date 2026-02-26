#ifndef _APP_MANAGER_H
#define _APP_MANAGER_H

#include <TJpg_Decoder.h>

#include "AppInterface.h"

#include "Helpers/Keypad_4x4.h"
#include "Helpers/Color.h"
#include "Helpers/Theme.h"
#include "Helpers/Screensaver.h"

#include "StatusBar.h"
#include "SettingsApp.h"

// Định nghĩa phím
#define KEY_EXIT        '#' 
#define KEY_UP          'U'
#define KEY_DOWN        'D'
#define KEY_SELECT      'O'
#define KEY_SETTINGS    'A'

extern int width;
extern int height;

static unsigned long lastInteraction = 0;
bool isScreensaverActive = false;

class AppManager {
private:
    AppInterface** apps;          // Mảng chứa danh sách các app
    int appCount = 0;              // Số lượng app
    int selectedIndex = 0;          // Chỉ mục app đang chọn
    AppInterface* activeApp = nullptr;    // App đang chạy

    StatusBar statusBar;     
    SettingsApp settingsApp;   

    TJpg_Decoder jpg = TJpg_Decoder();
    
    TFT_eSprite img = TFT_eSprite(&tft); 

public:
    AppManager(AppInterface* appList[], int count) {
        apps = appList;
        appCount = count;
    }

    void init() {
        // Khởi tạo Sprite khi bật nguồn
        img.setColorDepth(16);
        img.createSprite(width, height - BAR_HEIGHT); // Kích thước màn hình
        statusBar.init();
        renderMenu();
    }

    // Hàm vẽ toàn bộ Menu dùng Sprite (One-pass render)
    void renderMenu() {
        // 1. Xóa Sprite
        img.fillSprite(BLACK);

        // if (theme.wallpaperPath != "" && storage.exists(theme.wallpaperPath)) {
        //     // Cấu hình bộ giải mã JPG
        //     jpg.setJpgScale(1); // Mặc định tỉ lệ 1:1
        //     // jpg.setCallback(tft_output());
        //     jpg.setSwapBytes(true);

        //     // --- XỬ LÝ RESIZE (Tự động thu nhỏ nếu ảnh quá to) ---
        //     // Lấy kích thước ảnh trước khi vẽ
        //     uint16_t w = 0, h = 0;
        //     jpg.getJpgSize(&w, &h, theme.wallpaperPath.c_str());

        //     // Tính toán tỉ lệ thu nhỏ (Chỉ hỗ trợ 1, 2, 4, 8)
        //     // Ví dụ: Màn hình 160, Ảnh 320 -> Scale = 2
        //     int scale = 1;
        //     if (w > width || h > height) {
        //         if (w >= width * 8) scale = 8;
        //         else if (w >= width * 4) scale = 4;
        //         else if (w >= width * 2) scale = 2;
        //     }
        //     jpg.setJpgScale(scale);

        //     // Vẽ ảnh vào Sprite tại toạ độ 0,0
        //     // Lưu ý: TJpg_Decoder đọc trực tiếp từ SD
        //     jpg.drawJpg(0, 0, theme.wallpaperPath.c_str());
        // } 
        // else {
        //     // Nếu không có hình nền thì tô màu theme
        //     img.fillSprite(theme.color.bg);
        // }

        // // 2. Vẽ Tiêu đề (Phần này phải vẽ SAU hình nền để đè lên ảnh)
        // img.setCursor(25, 10);
        // // Nên vẽ thêm bóng chữ (Shadow) để dễ đọc trên hình nền
        // img.setTextColor(BLACK); 
        // img.drawString("MENU APPS", 26, 11); // Bóng đen
        // img.setTextColor(YELLOW);
        // img.drawString("MENU APPS", 25, 10); // Chữ vàng

        // 2. Vẽ Tiêu đề
        img.setCursor(25, 10);
        img.setTextColor(YELLOW);
        img.setTextSize(2);
        img.println("MENU APPS");

        // 3. Vẽ danh sách app
        img.setTextSize(1);
        for (int i = 0; i < appCount; i++) {
            int y = 35 + (i * 20); // Tăng khoảng cách dòng lên 20 cho thoáng
            
            bool isSelected = (i == selectedIndex);
            
            if (isSelected) {
                // Vẽ dấu mũi tên chọn
                img.setCursor(5, y);
                img.setTextColor(RED); 
                img.print(">");
                
                // Highlight tên game
                img.setTextColor(YELLOW);
            }
            else {
                img.setTextColor(WHITE);
            }
            
            img.setCursor(20, y);
            img.print(apps[i]->getName());
        }

        // 4. Đẩy ra màn hình
        statusBar.update();
        img.pushSprite(0, BAR_HEIGHT);
    }

    void handleInput(char key) {
        lastInteraction = millis();

        // --- XỬ LÝ THỨC DẬY (WAKE UP) ---
        if (isScreensaverActive) {
            isScreensaverActive = false;
            
            // Khôi phục độ sáng màn hình
            // theme.setBrightness(theme.brightness); 
            
            if (activeApp == nullptr) {
                renderMenu();
                statusBar.init();
            }
            
            return;
        }

        if (activeApp != nullptr) {
            // --- Đang trong app ---
            if (key == KEY_EXIT) {
                // Dừng app 
                activeApp->stop();
                activeApp = nullptr;
                
                // Tạo lại Sprite cho Menu
                img.createSprite(160, 128);
                
                // Vẽ lại Menu
                renderMenu();
            } else {
                activeApp->handleInput(key);
            }
        } else {
            // --- Đang ở Menu ---
            bool needRedraw = false;
            switch (key) {
                case KEY_UP:
                    selectedIndex--;
                    if (selectedIndex < 0) selectedIndex = appCount - 1;
                    needRedraw = true;
                    break;
                    
                case KEY_DOWN:
                    selectedIndex++;
                    if (selectedIndex >= appCount) selectedIndex = 0;
                    needRedraw = true;
                    break;
                    
                case KEY_SELECT:
                    // 1. Xóa Sprite Menu để giải phóng RAM cho Game
                    img.deleteSprite();
                    
                    // 2. Xóa màn hình thật về đen (tránh rác hình khi chuyển giao)
                    tft.fillScreen(BLACK);
                    
                    // 3. Khởi động Game
                    activeApp = apps[selectedIndex];
                    activeApp->start(); // Game sẽ tự tạo Sprite của nó trong hàm start()
                    return; // Thoát ngay, không vẽ lại menu
                
                case KEY_SETTINGS:
                    launchApp(&settingsApp);
                    break;
            }

            if (needRedraw) {
                renderMenu();
            }
        }
    }

    void launchApp(AppInterface* app) {
        // Xóa Sprite Menu để giải phóng RAM
        img.deleteSprite();

        if (app->showStatusBar()) {
            // App mới có hiện Bar
            tft.fillRect(0, BAR_HEIGHT, width, height - BAR_HEIGHT, BLACK);
            statusBar.forceRedraw(); 
        } 
        else {
            // App mới là Game Full màn hình 
            tft.fillScreen(BLACK);
        }
        
        // Gán activeApp và khởi động
        activeApp = app;
        activeApp->start();
    }

    void update() {
        // --- KIỂM TRA KÍCH HOẠT SCREENSAVER ---
        if (theme.screensaverEnabled && !isScreensaverActive) {
            if (millis() - lastInteraction > (theme.screensaverTimeout * 1000)) {
                isScreensaverActive = true;
                tft.fillScreen(BLACK); 
                prevDay = -1; 
                prevSec = -1;
                prevMinute = -1;
            }
        }

        // --- LOGIC HIỂN THỊ KHI ĐANG SLEEP ---
        if (isScreensaverActive) {
            if (theme.screensaverType == 0) {
                drawClockScreensaver(); // Hàm vẽ đồng hồ 
            }
            else if (theme.screensaverType == 1) {
                // drawImageScreensaver(); // Hàm slide ảnh
            }
            else if (theme.screensaverType == 2) {
                // theme.setBrightness(0); 
                tft.fillScreen(RED); 
            }
            return; 
        }

        if (activeApp == nullptr) {
            statusBar.update();
        }
        else {
            if(activeApp->showStatusBar()) {
                statusBar.update();
            }
            activeApp->update();
        }   
    }
};

#endif