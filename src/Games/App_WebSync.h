#ifndef _APP_WEBSYNC_H
#define _APP_WEBSYNC_H

#include "AppInterface.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

// Fix macro collision
#undef CLOSED

#include <ESPAsyncWebServer.h> 
#include <Update.h>            
#include <WiFi.h>
#include <SD_MMC.h> // Để ghi file vào thẻ nhớ

#include "Webserver/WebUI.h" // Gọi file giao diện HTML/CSS/JS

extern TFT_eSPI tft;
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
        return "PC CONNECT"; 
    }

    uint16_t getIconColor() override {
        return TFT_BLUE;
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
                    // Xóa dấu cách trong tên file để SD card không bị lỗi
                    filename.replace(" ", "_"); 
                    
                    if (filename.endsWith(".jpg")) {
                        // 1. Nếu là ảnh JPG -> Lưu vào thư mục Hình nền
                        if (!SD_MMC.exists("/SystemConfig/background")) {
                            SD_MMC.mkdir("/SystemConfig/background");
                        }
                        path = "/SystemConfig/background/" + filename;
                    } 
                    else if (filename.endsWith(".png")) {
                        // 2. Nếu là ảnh PNG -> Lưu vào thư mục Icon
                        if (!SD_MMC.exists("/SystemConfig/icon")) {
                            SD_MMC.mkdir("/SystemConfig/icon");
                        }
                        path = "/SystemConfig/icon/" + filename;
                    } 
                    else {
                        // 3. Nếu là các định dạng khác (Nhạc MP3, WAV...) -> Lưu vào Music
                        if (!SD_MMC.exists("/Music")) {
                            SD_MMC.mkdir("/Music");
                        }
                        path = "/Music/" + filename;
                    }
                    
                    // Mở file và gán con trỏ vào request
                    file = new fs::File(SD_MMC.open(path, FILE_WRITE));
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