#ifndef RGBCONTROLLER_H
#define RGBCONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Initialize NeoPixel strip and LED controls
void initRGB();
void updateStrip(); // New function to update the strip

// Color control functions
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

// Expose strip for global access if needed
//extern Adafruit_NeoPixel strip;

#endif
