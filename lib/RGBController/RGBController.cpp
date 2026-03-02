// RGBController.cpp
// Stubbed out: NeoPixel disabled for IRAM savings (~2-4KB from RMT ISR handlers).
// All functions are no-ops. Will be re-enabled when IRAM headroom is found.
#include "RGBController.h"

void initRGB() {}
void updateStrip() {}

void red() {}
void green() {}
void blue() {}
void white() {}
void halfWHITE() {}
void setRandomColors() {}
void setDeterminedColorsFront(uint8_t, uint8_t, uint8_t, uint8_t) {}
void setDeterminedColorsAll(uint8_t, uint8_t, uint8_t, uint8_t) {}
void setColorsOff() {}
void rainbow(int) {}

void mapToRainbow(int input, uint8_t dim, uint8_t &red, uint8_t &green, uint8_t &blue) {
    red = green = blue = 0;
}
