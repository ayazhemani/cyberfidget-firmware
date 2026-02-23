#include "Arduino.h"
#include "Wire.h"
#include "WiFi.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// ---- Framebuffer push to JS (single definition for the EM_JS symbol) ----
#ifdef __EMSCRIPTEN__
EM_JS(void, js_push_framebuffer, (const uint8_t* buf, int len), {
    if (Module.onFrameReady) {
        Module.onFrameReady(HEAPU8.slice(buf, buf + len));
    }
});
#else
void js_push_framebuffer(const uint8_t* buf, int len) { (void)buf; (void)len; }
#endif

// ---- Serial output to JS ----
#ifdef __EMSCRIPTEN__
EM_JS(void, js_serial_write, (const char* str, int len), {
    if (Module.onSerialOutput) {
        Module.onSerialOutput(UTF8ToString(str, len));
    }
});
#else
void js_serial_write(const char* str, int len) { (void)str; (void)len; }
#endif

// ---- Timing ----
static uint32_t s_startMs = 0;
static bool s_inited = false;

uint32_t millis() {
#ifdef __EMSCRIPTEN__
    return (uint32_t)emscripten_get_now();
#else
    return 0;
#endif
}

void delay(uint32_t ms) {
#ifdef __EMSCRIPTEN__
    emscripten_sleep(ms);
#else
    (void)ms;
#endif
}

// ---- Button / analog state (set by JS) ----
uint8_t wasm_button_states[8] = {0};
int wasm_analog_values[8] = {0};

const int wasm_pin_map[8] = {36, 37, 38, 39, 34, 15, -1, -1};

// ---- Serial singleton ----
HardwareSerial Serial;

// ---- Wire singleton ----
TwoWire Wire;

// ---- WiFi singleton ----
WiFiClass WiFi;

// ---- NeoPixel color utility ----
#include "Adafruit_NeoPixel.h"

uint32_t Adafruit_NeoPixel::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {
    uint8_t r, g, b;
    uint16_t h = hue / 256;
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;
    uint8_t p = (val * (255 - sat)) >> 8;
    uint8_t q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
    uint8_t t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:  r = val; g = t;   b = p;   break;
        case 1:  r = q;   g = val; b = p;   break;
        case 2:  r = p;   g = val; b = t;   break;
        case 3:  r = p;   g = q;   b = val; break;
        case 4:  r = t;   g = p;   b = val; break;
        default: r = val; g = p;   b = q;   break;
    }
    return Color(r, g, b);
}
