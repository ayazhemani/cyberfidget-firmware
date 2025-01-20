#include "RGBController.h"
#include "globals.h"

typedef void (*RGBW)(void);
RGBW colors[] = {red, green, blue, white, halfWHITE};

int colorLength = (sizeof(colors) / sizeof(RGBW));

Adafruit_NeoPixel strip = Adafruit_NeoPixel(RGB_COUNT, PIN, NEO_RGBW + NEO_KHZ800);

static void colorSet(uint32_t c, uint8_t wait) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  updateStrip();
}

void initRGB() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void updateStrip() {
  strip.show();
}

void red()         { colorSet(strip.Color(0, 25, 0, 0), 0); }
void green()       { colorSet(strip.Color(25, 0, 0, 0), 0); }
void blue()        { colorSet(strip.Color(0, 0, 25, 0), 0); }
void white()       { colorSet(strip.Color(0, 0, 0, 50), 0); }
void halfWHITE()   { colorSet(strip.Color(0, 0, 0, 25), 0); }

void setRandomColors() {
  uint8_t maxBrightness = 10;
  for (int i = 1; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(random(0, maxBrightness), random(0, maxBrightness), random(0, maxBrightness), 0));
  }
  updateStrip();
}

void setDeterminedColors(uint8_t colorR, uint8_t colorG, uint8_t colorB, uint8_t colorW) {
  for (int i = 1; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(colorR, colorG, colorB, colorW));
  }
  updateStrip();
}

void setColorsOff() {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0,0,0,0));
  }
  updateStrip();
}

void rainbow(int wait) {
  static long firstPixelHue = 0;
  firstPixelHue += 256;
  if (firstPixelHue >= 5 * 65536) {
    firstPixelHue = 0;
  }
  for(int i = 1; i < strip.numPixels(); i++) {
    int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  updateStrip();
}

// Function to map input (0 to 4095) to a rainbow progression (red -> orange -> yellow -> green t-> blue -> violet)
void mapToRainbow(int input, uint8_t dim, uint8_t &red, uint8_t &green, uint8_t &blue) {
    // Ensure input and dim are within bounds
    input = input < 0 ? 0 : (input > 4095 ? 4095 : input);
    dim = dim > 255 ? 255 : dim;

    // Normalize input to range 0.0–1.0
    float normalized = (float)input / 4095.0;

    // Calculate which segment of the rainbow we're in
    float x, y, z;
    if (normalized < 0.2) {  // Red → Orange
        float t = normalized / 0.2;
        x = 1.0;
        y = t;
        z = 0.0;
    } else if (normalized < 0.4) {  // Orange → Yellow
        float t = (normalized - 0.2) / 0.2;
        x = 1.0;
        y = 1.0;
        z = 0.0;
    } else if (normalized < 0.6) {  // Yellow → Green
        float t = (normalized - 0.4) / 0.2;
        x = 1.0 - t;
        y = 1.0;
        z = 0.0;
    } else if (normalized < 0.8) {  // Green → Blue
        float t = (normalized - 0.6) / 0.2;
        x = 0.0;
        y = 1.0 - t;
        z = t;
    } else {  // Blue → Violet
        float t = (normalized - 0.8) / 0.2;
        x = t;
        y = 0.0;
        z = 1.0;
    }

    // Scale RGB values by brightness (dim)
    red = (uint8_t)((y * dim));
    green = (uint8_t)((x * dim));
    blue = (uint8_t)((z * dim));
}