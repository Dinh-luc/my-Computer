#ifndef _HELPER_ICON_H
#define _HELPER_ICON_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>

// Nhúng thư viện Storage của bạn
#include "Helpers/Storage.h" 

// Sử dụng đối tượng storage toàn cục đã được khởi tạo từ hệ thống
extern Storage storage; 

namespace IconHelper {
    
    struct DecodeState {
        fs::File file;
        TFT_eSprite* spr;
        int x;
        int y;
    };

    static DecodeState* currentState = nullptr;

    static void* pngOpen(const char *filename, int32_t *size) {
        if (currentState == nullptr) return nullptr;
        
        currentState->file = storage.openFile(filename, FILE_READ);
        if (!currentState->file) return nullptr;
        
        *size = currentState->file.size();
        return &(currentState->file); 
    }

    static void pngClose(void *handle) {
        if (currentState && currentState->file) {
            currentState->file.close();
        }
    }

    static int32_t pngRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
        if (!currentState || !currentState->file) return 0;
        return currentState->file.read(buffer, length);
    }

    static int32_t pngSeek(PNGFILE *handle, int32_t position) {
        if (!currentState || !currentState->file) return 0;
        currentState->file.seek(position);
        return currentState->file.position(); 
    }

    static int pngDraw(PNGDRAW *pDraw) {
        if (!currentState || !currentState->spr) return 1;
        
        uint16_t lineBuffer[256]; 
        if (pDraw->iWidth > 256) return 1; 
        
        PNG* pngPtr = (PNG*)pDraw->pUser; 
        pngPtr->getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xFF00FF);
        
        currentState->spr->pushImage(
            currentState->x, 
            currentState->y + pDraw->y, 
            pDraw->iWidth, 
            1, 
            lineBuffer, 
            0x1FF8
        );
        
        return 1; 
    }

    // =========================================================
    // HÀM CHÍNH GỌI TỪ APP MANAGER
    // =========================================================
    static void drawIcon(TFT_eSprite* spr, String path, int x, int y) {
        if (!storage.exists(path)) return; // Dùng storage kiểm tra file
        
        DecodeState state;
        state.spr = spr;
        state.x = x;
        state.y = y;
        currentState = &state;

        // FIX LỖI STACK OVERFLOW: Đưa PNG lên vùng nhớ Heap (Dùng con trỏ)
        PNG* localPng = new PNG(); 
        
        int rc = localPng->open(path.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw);
        if (rc == PNG_SUCCESS) {
            // Truyền con trỏ localPng vào pUser
            localPng->decode(localPng, 0); 
        }
        
        localPng->close(); 
        delete localPng; // GIẢI PHÓNG RAM QUAN TRỌNG: Ngăn chặn tràn bộ nhớ (Memory Leak)
        
        if (state.file) state.file.close(); 
        currentState = nullptr; 
    }
}

#endif