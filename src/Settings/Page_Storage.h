#ifndef _PAGE_STORAGE_H
#define _PAGE_STORAGE_H

#include "SettingPage.h"
#include "Helpers/Storage.h" 
#include "Helpers/VirtualKeyboard.h"
#include "Helpers/Color.h"

extern Storage storage; 

#define MAX_FILES_PER_PAGE 20 

struct FileEntry {
    String name;
    bool isFolder;
    uint32_t size;
};

enum StorageState {
    ST_VIEW_LIST,       
    ST_CONTEXT_MENU,    
    ST_INPUT_RENAME,    
    ST_INPUT_NEW        
};

enum ContextOption {
    OPT_OPEN,
    OPT_NEW_FOLDER,
    OPT_RENAME,
    OPT_DELETE,
    OPT_CANCEL
};

class Page_Storage : public SettingPage {
private:
    // --- Data Variables ---
    String currentPath = "/";
    FileEntry fileList[MAX_FILES_PER_PAGE];
    int fileCount = 0;
    
    // --- Stats ---
    uint64_t totalBytes = 0;
    uint64_t usedBytes = 0;

    // --- Navigation UI ---
    int selectedIndex = 0;     
    int scrollOffset = 0;      
    
    // --- Context Menu UI ---
    StorageState currentState = ST_VIEW_LIST;
    int menuIndex = 0;         
    const char* menuOptions[5] = {"Open", "New Folder", "Rename", "Delete", "Cancel"};
    
    // --- Input ---
    VirtualKeyboard keyboard;
    String inputBuffer = "";
    
    String formatBytes(uint64_t bytes) {
        if (bytes < 1024) return String((uint32_t)bytes) + " B";
        else if (bytes < (1024 * 1024)) return String((uint32_t)(bytes / 1024)) + " KB";
        else if (bytes < (1024 * 1024 * 1024)) return String((uint32_t)(bytes / 1024 / 1024)) + " MB";
        else return String((float)(bytes / 1024 / 1024 / 1024.0), 2) + " GB";
    }

    // --- Biến cho Marquee (Chữ chạy) ---
    unsigned long lastScrollUpdate = 0;
    int scrollNameOffset = 0;
    const int SCROLL_DELAY = 150; // Tốc độ chạy chữ (ms)
    bool isScrolling = false;

public:
    const char* getName() override { return "STORAGE"; }

    void init() override {
        refreshStats();
        loadFolder("/");
    }

    void update() override {
        // Logic xử lý Chữ chạy (Marquee)
        if (currentState == ST_VIEW_LIST && millis() - lastScrollUpdate > SCROLL_DELAY) {
            // Chỉ chạy chữ nếu tên file dài hơn 10 ký tự
            if (fileCount > 0) {
                String currentName = fileList[selectedIndex].name;
                if (currentName.length() > 10) { 
                    scrollNameOffset++;
                    // Nếu chạy hết chữ + 1 đoạn khoảng trắng -> Reset về 0
                    if (scrollNameOffset > (int)currentName.length() - 5) {
                        scrollNameOffset = -3; // Delay 1 chút ở đầu (-3 * 150ms)
                    }
                    isScrolling = true;
                } else {
                    scrollNameOffset = 0;
                    isScrolling = false;
                }
            }
            lastScrollUpdate = millis();
        }
    }

    void drawDetails(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) override {
        drawHeader(spr, x, y, w);

        if (keyboard.isOpen()) {
            keyboard.draw(spr);
            return;
        }

        drawFileList(spr, x, y + 40, w, h - 40, isFocused);

        if (currentState == ST_CONTEXT_MENU) {
            drawContextMenu(spr, x + 10, y + 10, w - 20, 100);
        }
    }

    bool handleInput(char key) override {
        // 1. INPUT MODE
        if (keyboard.isOpen()) {
            bool done = keyboard.handleInput(key);
            if (done) {
                if (currentState == ST_INPUT_NEW) {
                    if (inputBuffer.length() > 0) {
                        String newPath = currentPath + (currentPath == "/" ? "" : "/") + inputBuffer;
                        storage.createDir(newPath); 
                    }
                } 
                else if (currentState == ST_INPUT_RENAME) {
                    if (inputBuffer.length() > 0 && selectedIndex < fileCount) {
                        String oldPath = currentPath + (currentPath == "/" ? "" : "/") + fileList[selectedIndex].name;
                        String newPath = currentPath + (currentPath == "/" ? "" : "/") + inputBuffer;
                        storage.rename(oldPath, newPath);
                    }
                }
                
                inputBuffer = "";
                refreshStats(); 
                loadFolder(currentPath);
                currentState = ST_VIEW_LIST;
            }
            return true;
        }

        // 2. CONTEXT MENU
        if (currentState == ST_CONTEXT_MENU) {
            if (key == 'U') { 
                menuIndex--;
                if (menuIndex < 0) menuIndex = 4;
            }
            else if (key == 'D') { 
                menuIndex++;
                if (menuIndex > 4) menuIndex = 0;
            }
            else if (key == 'O') { 
                executeMenuAction();
            }
            else if (key == '#') { 
                currentState = ST_VIEW_LIST;
            }
            return true;
        }

        // 3. FILE LIST VIEW
        if (currentState == ST_VIEW_LIST) {
            if (key == 'U') {
                selectedIndex--;
                scrollNameOffset = -3;
                if (selectedIndex < 0) selectedIndex = 0;
                if (selectedIndex < scrollOffset) scrollOffset = selectedIndex;
            }
            else if (key == 'D') {
                selectedIndex++;
                scrollNameOffset = -3;
                if (selectedIndex >= fileCount) selectedIndex = fileCount - 1;
                if (selectedIndex >= scrollOffset + 5) scrollOffset = selectedIndex - 4;
            }
            else if (key == 'O') { 
                if (fileList[selectedIndex].name == "..") {
                    goUp();
                } else {
                    currentState = ST_CONTEXT_MENU;
                    menuIndex = 0; 
                }
            }
            else if (key == 'L') { 
                 return false; 
            }
        }

        return true;
    }

private:
    // --- LOGIC ---

    void refreshStats() {
        // Dùng wrapper class Storage
        totalBytes = storage.totalBytes();
        usedBytes = storage.usedBytes();
    }

    void loadFolder(String path) {
        currentPath = path;
        fileCount = 0;
        selectedIndex = 0;
        scrollOffset = 0;

        // Dùng wrapper class Storage để mở thư mục
        fs::File root = storage.openFile(path);
        if (!root || !root.isDirectory()) return;

        if (path != "/") {
            fileList[fileCount++] = {"..", true, 0};
        }

        fs::File file = root.openNextFile();
        while (file && fileCount < MAX_FILES_PER_PAGE) {
            String fName = String(file.name());
            if (!fName.startsWith(".")) { 
                 fileList[fileCount] = {fName, file.isDirectory(), (uint32_t)file.size()};
                 fileCount++;
            }
            file = root.openNextFile();
        }
        root.close();
    }

    void goUp() {
        if (currentPath == "/") return;
        int lastSlash = currentPath.lastIndexOf('/');
        if (lastSlash == 0) loadFolder("/");
        else loadFolder(currentPath.substring(0, lastSlash));
    }

    void executeMenuAction() {
        ContextOption opt = (ContextOption)menuIndex;
        FileEntry target = fileList[selectedIndex];
        String fullPath = currentPath + (currentPath == "/" ? "" : "/") + target.name;

        switch (opt) {
            case OPT_OPEN:
                if (target.isFolder) {
                    loadFolder(fullPath);
                    currentState = ST_VIEW_LIST;
                } else {
                    currentState = ST_VIEW_LIST; // Chưa hỗ trợ mở file
                }
                break;
            
            case OPT_NEW_FOLDER:
                currentState = ST_INPUT_NEW;
                keyboard.begin("New Folder Name:", &inputBuffer);
                break;

            case OPT_RENAME:
                if (target.name == "..") break; 
                currentState = ST_INPUT_RENAME;
                inputBuffer = target.name; 
                keyboard.begin("Rename to:", &inputBuffer);
                break;

            case OPT_DELETE:
                if (target.name == "..") break;
                // Gọi hàm tương ứng của Storage
                if (target.isFolder) storage.removeDir(fullPath);
                else storage.removeFile(fullPath);
                
                refreshStats();
                loadFolder(currentPath);
                currentState = ST_VIEW_LIST;
                break;

            case OPT_CANCEL:
                currentState = ST_VIEW_LIST;
                break;
        }
    }

    // --- DRAWING ---
    void drawHeader(TFT_eSprite* spr, int x, int y, int w) {
        spr->fillRect(x, y, w, 38, 0x2124); 

        spr->setTextColor(WHITE, 0x2124);
        spr->setCursor(x + 5, y + 5);
        String displayPath = currentPath;
        if(displayPath.length() > 20) displayPath = "..." + displayPath.substring(displayPath.length() - 17);
        spr->print(displayPath);

        int barX = x + 5;
        int barY = y + 20;
        int barW = w - 10;
        int barH = 8;
        
        spr->drawRect(barX, barY, barW, barH, WHITE);
        
        // Tránh chia cho 0
        float percent = 0;
        if (totalBytes > 0) percent = (float)usedBytes / (float)totalBytes;
        
        int fillW = (int)(barW * percent);
        uint16_t color = (percent > 0.9) ? RED : GREEN;
        spr->fillRect(barX + 1, barY + 1, fillW - 2, barH - 2, color);

        spr->setCursor(x + 5, y + 30);
        spr->setTextSize(1);
        spr->setTextColor(LIGHTGREY, 0x2124);
        spr->print(formatBytes(usedBytes) + "/" + formatBytes(totalBytes));
    }

    void drawFileList(TFT_eSprite* spr, int x, int y, int w, int h, bool isFocused) {
        int itemH = 15;     // Chiều cao mỗi dòng
        int maxLines = 5;   // Số dòng tối đa có thể hiển thị

        spr->setTextWrap(false);

        for (int i = 0; i < maxLines; i++) {
            int idx = scrollOffset + i;
            if (idx >= fileCount) break;

            int lineY = y + (i * itemH);
            
            bool isSelected = (idx == selectedIndex);
            if (isSelected && isFocused && currentState == ST_VIEW_LIST) {
                spr->fillRect(x, lineY, w, itemH, BLUE); 
                spr->setTextColor(WHITE, BLUE);

                // --- LOGIC VẼ CHỮ CHẠY ---
                String name = fileList[idx].name;
                if (name.length() > 13) {
                    // Tính toán chuỗi con để hiển thị
                    // Kỹ thuật này cắt chuỗi để tạo hiệu ứng chạy
                    int startChar = (scrollNameOffset > 0) ? scrollNameOffset : 0;
                    String displayName = name.substring(startChar);
                    
                    // Vẽ text
                    spr->setCursor(x + 20, lineY + 3);
                    spr->print(displayName);
                } else {
                    spr->setCursor(x + 20, lineY + 3);
                    spr->print(name);
                }

            } else {
                spr->setTextColor(WHITE, BLACK); 
                spr->setCursor(x + 20, lineY + 3);
                if(fileList[idx].name.length() > 13) {
                    String shortName = fileList[idx].name.substring(0, 10) + "...";
                    spr->print(shortName);
                } 
                else spr->print(fileList[idx].name);
            }

            spr->setCursor(x + 1, lineY + 3);
            if (fileList[idx].isFolder) {
                spr->setTextColor(YELLOW); 
                spr->print("[D] ");
            } else {
                spr->setTextColor(CYAN);
                spr->print("[F] ");
            }
        }
    }

    void drawContextMenu(TFT_eSprite* spr, int x, int y, int w, int h) {
        spr->fillRoundRect(x, y, w, h, 4, DARKGREY);
        spr->drawRoundRect(x, y, w, h, 4, WHITE);

        spr->setTextColor(WHITE, DARKGREY);
        spr->setTextDatum(MC_DATUM);
        spr->drawString("- OPTIONS -", x + w/2, y + 10);
        spr->setTextDatum(TL_DATUM); 

        int startY = y + 25;
        for (int i = 0; i < 5; i++) {
            int lineY = startY + (i * 14);
            
            if (i == menuIndex) {
                spr->fillRect(x + 5, lineY - 2, w - 10, 14, BLUE);
                spr->setTextColor(WHITE, BLUE);
            } else {
                spr->setTextColor(WHITE, DARKGREY);
            }
            
            spr->setCursor(x + 15, lineY);
            spr->print(menuOptions[i]);
        }
    }
};

#endif