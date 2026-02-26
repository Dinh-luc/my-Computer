#ifndef _SETTING_PAGE_H
#define _SETTING_PAGE_H

#include <TFT_eSPI.h>

class SettingPage {
public:
    // Tên hiển thị ở menu bên trái
    virtual const char* getName() = 0;

    // Hàm khởi tạo (chạy 1 lần khi vào trang này)
    virtual void init() {}

    // Hàm vẽ nội dung bên phải (Control Panel)
    // x, y, w, h: Tọa độ và kích thước vùng bên phải được phép vẽ
    virtual void drawDetails(TFT_eSprite* sprite, int x, int y, int w, int h, bool isFocused) = 0;

    // Hàm xử lý nút bấm (Trái/Phải/OK/Serial)
    virtual bool handleInput(char key) = 0;

    // Hàm xử lý cập nhật
    virtual void update() {}
};

#endif