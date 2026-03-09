#ifndef _PAGE_WIFI_H
#define _PAGE_WIFI_H

#include "SettingPage.h"
#include <WiFi.h>
#include <Preferences.h>

#include "Helpers/Color.h"
#include "Helpers/VirtualKeyboard.h"

class Page_Wifi : public SettingPage {
private:
    bool isEnabled = false;
    String statusMsg = "Ready";

    int networkCount = 0;
    int listIndex = 0;
    String ssidList[20];
    int rssiList[20];

    // --- State Machine ---
    enum WifiPageState {
        STATE_OFF,
        STATE_SCANNING,
        STATE_CONNECTING,
        STATE_READY
    };
    WifiPageState currentState = STATE_OFF;
    bool isStatus = true;

    // --- Biến cho Marquee (Chữ chạy) ---
    unsigned long lastScrollUpdate = 0;
    int scrollOffset = 0;
    const int SCROLL_DELAY = 150;
    bool isScrolling = false;

    // --- Biến chống kẹt kết nối (Timeout) ---
    unsigned long connectStartTime = 0;

    // --- Virtual Keyboard ---
    VirtualKeyboard keyboard;
    String passwordInput = "";
    String selectedSSID = "";
    bool isInputMode = false;

    Preferences prefs;

public:
    const char* getName() override {
        return "WIFI";
    }

    void init() override {
        prefs.begin("wifi_creds", true);
        String savedSSID = prefs.getString("ssid", "");
        String savedPassword = prefs.getString("password", "");
        prefs.end();

        if (savedSSID.length() > 0) {
            isEnabled = true;
            isStatus = true;
            WiFi.mode(WIFI_STA);
            WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
            currentState = STATE_CONNECTING;
            statusMsg = "Auto Connecting...";
            connectStartTime = millis();
        }
    }

    void update() override {
        if (!isEnabled) return;

        // --- 1. XỬ LÝ KẾT NỐI & TIMEOUT ---
        if (currentState == STATE_CONNECTING) {
            if (WiFi.status() == WL_CONNECTED) {
                statusMsg = WiFi.SSID();
                // KẾT NỐI XONG -> RA LỆNH QUÉT MẠNG ĐỂ HIỆN LIST
                WiFi.scanNetworks(true); 
                currentState = STATE_SCANNING;
            } 
            else if (millis() - connectStartTime > 10000) { // Quá 10s không vào được -> Ép hủy
                statusMsg = "Connect Failed!";
                WiFi.disconnect();
                // HỦY XONG -> RA LỆNH QUÉT MẠNG ĐỂ NGƯỜI DÙNG CHỌN MẠNG KHÁC
                WiFi.scanNetworks(true); 
                currentState = STATE_SCANNING;
            }
        }
        // --- 2. XỬ LÝ QUÉT NGẦM MẠNG ---
        else if (currentState == STATE_SCANNING) {
            int16_t n = WiFi.scanComplete();
            if (n >= 0) {
                processScanResults(n);
                WiFi.scanDelete(); // Giải phóng RAM

                currentState = STATE_READY;
                if (WiFi.status() != WL_CONNECTED) {
                    statusMsg = (n == 0) ? "No networks found" : "Select a network";
                }
            } else if (n == -2) {
                // Nếu lỗi scan, tự động ép quét lại
                WiFi.scanNetworks(true);
            }
        }
        // --- 3. ĐANG Ở TRẠNG THÁI CHỜ ---
        else if (currentState == STATE_READY) {
            if (WiFi.status() != WL_CONNECTED && statusMsg == WiFi.SSID()) {
                statusMsg = "Disconnected";
            }
        }

        // --- 4. XỬ LÝ CHỮ CHẠY ---
        if (currentState != STATE_OFF && networkCount > 0 && millis() - lastScrollUpdate > SCROLL_DELAY) {
            String currentName = ssidList[listIndex];
            if (currentName.length() > 10) {
                scrollOffset++;
                if (scrollOffset > (int)currentName.length() - 5) scrollOffset = -3;
                isScrolling = true;
            } else {
                scrollOffset = 0;
                isScrolling = false;
            }
            lastScrollUpdate = millis();
        }
    }

    void drawDetails(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) override {
        spr->setCursor(x + 5, y + 5);
        spr->setTextColor(isFocused ? WHITE : DARKGREY);
        spr->print("WI-FI CONFIG");

        if(isStatus && isFocused) {
            spr->drawRect(x + 2, y + 22, w - 6, y + 12, YELLOW);
        }

        spr->setCursor(x + 5, y + 25);
        if (currentState == STATE_OFF) {
            spr->setTextColor(RED);
            spr->print("Status: [ OFF ]");
        }
        else if (currentState == STATE_CONNECTING) {
            spr->setTextColor(YELLOW);
            spr->print("Status: [ CONNECTING ]");
        }
        else if (currentState == STATE_SCANNING) {
            spr->setTextColor(ORANGE);
            spr->print("Status: [ SCANNING ]");
        }
        else {
            spr->setTextColor(GREEN);
            spr->print("Status: [ ON ]");
        }

        spr->setCursor(x + 5, y + 35);
        spr->setTextColor(LIGHTGREY);
        spr->print(statusMsg);

        if (keyboard.isOpen()) {
            isInputMode = true;
            keyboard.draw(spr);
            return;
        } else {
            isInputMode = false;
        }

        // --- VẼ LIST WIFI (Luôn hiện nếu có dữ liệu, không bị che đi nữa) ---
        if (currentState != STATE_OFF && networkCount > 0) {
            int startListY = 55;
            int displayLimit = 5;
            int startIdx = 0;

            spr->setTextColor(WHITE);
            spr->setCursor(x + 5, startListY - 10);
            spr->print("List Wifi:");

            if (listIndex >= displayLimit) startIdx = listIndex - displayLimit + 1;

            for (int i = 0; i < displayLimit && (startIdx + i) < networkCount; i++) {
                int idx = startIdx + i;
                int lineY = startListY + (i * 12);

                if (isFocused && idx == listIndex && !isStatus) {
                    spr->setTextColor(YELLOW);
                    spr->setCursor(x, lineY);
                    spr->print(">");

                    String name = ssidList[idx];
                    if (name.length() > 10) {
                        int startChar = (scrollOffset > 0) ? scrollOffset : 0;
                        spr->setCursor(x + 10, lineY);
                        spr->print(name.substring(startChar));
                    } else {
                        spr->setCursor(x + 10, lineY);
                        spr->print(name);
                    }
                }
                else {
                    spr->setTextColor(WHITE);
                    spr->setCursor(x + 10, lineY);
                    String name = ssidList[idx];
                    if(name.length() > 11) name = name.substring(0, 11);
                    spr->print(name);
                }
            }
        }
    }

    bool handleInput(char key) override {
        if (key == 'L' && !isInputMode) return false;

        if (isInputMode) {
            bool done = keyboard.handleInput(key);
            if (done) {
                WiFi.disconnect();
                WiFi.begin(selectedSSID.c_str(), passwordInput.c_str());

                prefs.begin("wifi_creds", false);
                prefs.putString("ssid", selectedSSID);
                prefs.putString("password", passwordInput);
                prefs.end();

                passwordInput = "";
                isInputMode = false;
                isStatus = false;

                currentState = STATE_CONNECTING;
                statusMsg = "Connecting...";
                connectStartTime = millis();
            }
            return true;
        }

        if (key == 'O') {
            if(isStatus) {
                if (currentState == STATE_OFF) toggleWifi(true);
                else toggleWifi(false);
            }
            else if (networkCount > 0) {
                selectedSSID = ssidList[listIndex];
                keyboard.begin("Pass for " + selectedSSID, &passwordInput);
                isInputMode = true;
            }
            return true;
        }

        if (key == 'U') {
            if (!isStatus) {
                listIndex--;
                scrollOffset = -3;
                if (listIndex < 0) {
                    listIndex = 0;
                    isStatus = true;
                }
            }
        }
        if (key == 'D') {
            if (isStatus) {
                if (currentState != STATE_OFF && networkCount > 0) {
                    isStatus = false;
                    listIndex = 0;
                    scrollOffset = -3;
                }
            }
            else {
                listIndex++;
                if (listIndex >= networkCount) listIndex = networkCount - 1;
                scrollOffset = -3;
            }
        }
        return true;
    }

private:
    void processScanResults(int n) {
        networkCount = 0;
        for (int i = 0; i < n && networkCount < 20; ++i) {
            String ssid = WiFi.SSID(i);
            if (ssid.length() == 0) continue;

            bool isDuplicate = false;
            for (int j = 0; j < networkCount; j++) {
                if (ssidList[j] == ssid) {
                    isDuplicate = true;
                    if (WiFi.RSSI(i) > rssiList[j]) rssiList[j] = WiFi.RSSI(i);
                    break;
                }
            }

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

            networkCount = 0;
            listIndex = 0;

            prefs.begin("wifi_creds", true);
            String savedSSID = prefs.getString("ssid", "");
            String savedPassword = prefs.getString("password", "");
            prefs.end();

            // Nếu có mạng lưu -> Ưu tiên kết nối trước, quét sau
            if (savedSSID.length() > 0) {
                WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
                currentState = STATE_CONNECTING;
                statusMsg = "Connecting...";
                connectStartTime = millis();
            } else {
                // Nếu không có mạng lưu -> Bắt đầu quét mạng ngay
                WiFi.scanNetworks(true);
                currentState = STATE_SCANNING;
                statusMsg = "Scanning...";
            }
        } else {
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            currentState = STATE_OFF;
            statusMsg = "Ready";
            networkCount = 0;
        }
    }
};

#endif