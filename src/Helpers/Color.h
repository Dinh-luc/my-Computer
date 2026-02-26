#ifndef _COLOR_H
#define _COLOR_H

#include <TFT_eSPI.h>

// ================= Màu cơ bản =================
#define BLACK           0x0000
#define WHITE           0xFFFF
#define RED             0xF800
#define GREEN           0x07E0
#define BLUE            0x001F
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define ORANGE          0xFD20
#define GREENYELLOW     0xAFE5

// ================= Màu tối (Dùng cho nền/border) =================
#define NAVY            0x000F
#define DARKGREEN       0x03E0
#define DARKCYAN        0x03EF
#define MAROON          0x7800
#define PURPLE          0x780F
#define OLIVE           0x7BE0
#define LIGHTGREY       0xC618
#define DARKGREY        0x7BEF
#define DARKGREY_2      0x2124  

// ================= Màu Pastel / Hiện đại (Cho Custom Theme) =================
#define TEAL            0x0410
#define EMERALD         0x57E8
#define CORAL           0xFBEA
#define SALMON          0xFC0E
#define GOLD            0xFEA0
#define SKYBLUE         0x867D
#define VIOLET          0x915C
#define CRIMSON         0xD8A7
#define AQUA            0x04FF
#define MINT            0x7FF3

// Danh sách màu để dùng cho Color Picker (12 màu)
static const uint16_t PALETTE_COLORS[] = {
    RED, ORANGE, YELLOW, GREEN,
    CYAN, BLUE, PURPLE, MAGENTA,
    TEAL, CORAL, VIOLET,  SKYBLUE
};

#endif