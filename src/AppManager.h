#ifndef _APP_MANAGER_H
#define _APP_MANAGER_H

#include <TJpg_Decoder.h>

#include "AppInterface.h"

#include "Helpers/Keypad_4x4.h"
#include "Helpers/Color.h"
#include "Helpers/Theme.h"
#include "Helpers/Screensaver.h"
#include "Helpers/Storage.h"
#include "Helpers/Icon.h"
#include "Helpers/SystemAssets.h"

#include "StatusBar.h"
#include "SettingsApp.h"

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

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (menuSpritePtr != nullptr) menuSpritePtr->pushImage(x, y, w, h, bitmap);
    return 1; 
}

class AppManager {
private:
    AppInterface** apps;                
    int appCount = 0;                   
    int selectedIndex = 0;              
    AppInterface* activeApp = nullptr;  

    StatusBar statusBar;     
    SettingsApp settingsApp;   
    
    TFT_eSprite img = TFT_eSprite(&tft); 
    TFT_eSprite bgCache = TFT_eSprite(&tft); 
    
    // --- BỘ NHỚ ĐỆM CHO ICON ---
    TFT_eSprite** iconSprites;

    bool isBgLoaded = false;
    String lastLoadedPath = "";

public:
    AppManager(AppInterface* appList[], int count) {
        apps = appList;
        appCount = count;
        // Cấp phát mảng con trỏ chứa Icon (Lưu trong RAM)
        iconSprites = new TFT_eSprite*[appCount];
        for(int i = 0; i < appCount; i++) {
            iconSprites[i] = nullptr;
        }
    }

    void init() {
        img.setColorDepth(16);
        img.createSprite(width, height - BAR_HEIGHT); 

        bgCache.setColorDepth(16);
        bgCache.createSprite(width, height - BAR_HEIGHT);

        statusBar.init();
        loadBackground();
        renderMenu();
    }

    void loadBackground() {
        menuSpritePtr = &bgCache; 
        TJpgDec.setCallback(tft_output);
        TJpgDec.setSwapBytes(true); 

        String bgPath = theme.wallpaperPath;

        // 1. NẾU KHÔNG CÓ HÌNH NỀN (CHỮ RỖNG) -> TÔ MÀU TRƠN VÀ THOÁT NGAY
        if (bgPath == "") {
            bgCache.fillSprite(theme.color.bg);
            isBgLoaded = true;
            return;
        }

        // 2. NẾU LÀ HÌNH NỀN TỪ MẢNG HEX (ROM)
        if(bgPath.startsWith("ROM:")) {
            if (bgPath == "ROM: Default 1") {
                Serial.println("Đang tải hình nền mặc định 1 từ ROM...");
                TJpgDec.drawJpg(0, 0, bg1, bg1_len); 
            }
            else if (bgPath == "ROM: Default 2") {
                Serial.println("Đang tải hình nền mặc định 2 từ ROM...");
                TJpgDec.drawJpg(0, 0, bg2, bg2_len); 
            }
            else if (bgPath == "ROM: Default 3") {
                Serial.println("Đang tải hình nền mặc định 3 từ ROM...");
                TJpgDec.drawJpg(0, 0, bg3, bg3_len); 
            }
        }
        // 3. NẾU LÀ HÌNH NỀN TỪ THẺ NHỚ (SD_MMC)
        else {
            String fullPath = "/SystemConfig/background/" + bgPath;
            
            // Chỉ đọc nếu đường dẫn KHÔNG PHẢI là thư mục
            if (storage.exists(fullPath) && !SD_MMC.open(fullPath).isDirectory()) {
                // BẮT BUỘC DÙNG drawFsJpg ĐỂ TRUYỀN Ổ ĐĨA SD_MMC VÀO
                TJpgDec.drawFsJpg(0, 0, fullPath.c_str(), SD_MMC);
            } else {
                Serial.println("Lỗi: Không tìm thấy hình nền hợp lệ trên thẻ SD!");
                bgCache.fillSprite(theme.color.bg);
            }
        }
        
        isBgLoaded = true;
    }

    void renderMenu() {
        // Đổ nền từ bộ nhớ đệm
        if (isBgLoaded) bgCache.pushToSprite(&img, 0, 0);
        else img.fillSprite(theme.color.bg);

        int appsPerPage = 6; 
        int currentPage = selectedIndex / appsPerPage;
        int totalPages = (appCount + appsPerPage - 1) / appsPerPage;
        
        int startIdx = currentPage * appsPerPage;
        int endIdx = startIdx + appsPerPage;
        if (endIdx > appCount) endIdx = appCount;

        int cellW = width / 3;             
        int cellH = (height - BAR_HEIGHT) / 2; 
        int startY = 5; 

        img.setTextDatum(TC_DATUM); 

        for (int i = startIdx; i < endIdx; i++) {
            int posOnPage = i - startIdx;
            int col = posOnPage % 3;
            int row = posOnPage / 3;

            int cx = col * cellW + (cellW / 2);
            int cy = startY + row * cellH + (cellH / 2) - 5; 

            bool isSelected = (i == selectedIndex);

            if (isSelected) {
                img.drawRect(cx - 17, cy - 17, 34, 34, theme.color.highlight);
            }

            // --- KIỂM TRA & TẢI ICON VÀO RAM ---
            if (iconSprites[i] == nullptr) {
                // Nếu Icon chưa có trong RAM, tạo mới một Sprite 32x32
                iconSprites[i] = new TFT_eSprite(&tft);
                iconSprites[i]->setColorDepth(16);
                iconSprites[i]->createSprite(32, 32);
                iconSprites[i]->fillSprite(MAGENTA); 
                
                String appName = String(apps[i]->getName());
                String iconName = String(apps[i]->getIconColor()); 
                iconName.replace(" ", "_"); 
                String iconPath = "/SystemConfig/icon/" + iconName + ".png";
                bool iconLoaded = false;

                if (storage.exists(iconPath)) {
                    iconLoaded = IconHelper::drawIconFromSD(iconSprites[i], iconPath, 0, 0);
                }

                if (!iconLoaded) {
                    if (appName == "SETTINGS") {
                        IconHelper::drawIconFromROM(iconSprites[i], icon_setting, icon_setting_len, 0, 0);
                    }
                    else if (appName == "TETRIS") {
                        IconHelper::drawIconFromROM(iconSprites[i], icon_tetris, icon_tetris_len, 0, 0);
                    }
                    else if (appName == "SNAKE") {
                        IconHelper::drawIconFromROM(iconSprites[i], icon_snake, icon_snake_len, 0, 0);
                    }
                    else if (appName == "DOWNLOAD") {
                        IconHelper::drawIconFromROM(iconSprites[i], icon_download, icon_download_len, 0, 0);
                    }
                    else {
                        IconHelper::drawIconFromROM(iconSprites[i], icon_default, icon_default_len, 0, 0);
                    }
                }
            }

            // Đẩy trực tiếp Icon từ RAM ra màn hình 
            iconSprites[i]->pushToSprite(&img, cx - 16, cy - 16, MAGENTA);

            img.setTextColor(isSelected ? theme.color.highlight : theme.color.text);
            img.drawString(String(apps[i]->getName()), cx, cy + 19);
        }
        
        img.setTextDatum(TL_DATUM); 

        if (totalPages > 1) {
            int dotSpace = 10;
            int startX = (width - ((totalPages - 1) * dotSpace)) / 2;
            int dotY = height - BAR_HEIGHT - 6;

            for(int p = 0; p < totalPages; p++) {
                if (p == currentPage) img.fillCircle(startX + p * dotSpace, dotY, 3, theme.color.highlight);
                else img.fillCircle(startX + p * dotSpace, dotY, 2, theme.color.textSub);
            }
        }

        statusBar.update();
        img.pushSprite(0, BAR_HEIGHT);
    }

    void handleInput(char key) {
        lastInteraction = millis();

        if (isScreensaverActive) {
            isScreensaverActive = false;
            if (activeApp == nullptr) {
                renderMenu();
                statusBar.init();
            }
            return;
        }

        if (activeApp != nullptr) {
            if (key == KEY_EXIT) {
                activeApp->stop();
                activeApp = nullptr;
                
                img.createSprite(width, height - BAR_HEIGHT);
                loadBackground();
                renderMenu();
            } else {
                activeApp->handleInput(key);
            }
        } else {
            bool needRedraw = false;
            switch (key) {
                case KEY_LEFT:
                    selectedIndex--;
                    if (selectedIndex < 0) selectedIndex = appCount - 1;
                    needRedraw = true;
                    break;
                    
                case KEY_RIGHT:
                    selectedIndex++;
                    if (selectedIndex >= appCount) selectedIndex = 0;
                    needRedraw = true;
                    break;

                case KEY_UP:
                    selectedIndex -= 3;
                    if (selectedIndex < 0) selectedIndex = (selectedIndex + appCount) % appCount;
                    needRedraw = true;
                    break;
                
                case KEY_DOWN:
                    selectedIndex += 3;
                    if (selectedIndex >= appCount) selectedIndex = selectedIndex % appCount;
                    needRedraw = true;
                    break;
                    
                case KEY_SELECT:
                    launchApp(apps[selectedIndex]);
                    return; 
                
                case KEY_SETTINGS:
                    launchApp(&settingsApp);
                    break;
            }

            if (needRedraw) renderMenu();
        }
    }

    void launchApp(AppInterface* app) {
        img.deleteSprite(); 

        if (app->showStatusBar()) {
            tft.fillRect(0, BAR_HEIGHT, width, height - BAR_HEIGHT, BLACK);
            statusBar.forceRedraw(); 
        } else {
            tft.fillScreen(BLACK);
        }
        
        activeApp = app;
        activeApp->start();
    }

    void update() {
        if (theme.screensaverEnabled && !isScreensaverActive) {
            if (millis() - lastInteraction > (theme.screensaverTimeout * 1000)) {
                isScreensaverActive = true;
                tft.fillScreen(BLACK); 
                prevDay = -1; 
                prevSec = -1; 
                prevMinute = -1;
            }
        }

        if (isScreensaverActive) {
            if (theme.screensaverType == 0) drawClockScreensaver(); 
            else if (theme.screensaverType == 2) tft.fillScreen(RED); 
            return; 
        }

        if (activeApp == nullptr) statusBar.update();
        else {
            if(activeApp->showStatusBar()) statusBar.update();
            activeApp->update();
        }   
    }
};

#endif