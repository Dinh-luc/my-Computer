// ==============================================================================
//   USER SETUP CHO MÀN HÌNH TFT 1.8" ST7735 - ESP32-S3
// ==============================================================================

// 1. CHỌN DRIVER MÀN HÌNH
#define ST7735_DRIVER     // Kích hoạt driver cho chip ST7735

// 2. KÍCH THƯỚC MÀN HÌNH
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// 3. CHỌN LOẠI MÀN HÌNH ST7735 (Tab Color)
// Màn hình 1.8" thường có miếng nilon dán ở mép kéo màn hình màu gì thì chọn màu đó.
// Phổ biến nhất là REDTAB hoặc BLACKTAB. 
// Nếu màn hình bị nhiễu hạt ở viền hoặc sai màu, hãy thử bật/tắt các dòng dưới đây:
// #define ST7735_INITB
// #define ST7735_GREENTAB
// #define ST7735_GREENTAB2
// #define ST7735_GREENTAB3
// #define ST7735_GREENTAB128
// #define ST7735_GREENTAB160x80
// #define ST7735_REDTAB
#define ST7735_BLACKTAB
// #define ST7735_REDTAB160x80

// Đảo màu (Nếu màu đỏ biến thành xanh thì bỏ comment dòng dưới)
// #define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF

// 4. CẤU HÌNH CHÂN KẾT NỐI CHO ESP32-S3
// CHÚ Ý: ĐÂY LÀ CÁC CHÂN VÍ DỤ. BẠN PHẢI SỬA LẠI CHO ĐÚNG VỚI SƠ ĐỒ MẠCH CỦA BẠN!
#define TFT_MOSI 46 // Chân SDA / DIN trên màn hình
#define TFT_SCLK 3 // Chân SCK / CLK trên màn hình
#define TFT_CS   1 // Chân CS (Chip Select)
#define TFT_DC    42 // Chân DC / RS (Data/Command)
#define TFT_RST   2 // Chân RES / RST (Reset)

// Cấu hình chân Đèn nền (Backlight) - Lúc trước ta quy ước là chân 21
// #define TFT_BL   21            // Chân LED / BL
#define TFT_BACKLIGHT_ON HIGH  // Mức tín hiệu để bật đèn (HIGH hoặc LOW)

// 5. CẤU HÌNH FONT CHỮ (Giữ lại các Font nhẹ và cần thiết để tiết kiệm Flash)
#define LOAD_GLCD   // Font chuẩn (nhỏ)
#define LOAD_FONT2  // Font 16 pixel (nhỏ)
#define LOAD_FONT4  // Font 26 pixel (vừa - dùng cho Flip Clock)
#define LOAD_FONT6  // Font 48 pixel (lớn)
#define LOAD_FONT7  // Font 7 đoạn (Led)
#define LOAD_FONT8  // Font 75 pixel (rất lớn)
#define LOAD_GFXFF  // BẮT BUỘC: Hỗ trợ FreeFonts (Vector) mà chúng ta đang dùng

#define SMOOTH_FONT // Hỗ trợ font làm mịn cạnh

// 6. CẤU HÌNH TỐC ĐỘ GIAO TIẾP SPI
// ST7735 chạy ổn định nhất ở mức 27MHz. Nếu màn hình bị nhiễu/rác, hãy hạ xuống 20000000
#define SPI_FREQUENCY  27000000

// Tốc độ khi đọc dữ liệu từ màn hình (Không quan trọng lắm vì ta ít khi đọc ngược)
#define SPI_READ_FREQUENCY  20000000

// Tốc độ cho cảm ứng (Nếu có module touch) - Màn của bạn không có nên cứ để nguyên
#define SPI_TOUCH_FREQUENCY  2500000