#ifndef _PAGE_WIFI_H
#define _PAGE_WIFI_H

#include "SettingPage.h"

#include <WiFi.h>
#include <Preferences.h>

#include "Helpers/Color.h"
#include "Helpers/LedControl.h"
#include "Helpers/VirtualKeyboard.h"

class Page_Wifi : public SettingPage {
private:
    bool isEnabled = false;         // Trạng thái Bật/Tắt WiFi
    String statusMsg = "Ready";     // Thông báo trạng thái

    int networkCount = 0;   // Số mạng quét được
    int listIndex = 0;      // Con trỏ chọn trong danh sách wifi
    String ssidList[20];    // Lưu tên các mạng wifi
    int rssiList[20];       // Lưu cường độ sóng (để vẽ vạch sóng)

    // --- State Machine ---
    enum WifiPageState {
        STATE_OFF,          // WiFi tắt
        STATE_SCANNING,     // Đang quét ngầm
        STATE_LIST_READY,   // Danh sách sẵn sàng
        STATE_CONNECTING,   // Thêm trạng thái đang kết nối
        STATE_CONNECTED     // Đã kết nối thành công
    };
    WifiPageState currentState = STATE_OFF;
    bool isStatus = true;      // Đang chọn trạng thái Bật/Tắt

    // --- Biến cho Marquee (Chữ chạy) ---
    unsigned long lastScrollUpdate = 0;
    int scrollOffset = 0;
    const int SCROLL_DELAY = 150; // Tốc độ chạy chữ (ms)
    bool isScrolling = false;

    // --- Virtual Keyboard ---
    VirtualKeyboard keyboard;
    String passwordInput = "";      // Lưu mật khẩu nhập từ bàn phím ảo
    String selectedSSID = "";       // Lưu SSID đang chọn
    bool isInputMode = false;       // chế độ nhập

    Preferences prefs;

public:
    const char* getName() override { 
        return "WIFI"; 
    }

    void init() override {
        connectToWiFi();
    }

    // Hàm update chạy liên tục trong vòng lặp
    void update() override {
        // Kiểm tra trạng thái kết nối (Non-blocking)
        if (currentState == STATE_CONNECTING) {
            if (WiFi.status() == WL_CONNECTED) {
                // currentState = STATE_CONNECTED;
                statusMsg =  WiFi.SSID();
            } 
        }
        else if (currentState == STATE_CONNECTED) {
            // Nếu đang connected mà bị rớt mạng
            if (WiFi.status() != WL_CONNECTED) {
                currentState = STATE_LIST_READY;    
                statusMsg = "Disconnected";
            }
        }

        // Logic xử lý Async Scan
        if (currentState == STATE_SCANNING) {
            int n = WiFi.scanComplete();
            if (n >= 0) {
                processScanResults(n); // Xử lý kết quả (Lọc trùng)
                currentState = STATE_LIST_READY;
                statusMsg = "Networks Found";
                WiFi.scanDelete(); // Xóa bộ nhớ đệm scan của ESP32
            } 
            else if (n == -2) {
                // Lỗi scan, thử lại
                WiFi.scanNetworks(true); 
            }
            // Nếu n == -1 nghĩa là đang quét, không làm gì cả
        }

        // Logic xử lý Chữ chạy (Marquee)
        if (currentState == STATE_LIST_READY && millis() - lastScrollUpdate > SCROLL_DELAY) {
            // Chỉ chạy chữ nếu tên Wifi dài hơn 10 ký tự
            if (networkCount > 0) {
                String currentName = ssidList[listIndex];
                if (currentName.length() > 10) { 
                    scrollOffset++;
                    // Nếu chạy hết chữ + 1 đoạn khoảng trắng -> Reset về 0
                    if (scrollOffset > (int)currentName.length() - 5) {
                        scrollOffset = -3; // Delay 1 chút ở đầu (-3 * 150ms)
                    }
                    isScrolling = true;
                } else {
                    scrollOffset = 0;
                    isScrolling = false;
                }
            }
            lastScrollUpdate = millis();
        }
    }

    void drawDetails(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) override {
        spr->setCursor(x + 5, y + 5);
        spr->setTextColor(isFocused? WHITE : DARKGREY);
        spr->print("WI-FI CONFIG");

        // Trạng thái Bật/Tắt
        spr->setCursor(x + 5, y + 25);
        // isStatus = true;

        if(isStatus && isFocused) {
            spr->drawRect(x + 2, y + 22, w - 6, y + 12, YELLOW);
        }

        if (currentState == STATE_OFF) {
            spr->setTextColor(RED); 
            spr->print("Status: [ OFF ]");
        } 
        else if (currentState == STATE_SCANNING) {
            spr->setTextColor(ORANGE); 
            spr->print("Scanning...");
        } 
        else {
            spr->setTextColor(GREEN); 
            spr->print("Status: [ ON ]");
        }

        // Show the currently connected Wi-Fi network
        spr->setCursor(x + 5, y + 35);
        spr->setTextColor(LIGHTGREY);

        if (isEnabled && WiFi.isConnected()) {
            spr->print("");
            spr->print(WiFi.SSID());
        } 
        else {
            spr->print("Not connected");
        } 

        if (keyboard.isOpen()) {
            isInputMode = true;
            keyboard.draw(spr); 
            return; // Dừng, không vẽ list wifi nữa
        }

        // List wifi
        if (currentState == STATE_LIST_READY) {
            int startListY = 55;
            int displayLimit = 5; // Số dòng hiển thị tối đa
            int startIdx = 0;
            
            spr->setTextColor(WHITE);
            spr->setCursor(x + 5, startListY - 10);
            spr->print("List Wifi:");

            // Tính toán cuộn danh sách (Vertical Scroll)
            if (listIndex >= displayLimit) startIdx = listIndex - displayLimit + 1;

            for (int i = 0; i < displayLimit && (startIdx + i) < networkCount; i++) {
                int idx = startIdx + i;
                int lineY = startListY + (i * 12);
                
                // --- Xử lý dòng đang chọn (Focus) ---
                if (isFocused && idx == listIndex && !isStatus) {
                    spr->setTextColor(YELLOW);
                    spr->setCursor(x, lineY); 
                    spr->print(">");
                    
                    // --- LOGIC VẼ CHỮ CHẠY ---
                    String name = ssidList[idx];
                    if (name.length() > 10) {
                        // Tính toán chuỗi con để hiển thị
                        // Kỹ thuật này cắt chuỗi để tạo hiệu ứng chạy
                        int startChar = (scrollOffset > 0) ? scrollOffset : 0;
                        String displayName = name.substring(startChar);
                        
                        // Vẽ text
                        spr->setCursor(x + 10, lineY);
                        spr->print(displayName);
                    } else {
                        // Tên ngắn thì vẽ bình thường
                        spr->setCursor(x + 10, lineY);
                        spr->print(name);
                    }
                } 
                // --- Các dòng không chọn ---
                else {
                    spr->setTextColor(WHITE);
                    spr->setCursor(x + 10, lineY);
                    // Cắt ngắn tĩnh nếu quá dài
                    String name = ssidList[idx];
                    if(name.length() > 11) name = name.substring(0, 11);
                    spr->print(name);
                }
            }
        }
    }

    bool handleInput(char key) override {
        if (!isInputMode) {
            Serial.print("Wifi: "); 
            Serial.println(key);

            // Press 'L' to switch menu focus to LEFT (Menu)
            if (key == 'L') {
                return false; 
            }

            // press 'O' to on/off wifi
            if (key == 'O') { 
                if(isStatus) {
                    if (currentState == STATE_OFF) {
                        toggleWifi(true);
                        currentState = STATE_SCANNING;
                        connectToWiFi();
                    } else {
                        toggleWifi(false);
                        currentState = STATE_OFF;
                    }
                }
                else if (currentState == STATE_LIST_READY) {
                    // Lấy tên Wifi đang chọn
                    selectedSSID = ssidList[listIndex];
                    // Mở bàn phím ảo
                    keyboard.begin("Pass for " + selectedSSID, &passwordInput);
                    isInputMode = true;
                }
                return true;
            }

            // Điều hướng List
            if (key == 'U') { // UP
                if (!isStatus) {
                    listIndex--;
                    scrollOffset = -3;
                    // Nếu đang ở đầu list mà bấm lên -> Nhảy về nút Status
                    if (listIndex < 0) {
                        listIndex = 0;
                        isStatus = true; // Focus về Status
                    }
                }
            }
            if (key == 'D') { // DOWN
                if (isStatus) {
                    if (currentState == STATE_LIST_READY && networkCount > 0) {
                        isStatus = false; // Bỏ focus Status
                        listIndex = 0;    // Chọn dòng đầu tiên
                        scrollOffset = -3;
                    }
                } 
                else {
                    listIndex++;
                    if (listIndex >= networkCount) listIndex = networkCount - 1;
                    scrollOffset = -3;
                }
            }
        }

        // ĐANG NHẬP PASSWORD
        if (keyboard.isOpen()) {
            Serial.print("Virtual: "); 
            Serial.println(key);

            bool done = keyboard.handleInput(key);
            if (done) {
                // Nhập xong pass -> Connect
                WiFi.begin(selectedSSID.c_str(), passwordInput.c_str());
                // lưu vào Preferences
                prefs.begin("wifi_creds", false);
                prefs.putString("ssid", selectedSSID);
                prefs.putString("password", passwordInput);
                prefs.end();
                // Reset lại input
                passwordInput = "";
                // Focus trả về list
                isStatus = false; // Tùy chọn: giữ ở list hay về status
                isInputMode = false;
            }
            return true;
        }

        return true;
    }

private:
    // Xử lý kết quả scan (lọc trùng SSID)
    void processScanResults(int n) {
        networkCount = 0;
        for (int i = 0; i < n && networkCount < 20; ++i) {
            String ssid = WiFi.SSID(i);
            
            // 1. Bỏ qua mạng ẩn (tên rỗng)
            if (ssid.length() == 0) continue;

            // 2. Kiểm tra trùng lặp
            bool isDuplicate = false;
            for (int j = 0; j < networkCount; j++) {
                if (ssidList[j] == ssid) {
                    isDuplicate = true;
                    // Mẹo: Nếu trùng, ta nên giữ lại cái nào sóng mạnh hơn (RSSI lớn hơn)
                    if (WiFi.RSSI(i) > rssiList[j]) {
                        rssiList[j] = WiFi.RSSI(i); // Cập nhật sóng
                    }
                    break;
                }
            }

            // 3. Nếu không trùng thì thêm vào list
            if (!isDuplicate) {
                ssidList[networkCount] = ssid;
                rssiList[networkCount] = WiFi.RSSI(i);
                networkCount++;
            }
        }
    }

    void toggleWifi(bool enable) {
        isEnabled = enable;
        if (isEnabled) {
            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
            
            networkCount = WiFi.scanNetworks();
            
            // Copy vào mảng của mình
            if (networkCount > 10) networkCount = 10;
            for (int i = 0; i < networkCount; i++) {
                ssidList[i] = WiFi.SSID(i);
                rssiList[i] = WiFi.RSSI(i);
                Serial.println("Found SSID: " + ssidList[i]);
            }
        } else {
            WiFi.mode(WIFI_OFF);
            networkCount = 0;
        }
    }
    
    void connectToWiFi() {
        prefs.begin("wifi_creds", true); // Read-only mode
        String savedSSID = prefs.getString("ssid", "");
        String savedPassword = prefs.getString("password", "");
        prefs.end();

        if (savedSSID.length() > 0) {
            isEnabled = true;
            isStatus = true; // Focus vào nút Status
            WiFi.mode(WIFI_STA);
            
            // Nếu chưa kết nối thì mới connect
            if (WiFi.status() != WL_CONNECTED) {
                WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
                statusMsg = "Auto Connecting...";
                currentState = STATE_CONNECTING;
            } else {
                currentState = STATE_CONNECTED;
                statusMsg = "Connected";
            }
        }
    }
};

#endif