#ifndef KEYPAD_4X4_H
#define KEYPAD_4X4_H

#include <Arduino.h>
#include <Keypad.h>

#define ROWS 3
#define COLS 3

class Keypad_4x4 {
private:
    // Cấu hình mặc định
    inline static constexpr char defaultKeys[ROWS][COLS] = {
        {'A', 'L', '*'},
        {'U', 'O', 'D'},
        {'B', 'R', '#'}
    };

    inline static constexpr byte defaultRowPins[ROWS] = {17, 16, 15};
    inline static constexpr byte defaultColPins[COLS] = {4, 5, 6};

    // Dữ liệu hoạt động
    char activeKeys[ROWS][COLS];
    byte activeRowPins[ROWS];
    byte activeColPins[COLS];

    // Đối tượng keypad
    Keypad* keypad = nullptr;

public:
    Keypad_4x4() {
        // Sao chép dữ liệu mặc định
        memcpy(activeKeys, defaultKeys, sizeof(activeKeys));
        memcpy(activeRowPins, defaultRowPins, sizeof(activeRowPins));
        memcpy(activeColPins, defaultColPins, sizeof(activeColPins));
    }

    // Gọi trong setup()
    void begin() {
        if (keypad != nullptr) {
            delete keypad;
        }
        keypad = new Keypad(makeKeymap(activeKeys), activeRowPins, activeColPins, ROWS, COLS);
    }

    // Gọi nếu muốn cấu hình khác
    void begin(const char customKeys[ROWS][COLS], const byte customRowPins[ROWS], const byte customColPins[COLS]) {
        memcpy(activeKeys, customKeys, sizeof(activeKeys));
        memcpy(activeRowPins, customRowPins, sizeof(activeRowPins));
        memcpy(activeColPins, customColPins, sizeof(activeColPins));

        if (keypad != nullptr) {
            delete keypad;
        }
        keypad = new Keypad(makeKeymap(activeKeys), activeRowPins, activeColPins, ROWS, COLS);
    }

    // Lấy phím nhấn
    char getKey() {
        if (!keypad) return NO_KEY;
        return keypad->getKey();
    }

    // Kiểm tra xem phím cụ thể có đang được nhấn không
    bool isPressed(char keyChar) {
        if (!keypad) return false;
        return keypad->isPressed(keyChar);
    }

    Keypad& getInstance() {
        return *keypad;
    }

    // kiểm tra nhấn giữ
    bool isHold(char keyChar) {
        if (!keypad) return false;
        int idx = keypad->findInList(keyChar);
        if (idx == -1) return false;
        return keypad->key[idx].kstate == HOLD;
    }

    // bouble click
    bool isDoubleClick(char keyChar, uint16_t interval = 500) {
        static unsigned long lastPressTime[128] = {0}; // Giả sử mã ASCII từ 0-127
        if (!keypad) return false;
        if (keypad->isPressed(keyChar)) {
            unsigned long currentTime = millis();
            if (currentTime - lastPressTime[(int)keyChar] <= interval) {
                lastPressTime[(int)keyChar] = 0; // Reset để tránh nhiều lần nhận diện
                return true;
            }
            lastPressTime[(int)keyChar] = currentTime;
        }
        return false;
    }
};

#endif // KEYPAD_4X4_H
