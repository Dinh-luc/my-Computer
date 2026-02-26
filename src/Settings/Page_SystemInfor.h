#ifndef _PAGE_SYSTEMINFOR_H
#define _PAGE_SYSTEMINFOR_H

#include "SettingPage.h"

#include "Helpers/Color.h"

class Page_SystemInfor : public SettingPage {
private:


public:
    const char* getName() override {
        return "SYSTEM";
    }

    void drawDetails(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) override {
        spr->setTextColor(WHITE);
        spr->setCursor(x + 5, y + 5);
        spr->print("SYSTEM INFO");
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