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

    // --- THÊM CÁC BIẾN MỚI CHO SCROLL & MARQUEE ---
    int wallScrollOffset = 0;           // Vị trí cuộn của danh sách ảnh
    const int wallItemsPerPage = 5;     // Số dòng hiển thị tối đa trong menu ảnh
    int wallTextScrollOffset = 0;       // Vị trí cắt chữ để làm hiệu ứng chạy
    unsigned long lastWallTextUpdate = 0; // Bộ đếm thời gian chạy chữ

public:
    const char* getName() override { return "DISPLAY"; }

    void init() override {
        
    }

    void update() override {
        // Logic xử lý Chữ chạy (Marquee)
        if (pageState == STATE_PICK_BR && millis() - lastWallTextUpdate > 150) {
            // Lấy tên file đang chọn (hoặc dòng Remove)
            String currentName = (wallIndex == wallFileCount) ? "[Remove / None]" : tempPathList[wallIndex];
            
            // Chỉ chạy chữ nếu tên dài hơn 14 ký tự
            if (currentName.length() > 14) { 
                wallTextScrollOffset++;
                // Nếu chạy hết chữ -> Reset về đầu, delay 1 chút (âm 3)
                if (wallTextScrollOffset > (int)currentName.length() - 5) {
                    wallTextScrollOffset = -3; 
                }
            } else {
                wallTextScrollOffset = 0;
            }
            lastWallTextUpdate = millis();
        }
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
        if (pageState == STATE_PICK_BR) {
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
        // --- 1. KHI ĐANG CHỌN MÀU (POPUP) ---
        if (pageState == STATE_PICK_COLOR) {
            if (key == 'L') { 
                pickerIndex--; 
                if (pickerIndex < 0) pickerIndex = 11; 
            }
            else if (key == 'R') { 
                pickerIndex++; 
                if (pickerIndex > 11) pickerIndex = 0; 
            }
            else if (key == 'U') { 
                pickerIndex -= 4; 
                if (pickerIndex < 0) pickerIndex += 12; 
            }
            else if (key == 'D') { 
                pickerIndex += 4; 
                if (pickerIndex > 11) pickerIndex -= 12; 
            }
            else if (key == 'O') { 
                theme.setCustomTheme(PALETTE_COLORS[pickerIndex]);
                pageState = STATE_NAVIGATE; 
            }
            else if (key == '#') {
                pageState = STATE_NAVIGATE; 
            }
            return true; 
        }

        // --- 2. KHI ĐANG CHỌN BACKGROUND (WALLPAPER) ---
        if (pageState == STATE_PICK_BR) {
            if (key == 'U') {
                wallIndex--;
                wallTextScrollOffset = -3;
                if (wallIndex < 0) wallIndex = wallFileCount;
            }
            else if (key == 'D') {
                wallIndex++;
                wallTextScrollOffset = -3;
                if (wallIndex > wallFileCount) wallIndex = 0;
            }
            else if (key == 'O') {
                if (wallIndex == wallFileCount) {
                    theme.wallpaperPath = ""; // User chọn [Remove / None]
                } 
                else {
                    theme.wallpaperPath = "/SystemConfig/background/" + tempPathList[wallIndex];
                }
                theme.saveSettings();
                pageState = STATE_NAVIGATE; // Xong việc, quay lại menu chính
            }
            else if (key == '#' || key == 'L') {
                pageState = STATE_NAVIGATE; // Hủy
            }
            return true;
        }

        // --- 3. KHI ĐANG CHỈNH GIÁ TRỊ (EDIT MODE) ---
        if (pageState == STATE_EDIT_ITEM) {
            if (key == 'L' || key == 'R') {
                adjustValue(key == 'R' ? 1 : -1);
            }
            else if (key == 'O' || key == '#') {
                if (selectedIndex == 0 && theme.currentMode == THEME_CUSTOM && key == 'O') {
                    pageState = STATE_PICK_COLOR;
                    pickerIndex = 0;
                } 
                else {
                    pageState = STATE_NAVIGATE; 
                }
            }
            return true; 
        }

        // --- 4. KHI ĐANG CUỘN DANH SÁCH (NAVIGATE MODE) ---
        if (pageState == STATE_NAVIGATE) {
            if (key == 'U') {
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = 0;
                if (selectedIndex < scrollOffset) scrollOffset = selectedIndex;
            }
            else if (key == 'D') {
                selectedIndex++;
                if (selectedIndex >= totalItems) selectedIndex = totalItems - 1;
                if (selectedIndex >= scrollOffset + itemsPerPage) scrollOffset = selectedIndex - itemsPerPage + 1;
            }
            else if (key == 'R' || key == 'O') { 
                if (selectedIndex == 4) { 
                    pageState = STATE_PICK_BR;
                    wallIndex = 0;
                    scanWallpapers(); // Chỉ gọi quét thẻ nhớ 1 lần duy nhất ở đây!
                }
                else if (selectedIndex != 0) {
                    // Các dòng khác thì vào chế độ chỉnh sửa trái/phải bình thường
                    pageState = STATE_EDIT_ITEM;
                }
            }
            else if (key == 'L') {
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
                }
                else {
                    spr->print("Image");
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
        // Xóa nền khung chọn
        spr->fillRect(x, y, w, h - 20, theme.color.bg);
        spr->setTextColor(theme.color.textSub);
        spr->drawString("Select Wallpaper:", x + 5, y);
        
        int startY = y + 20;
        int lineH = 16;
        int totalWallItems = wallFileCount + 1; // +1 cho nút Remove ở cuối

        // --- LOGIC CUỘN DANH SÁCH ---
        if (totalWallItems > wallItemsPerPage) {
            if (wallIndex >= wallScrollOffset + wallItemsPerPage) {
                wallScrollOffset = wallIndex - wallItemsPerPage + 1;
            } 
            else if (wallIndex < wallScrollOffset) {
                wallScrollOffset = wallIndex;
            }
        } else {
            wallScrollOffset = 0;
        }

        // Tắt tính năng tự xuống dòng để chữ chạy không bị rớt dòng
        spr->setTextWrap(false); 

        // --- VÒNG LẶP VẼ CÁC DÒNG ---
        for (int i = 0; i < wallItemsPerPage; i++) {
            int idx = i + wallScrollOffset;
            if (idx >= totalWallItems) break;

            int lineY = startY + (i * lineH);
            bool isSelected = (idx == wallIndex);

            String textToDraw = "";
            if (idx == wallFileCount) {
                textToDraw = "[Remove / None]";
                spr->setTextColor(isSelected ? theme.color.highlight : theme.color.alert);
            } else {
                textToDraw = tempPathList[idx];
                spr->setTextColor(isSelected ? theme.color.highlight : theme.color.text);
            }

            // Xóa nền của dòng cũ (Tránh nhòe chữ khi cuộn)
            spr->fillRect(x + 5, lineY, w - 10, lineH, theme.color.bg);

            spr->setCursor(x + 10, lineY + 2);

            if (isSelected) {
                spr->print(">"); // Dấu trỏ
                spr->setCursor(x + 20, lineY + 2);

                // --- LOGIC CHỮ CHẠY ---
                if (textToDraw.length() > 14) {
                    int startChar = (wallTextScrollOffset > 0) ? wallTextScrollOffset : 0;
                    String displayName = "";
                    // Cắt lấy đúng 14 ký tự để hiển thị
                    if (startChar + 14 < textToDraw.length()) {
                         displayName = textToDraw.substring(startChar, startChar + 14);
                    } else {
                         displayName = textToDraw.substring(startChar);
                    }
                    spr->print(displayName);
                } else {
                    spr->print(textToDraw);
                }
            } 
            else {
                // Các dòng không được chọn (Tĩnh)
                spr->setCursor(x + 20, lineY + 2);
                if(textToDraw.length() > 14) {
                    spr->print(textToDraw.substring(0, 12) + ".."); // Rút gọn tĩnh
                } else {
                    spr->print(textToDraw);
                }
            }
        }
        
        // Bật lại tính năng tự xuống dòng
        spr->setTextWrap(true); 
    }
};

#endif