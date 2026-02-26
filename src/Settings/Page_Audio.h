#ifndef _PAGE_AUDIO_H
#define _PAGE_AUDIO_H

#include "SettingPage.h"

class Page_Audio : public SettingPage {
private:
    int volume = 50;
    bool isMute = false;

public:
    const char* getName() override { 
        return "AUDIO"; 
    }

    void drawDetails(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) override {
        // Vẽ tiêu đề
        spr->setTextColor(TFT_WHITE);
        spr->setCursor(x + 5, y + 5);
        spr->print("VOLUME CONTROL");

        // Vẽ trạng thái
        spr->setCursor(x + 5, y + 25);
        if (isMute) {
            spr->setTextColor(TFT_RED);
            spr->print("Status: MUTED");
        } else {
            spr->setTextColor(TFT_GREEN);
            spr->print("Status: ACTIVE");
        }

        // Vẽ thanh Volume
        spr->drawRect(x + 5, y + 50, w - 10, 10, TFT_WHITE);
        int barW = map(volume, 0, 100, 0, w - 12);
        spr->fillRect(x + 6, y + 51, barW, 8, TFT_CYAN);

        spr->setCursor(x + 5, y + 65);
        spr->setTextColor(TFT_LIGHTGREY);
        spr->print(volume); spr->print("%");
        
        // Hướng dẫn
        spr->setTextColor(TFT_DARKGREY);
        spr->setCursor(x + 5, y + h - 15);
        spr->print("< > : Adjust");
    }

    bool handleInput(char key) override {
        if (key == 'L') { // LEFT
            volume -= 10;
            if (volume < 0) volume = 0;
        }
        else if (key == 'R') { // RIGHT
            volume += 10;
            if (volume > 100) volume = 100;
        }
        else if (key == 'O') { // OK
            isMute = !isMute;
        }
        return true; // Giữ Focus bên phải
    }
};

#endif