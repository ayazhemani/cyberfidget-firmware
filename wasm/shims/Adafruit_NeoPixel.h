// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023-2026 Dismo Industries LLC

#ifndef WASM_ADAFRUIT_NEOPIXEL_H
#define WASM_ADAFRUIT_NEOPIXEL_H

#include <cstdint>

#define NEO_GRBW 0x06
#define NEO_GRB  0x01
#define NEO_KHZ800 0x0100

class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel() : _numPixels(0) {}
    Adafruit_NeoPixel(uint16_t n, int16_t, uint8_t = NEO_GRB + NEO_KHZ800)
        : _numPixels(n) {
        for (int i = 0; i < 8; i++) {
            _r[i] = _g[i] = _b[i] = _w[i] = 0;
        }
    }

    void begin() {}

    void show() {
        _needsShow = true;
    }

    void setPixelColor(uint16_t n, uint32_t c) {
        if (n >= _numPixels || n >= 8) return;
        _w[n] = (c >> 24) & 0xFF;
        _r[n] = (c >> 16) & 0xFF;
        _g[n] = (c >>  8) & 0xFF;
        _b[n] =  c        & 0xFF;
    }

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) {
        return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    uint32_t ColorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255);

    static uint32_t gamma32(uint32_t c) { return c; }

    uint16_t numPixels() const { return _numPixels; }

    uint32_t getPixelColor(uint16_t n) const {
        if (n >= _numPixels || n >= 8) return 0;
        return Color(_r[n], _g[n], _b[n], _w[n]);
    }

    void getLedRGBW(uint16_t n, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &w) const {
        if (n >= _numPixels || n >= 8) { r=g=b=w=0; return; }
        r = _r[n]; g = _g[n]; b = _b[n]; w = _w[n];
    }

    bool needsShow() const { return _needsShow; }
    void clearNeedsShow() { _needsShow = false; }

private:
    uint16_t _numPixels;
    uint8_t _r[8], _g[8], _b[8], _w[8];
    bool _needsShow = false;
};

#endif
