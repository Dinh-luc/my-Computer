#ifndef _STORAGE_H
#define _STORAGE_H

#include "SD_MMC.h"
#include <FS.h>

#define SD_CLK 39
#define SD_CMD 38
#define SD_D0  40

class Storage {
private:
    bool isSDReady = false;

public:
    bool initSD() {
        if(!SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0)) {
            Serial.println("❌ SD_MMC: Đổi chân thất bại!");
            isSDReady = false;
            return false;
        }
        if (!SD_MMC.begin("/sdcard", true, false, 20)) {
            Serial.println("⚠️ CẢNH BÁO: Không tìm thấy thẻ nhớ SD!");
            isSDReady = false;
        } else {
            Serial.printf("✅ SD_MMC Ready! Dung lượng: %llu MB\n", SD_MMC.cardSize() / (1024 * 1024));
            isSDReady = true;
        }
        return isSDReady;
    }

    // --- CHỈ LÀM VIỆC VỚI THẺ NHỚ ---
    bool exists(String path) {
        return isSDReady ? SD_MMC.exists(path) : false;
    }

    fs::File openFile(String path, const char* mode = FILE_READ, const bool create = false) {
        if (isSDReady) return SD_MMC.open(path.c_str(), mode, create); 
        return fs::File(); 
    }

    void createDir(String path) {
        if (isSDReady && !SD_MMC.exists(path)) SD_MMC.mkdir(path);
    }

    void removeFile(String path) {
        if (isSDReady && SD_MMC.exists(path)) SD_MMC.remove(path);
    }

    void removeDir(String path) {
        if (isSDReady && SD_MMC.exists(path)) SD_MMC.rmdir(path);
    }

    void reName(String oldPath, String newPath) {
        if (isSDReady && SD_MMC.exists(oldPath)) SD_MMC.rename(oldPath, newPath);
    }

    uint64_t totalBytes() { return isSDReady ? SD_MMC.totalBytes() : 0; }
    uint64_t usedBytes() { return isSDReady ? SD_MMC.usedBytes() : 0; }
};

#endif