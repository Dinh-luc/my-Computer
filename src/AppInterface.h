#ifndef _APP_INTERFACE_H
#define _APP_INTERFACE_H

#include <TFT_eSPI.h>

#include "Helpers/Keypad_4x4.h"

// Tham chiếu extern để các app con dùng được tft
extern TFT_eSPI tft;

class AppInterface {
public:
    // Tên app hiển thị trên Menu
    virtual const char* getName() = 0;

    // Icon app (tạm thời chưa dùng bitmap, có thể dùng màu đại diện)
    virtual uint16_t getIconColor() { 
        return TFT_WHITE; 
    }

    // Hàm khởi tạo khi app BẮT ĐẦU (cấp phát bộ nhớ, reset điểm...)
    virtual void start() = 0;

    // Hàm chạy liên tục trong vòng lặp (logic app + vẽ)
    virtual void update() = 0;

    // Hàm xử lý nút bấm
    virtual void handleInput(char key) = 0;

    // Hàm dọn dẹp khi THOÁT app (giải phóng bộ nhớ)
    virtual void stop() = 0;

    // Hàm cho biết có hiển thị thanh trạng thái hay không
    virtual bool showStatusBar() { 
        return false; 
    }
};

#endif