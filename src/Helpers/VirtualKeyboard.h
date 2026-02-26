#ifndef _VIRTUAL_KEYBOARD_H
#define _VIRTUAL_KEYBOARD_H

#include <TFT_eSPI.h>
#include <Arduino.h>

#include "Color.h"

// --- Cấu hình giao diện ---
#define ROWS        5
#define MAX_COLS    10

#define KB_X        5       // Vị trí X bắt đầu bàn phím
#define KB_Y        35      // Vị trí Y bắt đầu bàn phím
#define KB_W        150     // Chiều rộng bàn phím  
#define KB_H        80      // Chiều cao bàn phím
#define KEY_GAP     1       // Khoảng cách giữa các phím
#define KEY_RADIUS  1       // Bán kính bo góc phím

// --- Màu sắc ---
#define COLOR_BG        BLACK       
#define COLOR_KEY       0x5AEB          // Xám xanh
#define COLOR_KEY_ACT   0x001F          // Xanh dương (Khi nhấn)
#define COLOR_CURSOR    RED         // MÀU CON TRỎ (QUAN TRỌNG)
#define COLOR_TEXT      WHITE       

enum KeyType {
    KEY_NORMAL,             
    KEY_SHIFT,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_SPACE,
    KEY_SPECIAL_SWITCH
};

struct KeyDef {
    const char* label;
    char lowerVal;
    char upperVal;
    uint8_t width;
    KeyType type;
};

class VirtualKeyboard {
private:
    String* targetString;
    int cursorX = 0;
    int cursorY = 0;
    bool isActive = false;
    bool isShiftActive = false;
    String title = "Input:";
    
    // Layout bàn phím (Đưa vào static để gọn code)
    static const KeyDef layout[ROWS][MAX_COLS];

public:
    void begin(String inputTitle, String* outputBuffer) {
        targetString = outputBuffer;
        title = inputTitle;
        isActive = true;
        cursorX = 0; // Reset về phím đầu tiên
        cursorY = 0;
        isShiftActive = false;
        *targetString = ""; 
    }

    bool isOpen() { return isActive; }
    void close() { isActive = false; }

    // --- LOGIC VẼ GIAO DIỆN ---
    void draw(TFT_eSprite* spr) {
        if (!isActive) return;

        spr->fillSprite(COLOR_BG); 

        // 1. Vẽ Input Box
        spr->drawRect(5, 5, 150, 25, WHITE);
        spr->setTextColor(GREEN, COLOR_BG);
        spr->setCursor(10, 8);
        spr->print(title);
        
        spr->setTextColor(COLOR_TEXT, COLOR_BG);
        spr->setCursor(10, 20); // Chỉnh lại vị trí text cho dễ nhìn
        spr->print(*targetString + "_"); 

        // 2. Tính toán kích thước
        int rowHeight = KB_H / ROWS;
        int unitWidth = (KB_W - (10 * KEY_GAP)) / 10;
        int currentY = KB_Y;

        for (int r = 0; r < ROWS; r++) {
            // Tính tổng width để căn giữa
            int totalRowWidth = 0;
            for (int c = 0; c < MAX_COLS; c++) {
                if (layout[r][c].width == 0) break;
                KeyDef k = layout[r][c];
                int keyW = (k.width * unitWidth) + ((k.width - 1) * KEY_GAP);
                totalRowWidth += keyW + KEY_GAP;
            }
            if (totalRowWidth > 0) totalRowWidth -= KEY_GAP;

            int currentX = KB_X + (KB_W - totalRowWidth) / 2;

            // Vẽ từng phím
            for (int c = 0; c < MAX_COLS; c++) {
                if (layout[r][c].width == 0) break;

                KeyDef k = layout[r][c];
                int realKeyW = (k.width * unitWidth) + ((k.width - 1) * KEY_GAP);
                int realKeyH = rowHeight - KEY_GAP;

                // Kiểm tra xem phím này có đang được trỏ tới không?
                bool isSelected = (r == cursorY && c == cursorX);

                drawKey(currentX, currentY, realKeyW, realKeyH, k, isSelected, spr);

                currentX += realKeyW + KEY_GAP;
            }
            currentY += rowHeight;
        }
    }

    // --- XỬ LÝ NHẬP LIỆU ---
    bool handleInput(char key) {
        if (!isActive) return false;

        int colsInCurrentRow = getColsCount(cursorY);

        switch (key) {
            case 'U': // LÊN
                cursorY--;
                if (cursorY < 0) cursorY = ROWS - 1;
                // Clamp: Nếu hàng mới ít phím hơn hàng cũ, kéo cursorX về cuối hàng mới
                if (cursorX >= getColsCount(cursorY)) cursorX = getColsCount(cursorY) - 1;
                break;

            case 'D': // XUỐNG
                cursorY++;
                if (cursorY >= ROWS) cursorY = 0;
                // Clamp
                if (cursorX >= getColsCount(cursorY)) cursorX = getColsCount(cursorY) - 1;
                break;

            case 'L': // TRÁI
                cursorX--;
                if (cursorX < 0) cursorX = colsInCurrentRow - 1; // Wrap về cuối hàng
                break;

            case 'R': // PHẢI
                cursorX++;
                if (cursorX >= colsInCurrentRow) cursorX = 0; // Wrap về đầu hàng
                break;

            case '#': // HỦY
                isActive = false;
                return true; 

            case 'O': // CHỌN (SELECT)
                KeyDef target = layout[cursorY][cursorX];
                processKey(target);
                // Nếu nhấn Enter thì đóng phím
                if (target.type == KEY_ENTER) {
                    isActive = false;
                    return true;
                }
                break;
        }
        return false;
    }

private:
    // Đếm số cột có dữ liệu trong 1 hàng
    int getColsCount(int r) {
        int count = 0;
        for (int c = 0; c < MAX_COLS; c++) {
            if (layout[r][c].width == 0) break;
            count++;
        }
        return count;
    }

    void processKey(KeyDef k) {
        if (k.type == KEY_NORMAL) {
            *targetString += (isShiftActive ? k.upperVal : k.lowerVal);
        } 
        else if (k.type == KEY_SPACE) {
            *targetString += ' ';
        }
        else if (k.type == KEY_BACKSPACE) {
            if (targetString->length() > 0) 
                targetString->remove(targetString->length() - 1);
        }
        else if (k.type == KEY_SHIFT) {
            isShiftActive = !isShiftActive;
        }
        else if (k.type == KEY_SPECIAL_SWITCH) {
            isShiftActive = !isShiftActive; 
        }
    }

    void drawKey(int x, int y, int w, int h, KeyDef key, bool selected, TFT_eSprite* spr) {
        uint16_t keyColor = COLOR_KEY;
        uint16_t textColor = COLOR_TEXT;
        uint16_t borderColor = TFT_DARKGREY;

        // Xử lý màu sắc
        if (key.type == KEY_SHIFT && isShiftActive) {
            keyColor = TFT_ORANGE;
            textColor = TFT_BLACK;
        }
        
        // --- HIỆU ỨNG CON TRỎ (QUAN TRỌNG NHẤT) ---
        if (selected) {
            borderColor = COLOR_CURSOR; // Viền đỏ
            keyColor = TFT_WHITE;       // Nền trắng sáng lên
            textColor = TFT_BLACK;      // Chữ đen
        }

        // Vẽ
        spr->fillRoundRect(x, y, w, h, KEY_RADIUS, keyColor);
        // Vẽ viền dày hơn nếu đang chọn
        if (selected) {
            spr->drawRoundRect(x, y, w, h, KEY_RADIUS, borderColor);
            spr->drawRoundRect(x+1, y+1, w-2, h-2, KEY_RADIUS, borderColor);
        } else {
            spr->drawRoundRect(x, y, w, h, KEY_RADIUS, borderColor);
        }

        // Label
        String label = "";
        if (key.type == KEY_NORMAL) {
            label = String(isShiftActive ? key.upperVal : key.lowerVal);
        } else if (key.label != NULL) {
            label = String(key.label);
        }

        spr->setTextColor(textColor, keyColor);
        spr->setTextDatum(MC_DATUM);
        spr->setTextSize(1);
        spr->drawString(label, x + w / 2, y + h / 2);
    }
};

// Định nghĩa dữ liệu Static bên ngoài class
const KeyDef VirtualKeyboard::layout[ROWS][MAX_COLS] = {
    { 
        {"1", '1', '!', 1, KEY_NORMAL}, {"2", '2', '@', 1, KEY_NORMAL}, 
        {"3", '3', '#', 1, KEY_NORMAL}, {"4", '4', '$', 1, KEY_NORMAL}, 
        {"5", '5', '%', 1, KEY_NORMAL}, {"6", '6', '^', 1, KEY_NORMAL}, 
        {"7", '7', '&', 1, KEY_NORMAL}, {"8", '8', '*', 1, KEY_NORMAL}, 
        {"9", '9', '(', 1, KEY_NORMAL}, {"0", '0', ')', 1, KEY_NORMAL} 
    },
    { 
        {"q", 'q', 'Q', 1, KEY_NORMAL}, {"w", 'w', 'W', 1, KEY_NORMAL}, 
        {"e", 'e', 'E', 1, KEY_NORMAL}, {"r", 'r', 'R', 1, KEY_NORMAL}, 
        {"t", 't', 'T', 1, KEY_NORMAL}, {"y", 'y', 'Y', 1, KEY_NORMAL}, 
        {"u", 'u', 'U', 1, KEY_NORMAL}, {"i", 'i', 'I', 1, KEY_NORMAL}, 
        {"o", 'o', 'O', 1, KEY_NORMAL}, {"p", 'p', 'P', 1, KEY_NORMAL} 
    },
    { 
        {"a", 'a', 'A', 1, KEY_NORMAL}, {"s", 's', 'S', 1, KEY_NORMAL}, 
        {"d", 'd', 'D', 1, KEY_NORMAL}, {"f", 'f', 'F', 1, KEY_NORMAL}, 
        {"g", 'g', 'G', 1, KEY_NORMAL}, {"h", 'h', 'H', 1, KEY_NORMAL}, 
        {"j", 'j', 'J', 1, KEY_NORMAL}, {"k", 'k', 'K', 1, KEY_NORMAL}, 
        {"l", 'l', 'L', 1, KEY_NORMAL}, {NULL, 0, 0, 0, KEY_NORMAL} 
    },
    { 
        {"⇑", 0, 0, 2, KEY_SHIFT}, {"z", 'z', 'Z', 1, KEY_NORMAL}, 
        {"x", 'x', 'X', 1, KEY_NORMAL}, {"c", 'c', 'C', 1, KEY_NORMAL}, 
        {"v", 'v', 'V', 1, KEY_NORMAL}, {"b", 'b', 'B', 1, KEY_NORMAL}, 
        {"n", 'n', 'N', 1, KEY_NORMAL}, {"m", 'm', 'M', 1, KEY_NORMAL}, 
        {"DEL", 8, 8, 2, KEY_BACKSPACE}, {NULL, 0, 0, 0, KEY_NORMAL} 
    },
    { 
        {"123", 0, 0, 2, KEY_SPECIAL_SWITCH},
        {",", ',', ',', 1, KEY_NORMAL}, 
        {"SPACE", ' ', ' ', 4, KEY_SPACE}, 
        {".",'.','.',1,KEY_NORMAL}, 
        {"OK", 13, 13, 2, KEY_ENTER}, 
        {NULL, 0, 0, 0, KEY_NORMAL} 
    }
};

#endif