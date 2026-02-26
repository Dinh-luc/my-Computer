#ifndef _SENSORS_H
#define _SENSORS_H

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "RTClib.h"

// Chân I2C 
#define I2C_SDA 45  
#define I2C_SCL 47

class SensorManager {
private:
    Adafruit_MPU6050 mpu;
    RTC_DS3231 rtc;

public:
    // Dữ liệu cảm biến để Game truy cập
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float temp;

    bool init() {
        // 1. Khởi tạo bus I2C chung
        Wire.begin(I2C_SDA, I2C_SCL);

        bool success = true;

        // 2. Khởi tạo DS3231 (Địa chỉ 0x68)
        if (!rtc.begin()) {
            Serial.println("❌ Không tìm thấy DS3231");
            success = false;
        } else {
            Serial.println("✅ DS3231 OK (Addr: 0x68)");
        }

        // 3. Khởi tạo MPU6050 (Địa chỉ 0x69 - Do đã nối AD0 lên 3.3V)
        // Lưu ý: Phải truyền địa chỉ 0x69 vào hàm begin() nếu thư viện hỗ trợ, 
        // hoặc thư viện tự dò. Với Adafruit_MPU6050, nó tự dò hoặc set thủ công.
        
        if (!mpu.begin(0x69)) { // <--- QUAN TRỌNG: Gọi địa chỉ 0x69
            Serial.println("❌ Không tìm thấy MPU6050 (Check AD0 pin!)");
            // Thử fallback về 0x68 xem có quên nối dây không
            if (mpu.begin(0x68)) {
                Serial.println("⚠️ MPU6050 tìm thấy ở 0x68 -> XUNG ĐỘT VỚI RTC!");
            }
            success = false;
        } else {
            Serial.println("✅ MPU6050 OK (Addr: 0x69)");
            
            // Cấu hình MPU6050 cơ bản
            mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
            mpu.setGyroRange(MPU6050_RANGE_500_DEG);
            mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        }

        return success;
    }

    void update() {
        // Đọc dữ liệu MPU6050
        sensors_event_t a, g, t;
        mpu.getEvent(&a, &g, &t);

        accelX = a.acceleration.x;
        accelY = a.acceleration.y;
        accelZ = a.acceleration.z;
        
    }

    DateTime getTime() {
        return rtc.now();
    }

    // Lấy chuỗi thời gian HH:MM từ RTC
    String getTimeStr() {
        DateTime now = rtc.now();
        char buf[10];
        sprintf(buf, "%02d:%02d", now.hour(), now.minute());
        return String(buf);
    }

    // Ghi thời gian vào RTC
    void setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
        rtc.adjust(DateTime(year, month, day, hour, minute, second));
    }

};

#endif