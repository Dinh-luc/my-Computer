#ifndef _PAGE_DISPLAY_H
#define _PAGE_DISPLAY_H

#include "SettingPage.h"

#include "Helpers/Color.h"
#include "Helpers/Storage.h"
#include "Helpers/Theme.h"

extern Storage storage;
extern ThemeManager theme;

enum DisplayPageState {
    STATE_NAVIGATE,     // Lên/Xuống chọn dòng, Trái thoát ra Menu chính
    STATE_EDIT_ITEM,    // Trái/Phải chỉnh giá trị (Slider, Radio)
    STATE_PICK_COLOR,   // Đang mở bảng màu Custom
    STATE_PICK_BR       // Đang mở BR
};

class Page_Display : public SettingPage {
private:
    int selectedIndex = 0;
    int scrollOffset = 0;
    const int itemsPerPage = 5; // Số dòng hiển thị tối đa trên màn hình
    const int totalItems = 7;   // Tổng số setting

    DisplayPageState pageState = STATE_NAVIGATE;

    // --- Color Picker Logic ---
    int pickerIndex = 0; 
    
    // Để chọn hình nền
    bool isPickingWallpaper = false;
    String tempPathList[10]; 
    int wallFileCount = 0;
    int wallIndex = 0;

public:
    const char* getName() override { return "DISPLAY"; }

    void init() override {
        
    }

    void update() override {
        
    }

    void drawDetails(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) override {
        // Tiêu đề
        spr->setTextColor(theme.color.textSub);
        spr->setCursor(x + 5, y + 5);
        spr->print("DISPLAY & THEME");
        spr->drawFastHLine(x, y + 15, w, theme.color.border);

        // Xử lý Popup chọn màu (Vẽ đè lên tất cả nếu đang active)
        if (pageState == STATE_PICK_COLOR) {
            drawColorPicker(spr, x, y + 20, w, h - 20);
            return;
        }

        int lineH = 18;
        int startY = y + 20;

        // Nếu đang chọn hình nền thì vẽ giao diện chọn hình
        if (isPickingWallpaper) {
            drawWallpaperPicker(spr, x, startY, w, h);
            return;
        }

        if (totalItems > itemsPerPage) {
            // Tính toán cuộn danh sách
            if (selectedIndex >= scrollOffset + itemsPerPage) {
                scrollOffset = selectedIndex - itemsPerPage + 1;
            } 
            else if (selectedIndex < scrollOffset) {
                scrollOffset = selectedIndex;
            }
        } else {
            scrollOffset = 0;
        }

        // Vẽ Menu chính
        for(int i=0; i < itemsPerPage && (i + scrollOffset < totalItems); i++) {
            int idx = i + scrollOffset;
            // if (idx >= totalItems) break;

            int lineY = startY + (i * lineH);
            bool isSelected = (idx == selectedIndex);

            if (idx == 3) {
                drawItem(spr, idx, x + 5, lineY + 4, w - 10, isSelected);
                continue;
            }

            if (isSelected && isFocused) {
                // EDIT thì viền màu Highlight, nền vẫn tối để tập trung vào control
                if (pageState == STATE_EDIT_ITEM) {
                    spr->drawRect(x + 2, lineY, w - 4, lineH, theme.color.highlight);
                } else {
                    // Nếu đang NAVIGATE thì fill nền nhẹ
                    spr->fillRect(x + 2, lineY, w - 4, lineH, theme.color.border);
                }
            }

            drawItem(spr, idx, x + 5, lineY + 4, w - 10, isSelected);
        }
    }

    bool handleInput(char key) override {
        // --- A. KHI ĐANG CHỌN MÀU (POPUP) ---
        if (pageState == STATE_PICK_COLOR) {
            if (key == 'L') { // Trái
                pickerIndex--;
                if (pickerIndex < 0) pickerIndex = 11;
            }
            else if (key == 'R') { // Phải
                pickerIndex++;
                if (pickerIndex > 11) pickerIndex = 0;
            }
            else if (key == 'U') { // Lên
                pickerIndex -= 4;
                if (pickerIndex < 0) pickerIndex += 12;
            }
            else if (key == 'D') { // Xuống
                pickerIndex += 4;
                if (pickerIndex > 11) pickerIndex -= 12;
            }
            else if (key == 'O') { // OK -> Apply màu
                theme.setCustomTheme(PALETTE_COLORS[pickerIndex]);
                pageState = STATE_NAVIGATE; // Thoát ra
            }
            else if (key == '#') { // Cancel
                pageState = STATE_NAVIGATE;
            }
            return true; 
        }

        // --- B. KHI ĐANG CHỈNH GIÁ TRỊ (EDIT MODE) ---
        if (pageState == STATE_EDIT_ITEM) {
            if (key == 'L' || key == 'R') {
                adjustValue(key == 'R' ? 1 : -1);
            }
            else if (key == 'O' || key == '#') {
                // Nếu đang ở dòng Theme mà chọn Custom -> Nhảy vào chọn màu
                if (selectedIndex == 0 && theme.currentMode == THEME_CUSTOM && key == 'O') {
                    pageState = STATE_PICK_COLOR;
                    pickerIndex = 0;
                } 
                // else if (selectedIndex == 4 && isPickingWallpaper && key == 'O') {
                //     isPickingWallpaper
                // }
                else {
                    pageState = STATE_NAVIGATE; // Quay về chế độ cuộn
                }
            }
            return true; // Chặn input
        }

        // --- C. KHI ĐANG CUỘN DANH SÁCH (NAVIGATE MODE) ---
        if (pageState == STATE_NAVIGATE) {
            if (key == 'U') {
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = 0;
                if (selectedIndex < scrollOffset) scrollOffset = selectedIndex; // Cuộn lên
            }
            else if (key == 'D') {
                selectedIndex++;
                if (selectedIndex >= totalItems) selectedIndex = totalItems - 1;
                if (selectedIndex >= scrollOffset + itemsPerPage) scrollOffset = selectedIndex - itemsPerPage + 1; // Cuộn xuống
            }
            else if (key == 'R' || key == 'O') { 
                // Nút Phải hoặc OK để VÀO chế độ sửa
                pageState = STATE_EDIT_ITEM;
            }
            else if (key == 'L') {
                // Nút Trái để thoát ra Menu chính (SettingsApp xử lý)
                return false; 
            }
        }
        return true;
    }

private:
    void drawItem(TFT_eSprite* spr, int idx, int x, int y, int w, bool selected) {
        spr->setTextColor(selected ? theme.color.highlight : theme.color.text);
        spr->setCursor(x, y);

        switch (idx) {
            case 0: 
                spr->print("Theme: ");
                break;

            case 1:
                drawRadio(spr, x , y , "Dark", theme.currentMode == THEME_DARK);
                drawRadio(spr, x + 40, y , "Light", theme.currentMode == THEME_LIGHT);
                break;

            case 2: // BRIGHTNESS (Slider)
                spr->print("Bright: ");
                break;

            case 3:
                // Vẽ khung slider
                spr->drawRect(x + 15, y, 70, 6, theme.color.textSub);
                // Vẽ ruột
                // int fillW = map(theme.brightness, 5, 255, 0, 58);
                // spr->fillRect(x + 51, y+3, fillW, 4, theme.color.highlight);
                break;

            case 4: // WALLPAPER
                spr->print("Wallppr: ");
                spr->setTextColor(theme.color.textSub);
                if (theme.wallpaperPath == "") {
                    spr->print("None");
                    isPickingWallpaper = false;
                }
                else {
                    spr->print("Image");
                    isPickingWallpaper = true;
                }
                break;

            case 5: // TIMEOUT
                spr->print("Sleep: ");
                if (theme.screensaverEnabled) spr->print(String(theme.screensaverTimeout) + "s");
                else spr->print("Never");
                break;

            case 6: // SAVER TYPE
                spr->print("Saver: ");
                if (theme.screensaverType == 0) spr->print("Clock");
                else if (theme.screensaverType == 1) spr->print("Images");
                else spr->print("Black");
                break;
        }
    }

    void drawRadio(TFT_eSprite* spr, int x, int y, const char* label, bool active) {
        // if (theme.currentMode == THEME_CUSTOM) return; 
        
        uint16_t color = active ? theme.color.highlight : theme.color.textSub;
        // Vẽ vòng tròn
        spr->drawCircle(x + 3, y + 4, 3, color);
        if (active) spr->fillCircle(x + 3, y + 4, 1, color);
        
        // Vẽ chữ
        spr->setTextColor(active ? theme.color.text : theme.color.textSub);
        spr->setCursor(x + 10, y);
        spr->print(label);
    }

    void drawColorPicker(TFT_eSprite* spr, int x, int y, int w, int h) {
        // Vẽ khung nền popup
        spr->fillRoundRect(x, y, w, h, 4, DARKGREY_2);
        spr->drawRoundRect(x, y, w, h, 4, WHITE);
        
        spr->setTextColor(WHITE);
        spr->setTextDatum(TC_DATUM);
        // spr->drawString("Pick Color", x + w/2, y + 5);
        spr->drawString("[O]Apply [#]Exit", x + w/2, y + 5);
        spr->setTextDatum(TL_DATUM);

        int gridX = x + 3;
        int gridY = y + 20;
        int boxSize = 20;
        int gap = 5;

        for (int i = 0; i < 12; i++) {
            int row = i / 4;
            int col = i % 4;
            int bx = gridX + col * (boxSize + gap);
            int by = gridY + row * (boxSize + gap);

            // Vẽ ô màu
            spr->fillRect(bx, by, boxSize, boxSize, PALETTE_COLORS[i]);
            
            // Vẽ viền chọn
            if (i == pickerIndex) {
                spr->drawRect(bx - 2, by - 2, boxSize + 4, boxSize + 4, WHITE);
            }
        }
    }

    void adjustValue(int dir) {
        switch (selectedIndex) {
            case 1: // Theme
                {
                    int m = (int)theme.currentMode + dir;
                    if (m < 0) m = 2; 
                    if (m > 2) m = 0;
                    theme.applyTheme((ThemeMode)m);
                }
                break;
            case 2: // Brightness
                {
                    // int b = theme.brightness + (dir * 25);
                    // theme.setBrightness(b);
                }
                break;
            case 4: // Wallpaper (Toggle On/Off đơn giản)
                if (theme.wallpaperPath != "") theme.wallpaperPath = "";
                else theme.wallpaperPath = "/SystemConfig/background/ameno-element.jpg"; 
                theme.saveSettings();
                break;
            case 5: // Timeout
                theme.screensaverTimeout += (dir * 15);
                if (theme.screensaverTimeout < 15) {
                    theme.screensaverEnabled = false;
                    theme.screensaverTimeout = 15;
                } else {
                    theme.screensaverEnabled = true;
                }
                if (theme.screensaverTimeout > 120) theme.screensaverTimeout = 120;
                theme.saveSettings();
                break;
             case 6: // Type
                 theme.screensaverType += dir;
                 if (theme.screensaverType < 0) theme.screensaverType = 2;
                 if (theme.screensaverType > 2) theme.screensaverType = 0;
                 theme.saveSettings();
                 break;
        }
    }

    void scanWallpapers() {
        wallFileCount = 0;
        if (!storage.exists("/SystemConfig/background")) storage.createDir("/SystemConfig/background");
        
        fs::File root = storage.openFile("/SystemConfig/background");
        fs::File file = root.openNextFile();
        while(file && wallFileCount < 10) {
            String name = String(file.name());
            // Chỉ lấy file ảnh (Logic đơn giản)
            if (name.endsWith(".jpg") || name.endsWith(".bmp") || name.endsWith(".png")) {
                tempPathList[wallFileCount++] = name;
            }
            file = root.openNextFile();
        }
        root.close();
    }

    void drawWallpaperPicker(TFT_eSprite* spr, int x, int y, int w, int h) {
        spr->fillRect(x, y, w, h - 20, theme.color.bg);
        spr->setTextColor(theme.color.highlight);
        spr->drawString("Select Wallpaper:", x + 5, y);
        
        for (int i=0; i < wallFileCount; i++) {
            if (i == wallIndex) spr->setTextColor(theme.color.highlight);
            else spr->setTextColor(theme.color.text);
            
            spr->setCursor(x + 10, y + 15 + (i*12));
            spr->print(tempPathList[i]);
        }

        // Nút Remove/None ở cuối
        if (wallIndex == wallFileCount) spr->setTextColor(theme.color.highlight);
        else spr->setTextColor(theme.color.alert);
        spr->setCursor(x + 10, y + 15 + (wallFileCount*12));
        spr->print("[Remove / None]");
    }
};

#endif