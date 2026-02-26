#ifndef _LED_CONTROL_H
#define _LED_CONTROL_H

#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define LED_COUNT 1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

class LEDControl {
public:
    static void init() {
        strip.begin();
        strip.show();
        strip.setBrightness(50);
    }

    static void setColor(uint8_t r, uint8_t g, uint8_t b) {
        strip.setPixelColor(0, strip.Color(r, g, b));
        strip.show();
    }
};

#endif 