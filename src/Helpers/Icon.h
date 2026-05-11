#ifndef _HELPER_ICON_H
#define _HELPER_ICON_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>

#include "Helpers/Storage.h" 

extern Storage storage; 

namespace IconHelper {
    
    struct DecodeState {
        fs::File file;
        TFT_eSprite* spr;
        int x;
        int y;
    };

    static DecodeState* currentState = nullptr;

    // --- CÁC HÀM CALLBACK CHO THẺ NHỚ SD (Giữ nguyên logic của bạn) ---
    static void* pngOpen(const char *filename, int32_t *size) {
        if (currentState == nullptr) return nullptr;
        currentState->file = storage.openFile(filename, FILE_READ);
        if (!currentState->file) return nullptr;
        *size = currentState->file.size();
        return &(currentState->file); 
    }

    static void pngClose(void *handle) {
        if (currentState && currentState->file) currentState->file.close();
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
        
        currentState->spr->pushImage(currentState->x, currentState->y + pDraw->y, pDraw->iWidth, 1, lineBuffer, 0x1FF8);
        return 1; 
    }

    // =========================================================
    // HÀM 1: TẢI ICON TỪ THẺ NHỚ SD (Dùng Callbacks)
    // =========================================================
    static bool drawIconFromSD(TFT_eSprite* spr, String path, int x, int y) {
        if (!storage.exists(path)) return false; 
        
        DecodeState state;
        state.spr = spr;
        state.x = x;
        state.y = y;
        currentState = &state;

        PNG* localPng = new PNG(); 
        bool success = false;
        
        if (localPng->open(path.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw) == PNG_SUCCESS) {
            localPng->decode(localPng, 0); 
            success = true;
        }
        
        localPng->close(); 
        delete localPng; 
        if (state.file) state.file.close(); 
        currentState = nullptr; 
        
        return success;
    }

    // =========================================================
    // HÀM 2: TẢI ICON TỪ MÃ HEX (PROGMEM) - Siêu tốc độ
    // =========================================================
    static bool drawIconFromROM(TFT_eSprite* spr, const uint8_t* pngArray, uint32_t arraySize, int x, int y) {
        DecodeState state;
        state.spr = spr;
        state.x = x;
        state.y = y;
        currentState = &state;

        PNG* localPng = new PNG();
        bool success = false;
        
        // Đường cao tốc: Gọi hàm openFLASH nạp trực tiếp mảng array từ PROGMEM, bỏ qua Callbacks
        if (localPng->openFLASH((uint8_t*)pngArray, arraySize, pngDraw) == PNG_SUCCESS) {
            localPng->decode(NULL, 0);
            success = true;
        }
        
        localPng->close();
        delete localPng;
        currentState = nullptr;
        
        return success;
    }
}

#endif