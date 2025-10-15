#include "RGBController.h"
#include "globals.h"
#include "HAL.h"

typedef void (*RGBW)(void);
RGBW colors[] = {red, green, blue, white, halfWHITE};
int colorLength = (sizeof(colors) / sizeof(RGBW));

// Reference alias for `HAL::strip()`
static Adafruit_NeoPixel& strip = HAL::strip();

// --- Update Throttle ---
static bool     s_dirty       = false;
static uint32_t s_lastShowMs  = 0;
static uint16_t s_minInterval = 17; // divide into 1000 for fps

// Cache of last "set all" color to avoid redundant updates
static uint8_t s_lastR = 0, s_lastG = 0, s_lastB = 0, s_lastW = 0;
static bool    s_lastAllValid = false;

static void markDirty() { s_dirty = true; }

static void colorSet(uint32_t c, uint8_t /*wait*/) {
  // This is a blanket setter; it invalidates last-all cache.
  s_lastAllValid = false;
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  markDirty();
}

void initRGB() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  s_lastR = s_lastG = s_lastB = s_lastW = 0;
  s_lastAllValid = true;
  s_lastShowMs = millis();
  s_dirty = false;
}

// Throttled show: only when dirty AND interval elapsed
void updateStrip() {
  uint32_t now = millis();
  if (!s_dirty) return;
  if ((now - s_lastShowMs) < s_minInterval) return;

  strip.show();                 // still blocking, but much less often
  s_lastShowMs = now;
  s_dirty = false;
}

void red()         { colorSet(strip.Color(0, 25, 0, 0), 0); }
void green()       { colorSet(strip.Color(25, 0, 0, 0), 0); }
void blue()        { colorSet(strip.Color(0, 0, 25, 0), 0); }
void white()       { colorSet(strip.Color(0, 0, 0, 50), 0); }
void halfWHITE()   { colorSet(strip.Color(0, 0, 0, 25), 0); }

void setRandomColors() {
  s_lastAllValid = false;
  uint8_t maxBrightness = 10;
  for (int i = 1; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(random(0, maxBrightness),
                                       random(0, maxBrightness),
                                       random(0, maxBrightness), 0));
  }
  markDirty();
}

/**
 * @brief Update front-only RGB LEDs to a determined color
 * 
 * @param colorR uint8_t Red
 * @param colorG uint8_t Green
 * @param colorB uint8_t Blue
 * @param colorW uint8_t White
 */
void setDeterminedColorsFront(uint8_t colorR, uint8_t colorG, uint8_t colorB, uint8_t colorW) {
  s_lastAllValid = false; // front-only changes invalidate "all" cache
  for (int i = 1; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(colorR, colorG, colorB, colorW));
  }
  markDirty();
}

/**
 * @brief Update all RGB LEDs to a determined color
 * 
 * @param colorR uint8_t Red
 * @param colorG uint8_t Green
 * @param colorB uint8_t Blue
 * @param colorW uint8_t White
 */
void setDeterminedColorsAll(uint8_t colorR, uint8_t colorG, uint8_t colorB, uint8_t colorW) {
  // Skip if identical to last "all" request
  if (s_lastAllValid &&
      colorR == s_lastR && colorG == s_lastG &&
      colorB == s_lastB && colorW == s_lastW) {
    return; // no buffer writes, no show
  }

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(colorR, colorG, colorB, colorW));
  }

  s_lastR = colorR; s_lastG = colorG; s_lastB = colorB; s_lastW = colorW;
  s_lastAllValid = true;
  markDirty();
}

void setColorsOff() {
  if (s_lastAllValid && s_lastR == 0 && s_lastG == 0 && s_lastB == 0 && s_lastW == 0) {
    return; // already off
  }
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
  s_lastR = s_lastG = s_lastB = s_lastW = 0;
  s_lastAllValid = true;
  markDirty();
}

void rainbow(int /*wait*/) {
  s_lastAllValid = false;
  static long firstPixelHue = 0;
  firstPixelHue += 256;
  if (firstPixelHue >= 5 * 65536) firstPixelHue = 0;

  for (int i = 1; i < strip.numPixels(); i++) {
    int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  markDirty();
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