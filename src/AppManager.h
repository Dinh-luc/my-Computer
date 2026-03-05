#ifndef _APP_MANAGER_H
#define _APP_MANAGER_H

#include <TJpg_Decoder.h>

#include "AppInterface.h"

#include "Helpers/Keypad_4x4.h"
#include "Helpers/Color.h"
#include "Helpers/Theme.h"
#include "Helpers/Screensaver.h"
#include "Helpers/Storage.h"

#include "StatusBar.h"
#include "SettingsApp.h"

// Định nghĩa phím
#define KEY_EXIT        '#' 
#define KEY_UP          'U'
#define KEY_DOWN        'D'
#define KEY_LEFT        'L'
#define KEY_RIGHT       'R'
#define KEY_SELECT      'O'
#define KEY_SETTINGS    'A'

extern int width;
extern int height;

extern Storage storage;
extern ThemeManager theme;

static unsigned long lastInteraction = 0;
bool isScreensaverActive = false;

TFT_eSprite* menuSpritePtr = nullptr;

// Hàm Callback: TJpg_Decoder sẽ gọi hàm này liên tục để đẩy từng khối pixel của ảnh ra
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (menuSpritePtr != nullptr) {
        // Đẩy khối pixel vào Sprite của Menu
        menuSpritePtr->pushImage(x, y, w, h, bitmap);
    }
    return 1; 
}

class AppManager {
private:
    AppInterface** apps;                // Mảng chứa danh sách các app
    int appCount = 0;                   // Số lượng app
    int selectedIndex = 0;              // Chỉ mục app đang chọn
    AppInterface* activeApp = nullptr;  // App đang chạy

    StatusBar statusBar;     
    SettingsApp settingsApp;   
    
    TFT_eSprite img = TFT_eSprite(&tft); 

    // --- THÊM SPRITE LÀM BỘ NHỚ ĐỆM ẢNH NỀN ---
    TFT_eSprite bgCache = TFT_eSprite(&tft); 
    bool isBgLoaded = false;
    String lastLoadedPath = "";

public:
    AppManager(AppInterface* appList[], int count) {
        apps = appList;
        appCount = count;
    }

    void init() {
        // Khởi tạo Sprite khi bật nguồn
        img.setColorDepth(16);
        img.createSprite(width, height - BAR_HEIGHT); 

        bgCache.setColorDepth(16);
        bgCache.createSprite(width, height - BAR_HEIGHT);

        statusBar.init();
        loadBackground();
        renderMenu();
    }

    // --- LOAD ẢNH 1 LẦN VÀO RAM ---
    void loadBackground() {
        // Nếu không có hình nền
        if (theme.wallpaperPath == "") {
            isBgLoaded = false;
            lastLoadedPath = "";
            return;
        }

        // Nếu ảnh này đã được load rồi thì không làm gì cả (Tối ưu CPU)
        if (isBgLoaded && lastLoadedPath == theme.wallpaperPath) {
            return; 
        }

        if (storage.exists(theme.wallpaperPath)) {
            // Trỏ callback vào sprite đệm
            menuSpritePtr = &bgCache; 

            TJpgDec.setCallback(tft_output);
            TJpgDec.setSwapBytes(true); 

            uint16_t jpgW = 0, jpgH = 0;
            TJpgDec.getFsJpgSize(&jpgW, &jpgH, theme.wallpaperPath.c_str(), SD_MMC);
            
            // Vẽ ảnh gốc vào bgCache
            TJpgDec.drawFsJpg(0, 0, theme.wallpaperPath.c_str(), SD_MMC);
            
            isBgLoaded = true;
            lastLoadedPath = theme.wallpaperPath;
        } else {
            isBgLoaded = false;
        }
    }

    // Hàm vẽ toàn bộ Menu dùng Sprite (One-pass render)
    void renderMenu() {

        // VẼ NỀN TỪ CACHE (Tốc độ ánh sáng!)
        if (isBgLoaded) {
            // Copy toàn bộ pixel từ bgCache sang img
            bgCache.pushToSprite(&img, 0, 0);
        } else {
            img.fillSprite(theme.color.bg);
        }

        // VẼ TIÊU ĐỀ (Có hiệu ứng Drop Shadow để dễ đọc trên nền ảnh)
        img.setTextSize(2);
        img.setTextColor(BLACK); // Bóng đen
        img.drawString("MENU APPS", 27, 12); 
        img.setTextColor(theme.color.highlight); // Chữ màu
        img.drawString("MENU APPS", 25, 10); 

        // VẼ DANH SÁCH APP
        img.setTextSize(1);
        for (int i = 0; i < appCount; i++) {
            int lineY = 40 + (i * 20); 
            bool isSelected = (i == selectedIndex);
            
            if (isSelected) {
                // Vẽ một dải màu làm nền nổi bật cho mục đang chọn
                img.fillRect(10, lineY - 4, width - 20, 16, theme.color.highlight);
                img.setTextColor(theme.color.bg); 
                img.drawString(">", 15, lineY);
                img.drawString(apps[i]->getName(), 30, lineY);
            }
            else {
                img.setTextColor(BLACK);
                img.drawString(apps[i]->getName(), 31, lineY + 1); 
                
                img.setTextColor(theme.color.text);
                img.drawString(apps[i]->getName(), 30, lineY);     
            }
        }

        // ĐẨY RA MÀN HÌNH
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
                // loadBackground();
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
                
                img.createSprite(width, height - BAR_HEIGHT);
                bgCache.createSprite(width, height - BAR_HEIGHT);
                
                // Vẽ lại Menu
                isBgLoaded = false;
                loadBackground();
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
                    bgCache.deleteSprite();
                    isBgLoaded = false;
                    
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
        bgCache.deleteSprite();
        isBgLoaded = false;

        if (app->showStatusBar()) {
            // App mới có hiện Bar
            tft.fillRect(0, BAR_HEIGHT, width, height - BAR_HEIGHT, BLACK);
            statusBar.forceRedraw(); 
        } 
        else {
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