#include <math.h>
#include <TFT_eSPI.h>

#include "Helpers/Sensors.h"
#include "Helpers/Color.h"
#include "Helpers/Theme.h"

extern TFT_eSPI tft;
extern SensorManager sensors;
extern ThemeManager theme;

extern int width;
extern int height;

static int prevSec = -1;
static int prevDay = -1;
static int prevMinute = -1;
static bool tickState = false; 

void drawFlipCard(int x, int y, int w, int h, int number) {
    uint16_t cardColor = 0x2124; // Màu xám tối (Dark Grey) cho thẻ
    uint16_t numColor = theme.color.highlight;
    
    // 1. Vẽ nền thẻ (Bo góc nhẹ)
    tft.fillRoundRect(x, y, w, h, 6, cardColor);
    
    // 2. Vẽ viền nhẹ để tạo độ nổi (Optional)
    tft.drawRoundRect(x, y, w, h, 6, 0x4208); 

    // 3. Vẽ Số
    tft.setTextColor(numColor, cardColor);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM); 
    
    // Format số thành 2 chữ số 
    String numStr = number < 10 ? "0" + String(number) : String(number);
    tft.drawString(numStr, x + w/2, y + h/2 + 2);

    // 4. Vẽ vết cắt ngang (Flip Effect)
    tft.fillRect(x + 2, y + h/2 - 1, w - 4, 2, BLACK); 
    
    tft.drawFastHLine(x + 2, y + h/2 - 1, w - 4, 0x10A2); 
}

void drawClockScreensaver() {
    DateTime now = sensors.getTime(); 

    int cardW = 40;   
    int cardH = 50;   
    int gap = 5;    
    int startX = (width - (cardW * 3 + gap)) / 2; 
    int startY = (height - cardH) / 2 + 30;       

    // VẼ PHẦN LỊCH (Chỉ vẽ lại khi qua ngày mới hoặc lần đầu chạy)
    if (prevDay != now.day()) {
        tft.fillRect(0, 0, width, height / 2, BLACK); // Xóa nửa trên màn hình
        tft.setTextFont(1);

        // --- Cấu hình Font & Màu ---
        tft.setTextDatum(MC_DATUM); // Căn giữa
        int headerY = 15;
        
        // --- Vẽ Tháng (Dạng chữ to) ---
        tft.setTextSize(2); 
        tft.setTextColor(theme.color.highlight); 
        
        // Mảng tên tháng
        const char* months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        tft.drawString(months[now.month() - 1], width / 2, headerY);

        // --- Vẽ Các Thứ trong tuần ---
        const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
        int dayY = headerY + 20;
        int colW = (width / 7) + 1; // Chia màn hình thành 7 cột
        int currentDayOfWeek = now.dayOfTheWeek(); 

        tft.setTextSize(1);
        for (int i = 0; i < 7; i++) {
            int x = (i * colW) + (colW / 2);
            
            // Nếu là thứ hiện tại -> Vẽ khung bao quanh (Style Rainmeter: MeterDateDot)
            if (i == currentDayOfWeek) {
                tft.drawRect(x - 10, dayY - 8, 20, 15, theme.color.highlight); 
                tft.setTextColor(theme.color.highlight, BLACK);
            } else {
                tft.setTextColor(theme.color.textSub, BLACK);
            }
            tft.drawString(days[i], x, dayY);
        }

        // --- Vẽ Ngày & Năm (Dưới thứ hiện tại) ---
        int focusX = (currentDayOfWeek * colW) + (colW / 2);
        int dateY = dayY + 15;
        
        String dateStr = String(now.day());
        String yearStr = String(now.year());
        
        tft.setTextColor(WHITE, BLACK);
        tft.drawString(dateStr, focusX, dateY);
        
        tft.setTextColor(theme.color.textSub, BLACK);
        tft.drawString(yearStr, focusX, dateY + 12);

        prevDay = now.day();
    }

    if (prevMinute != now.minute() || prevSec != now.second()) {
        // Vẽ thẻ Giờ
        drawFlipCard(startX, startY, cardW, cardH, now.hour());

        // Vẽ thẻ Phút
        drawFlipCard(startX + cardW + gap, startY, cardW, cardH, now.minute());

        // Vẽ giây
        drawFlipCard(startX + cardW*2 + gap*2, startY, cardW, cardH, now.second());

        prevMinute = now.minute();
        prevSec = now.second();
    }
}