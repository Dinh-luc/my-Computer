#ifndef _APP_WEBSYNC_H
#define _APP_WEBSYNC_H

#include "AppInterface.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

#undef CLOSED

#include <ESPAsyncWebServer.h> 
#include <Update.h>            
#include <WiFi.h>

#include "Webserver/WebUI.h"

#include "Helpers/Storage.h"

extern TFT_eSPI tft;
extern Storage storage; 
extern int width;
extern int height;

#define BAR_HEIGHT 18

class App_WebSync : public AppInterface {
private:
    AsyncWebServer* server = nullptr;
    bool isServerRunning = false;
    String localIP;
    bool isWifi = false;

    TFT_eSprite img = TFT_eSprite(&tft);

public:
    App_WebSync(){}
    ~App_WebSync(){}

    const char* getName() override { 
        return "DOWNLOAD"; 
    }

    const char* getIconColor() override {
        return "download";
    }

    bool showStatusBar() override { 
        return true; 
    }

    void start() override {
        img.setColorDepth(16);
        img.createSprite(width, height);

        // 1. Kiểm tra WiFi
        if (WiFi.status() != WL_CONNECTED) {
            isWifi = false;
            drawUI();
            return;
        }
        isWifi = true;
        localIP = WiFi.localIP().toString();

        drawUI();

        // 2. Khởi tạo Server
        server = new AsyncWebServer(80);
        setupRoutes();
        server->begin();
        isServerRunning = true;
    }

    void stop() override {
        if (server != nullptr) {
            server->end();
            delete server; 
            server = nullptr;
        }
        img.deleteSprite(); // Nhớ giải phóng sprite
        isServerRunning = false;
    }

    void update() override {
        unsigned long lastInteraction;
        lastInteraction = millis();
    }

    void handleInput(char key) override {
    }

private:
    void drawUI() {
        img.fillSprite(TFT_BLACK);
        img.setTextColor(TFT_WHITE);

        if (isWifi) {
            img.setTextSize(2);
            img.setTextColor(0x07FF); // Cyan
            img.drawString("WEB SYNC", 10, 10);
            
            img.setTextSize(1);
            img.setTextColor(TFT_WHITE);
            img.drawString("PC / Phone Browser:", 10, 35);
            
            img.setTextSize(2);
            img.setTextColor(TFT_YELLOW);
            img.drawString(localIP, 10, 50);

            img.setTextSize(1);
            img.setTextColor(TFT_LIGHTGREY);
            img.drawString("Keep this screen open", 10, 80);
            img.drawString("during file transfer", 10, 95);
        } else {
            img.setTextSize(2);
            img.setTextColor(TFT_RED);
            img.drawString("NO WIFI", 10, 30);
            
            img.setTextSize(1);
            img.setTextColor(TFT_WHITE);
            img.drawString("Go to Settings -> WiFi", 10, 60);
        }

        img.pushSprite(0, BAR_HEIGHT); // Vẽ ngay dưới StatusBar
    }

    // --- CẤU HÌNH CÁC ĐƯỜNG DẪN WEB ---
    void setupRoutes() {
        if (server == nullptr) return;
        
        // 1. Route chính: Gửi giao diện Web
        server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", index_html);
        });

        // Route phụ: Trả về kích thước màn hình động dạng JSON
        server->on("/sysinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
            String json = "{\"w\":" + String(width) + ",\"h\":" + String(height) + "}";
            request->send(200, "application/json", json);
        });
        
        // 2. Route Nhận File vào Thẻ nhớ (SD_MMC)
        server->on("/upload", HTTP_POST, 
            [](AsyncWebServerRequest *request) {
                request->send(200, "text/plain", "Đã lưu vào thẻ nhớ!");
            },
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                
                // Ép kiểu biến tempObject của request thành con trỏ File
                fs::File* file = (fs::File*)request->_tempObject;
                
                // Gói tin đầu tiên: Tạo file và Phân loại thư mục
                if (index == 0) {
                    String path = "";
                    filename.replace(" ", "_"); 
                    
                    if (filename.endsWith(".jpg")) {
                        // 1. Ảnh JPG -> Hình nền -> Lưu Thẻ nhớ
                        storage.createDir("/Background");
                        path = "/Background/" + filename;
                    } 
                    else if (filename.endsWith(".png")) {
                        // 2. Ảnh PNG -> Icon -> Lưu Flash (LittleFS)
                        storage.createDir("/sys");
                        storage.createDir("/sys/ic");
                        path = "/sys/ic/" + filename;
                    } 
                    else if (filename.startsWith("SYS_")) {
                        // 3. Âm thanh Hệ thống (Đã được JS gắn tiền tố SYS_) -> Lưu Flash
                        filename.remove(0, 4); // Xóa chữ "SYS_" đi cho đẹp tên file
                        storage.createDir("/sys");
                        storage.createDir("/sys/au");
                        path = "/sys/au/" + filename;
                    }
                    else if (filename.startsWith("SD_")) {
                        // 4. Nhạc giải trí (Đã được JS gắn tiền tố SD_) -> Lưu Thẻ nhớ
                        filename.remove(0, 3); // Xóa chữ "SD_" 
                        storage.createDir("/Music");
                        path = "/Music/" + filename;
                    }
                    else {
                        // Các định dạng khác mặc định đẩy ra thẻ nhớ
                        storage.createDir("/Data");
                        path = "/Data/" + filename;
                    }
                    
                    // Tuyệt chiêu: Mở file bằng Storage thông minh. 
                    // Nó sẽ tự biết gọi LittleFS hay SD_MMC dựa trên Path!
                    file = new fs::File(storage.openFile(path, FILE_WRITE));
                    request->_tempObject = (void*)file;
                }
                
                // Ghi dữ liệu vào file
                if (file && *file) {
                    file->write(data, len);
                }
                
                // Gói tin cuối cùng: Đóng file và dọn dẹp RAM
                if (final) {
                    if (file) {
                        file->close();
                        delete file; 
                        request->_tempObject = nullptr;
                    }
                }
            }
        );

        // 3. Route Nạp Firmware OTA
        server->on("/update", HTTP_POST, 
            // Hàm phản hồi khi upload xong
            [](AsyncWebServerRequest *request) {
                bool shouldReboot = !Update.hasError();
                AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
                response->addHeader("Connection", "close");
                request->send(response);
                
                if (shouldReboot) {
                    delay(500);
                    ESP.restart(); // Khởi động lại hệ thống với Code mới
                }
            },
            // Hàm xử lý ghi dữ liệu vào Flash (OTA)
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                // Gói tin đầu: Bắt đầu quá trình Update
                if (index == 0) {
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
                        Update.printError(Serial);
                    }
                }
                
                // Ghi khối dữ liệu vào Flash
                if (!Update.hasError()) {
                    if (Update.write(data, len) != len) {
                        Update.printError(Serial);
                    }
                }
                
                // Gói tin cuối: Kết thúc và xác nhận an toàn
                if (final) {
                    if (Update.end(true)) {
                        Serial.println("OTA Update Complete!");
                    } else {
                        Update.printError(Serial);
                    }
                }
            }
        );
    }
};
#endif