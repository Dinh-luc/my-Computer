#ifndef _STORAGE_H
#define _STORAGE_H

#include "SD_MMC.h"
#include <FS.h>

#define SD_CLK 39
#define SD_CMD 38
#define SD_D0  40

class Storage {
private:

public:
    bool initSD() {
        if(!SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0)) {
            Serial.println("FileStorage log: ❌ SD_MMC: đổi chân thất bại!");
            return false;
        }

        if (!SD_MMC.begin("/sdcard", true, false, 10)) {
            Serial.println("FileStorage log: ❌ SD_MMC Mount Failed!");
            return false;
        }

        uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
        Serial.printf("FileStorage log: ✅ SD_MMC Mounted! Dung lượng: %llu MB", cardSize);
        return true;
    }

    bool exists(String path) {
        return SD_MMC.exists(path);
    }

    void showAllFile() {
        fs::File root = SD_MMC.open("/");
        if (!root) {
            Serial.println("FileStorage log: ❌ Failed to open root directory!");
            return;
        }

        Serial.println("FileStorage log: 📂 File list in SD:");
        fs::File f = root.openNextFile();
        while (f) {
            Serial.printf("   %s (%u bytes)", f.name(), (unsigned int)f.size());
            f = root.openNextFile();
        }
    }

    void createDir(String path) {
        if (SD_MMC.exists(path)) {
            Serial.printf("FileStorage log: ⚠️ Thư mục %s đã tồn tại.\n", path.c_str());
            return;
        }

        if (!SD_MMC.mkdir(path)) {
            Serial.printf("FileStorage log: ❌ Không thể tạo thư mục %s\n", path.c_str());
            return;
        }

        Serial.printf("FileStorage log: ✅ Tạo thư mục thành công: %s\n", path.c_str());
    }

    void removeDir(String path) {
        if (!SD_MMC.exists(path)) {
            Serial.printf("FileStorage log: ⚠️ Thư mục %s không tồn tại.\n", path.c_str());
            return;
        }
        if (!SD_MMC.rmdir(path)) {
            Serial.printf("FileStorage log: ❌ Không thể xóa thư mục %s\n", path.c_str());
            return;
        }
        Serial.printf("FileStorage log: ✅ Xóa thư mục thành công: %s\n", path.c_str());
    }

    void removeFile(String path) {
        if (SD_MMC.remove(path)) {
            Serial.printf("Storage: Removed File %s\n", path.c_str());
        } else {
            Serial.printf("Storage: ❌ Remove File Failed %s\n", path.c_str());
        }
    }

    void rename(String oldPath, String newPath) {
        if (!SD_MMC.exists(oldPath)) {
            Serial.printf("FileStorage log: ⚠️ File/Thư mục %s không tồn tại.\n", oldPath.c_str());
            return;
        }
        if (SD_MMC.exists(newPath)) {
            Serial.printf("FileStorage log: ⚠️ File/Thư mục %s đã tồn tại.\n", newPath.c_str());
            return;
        }
        if (!SD_MMC.rename(oldPath, newPath)) {
            Serial.printf("FileStorage log: ❌ Không thể đổi tên %s thành %s\n", oldPath.c_str(), newPath.c_str());
            return;
        }
        Serial.printf("FileStorage log: ✅ Đổi tên thành công: %s -> %s\n", oldPath.c_str(), newPath.c_str());
    }

    fs::File openFile(String path, const char* mode = FILE_READ, const bool create = false) {
        return SD_MMC.open(path.c_str(), mode, create);
    }

    uint64_t totalBytes() {
        return SD_MMC.totalBytes();
    }

    uint64_t usedBytes() {
        return SD_MMC.usedBytes();
    }

};

#endif