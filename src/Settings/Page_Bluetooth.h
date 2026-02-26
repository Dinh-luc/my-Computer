#ifndef _PAGE_BLUETOOTH_H
#define _PAGE_BLUETOOTH_H

#include "SettingPage.h"

#include "Helpers/Color.h"

class Page_Bluetooth : public SettingPage {
private:


public:
    const char* getName() override {
        return "BLE";
    }

    void drawDetails(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) override {
        spr->setTextColor(WHITE);
        spr->setCursor(x + 5, y + 5);
        spr->print("BLUETOOTH");
    }

    bool handleInput(char key) override {
        // Press '4' to switch menu focus to LEFT (Menu)
        if (key == '4') {
            return false; 
        }
        return true; // Giữ Focus bên phải
    }

};

#endif