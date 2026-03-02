#ifndef RGBCONTROLLER_H
#define RGBCONTROLLER_H

#include <Arduino.h>

// RGB LED control functions - stubbed out when NeoPixel is disabled for IRAM savings.
// These will be re-enabled when we find IRAM headroom for the RMT driver.
void initRGB();
void updateStrip();

void red();
void green();
void blue();
void white();
void halfWHITE();
void setRandomColors();
void setDeterminedColorsFront(uint8_t colorR, uint8_t colorG, uint8_t colorB, uint8_t colorW);
void setDeterminedColorsAll(uint8_t colorR, uint8_t colorG, uint8_t colorB, uint8_t colorW);
void setColorsOff();
void rainbow(int wait);
void mapToRainbow(int input, uint8_t dim, uint8_t &red, uint8_t &green, uint8_t &blue);

#endif
