#ifndef PIXEL_WATERFALL_GAME_H
#define PIXEL_WATERFALL_GAME_H

#include "SSD1306Wire.h"
#include <vector>

class PixelWaterfallGame {
public:
    struct Pixel {
        float x, y;   // Position
        float vx, vy; // Velocity
    };

    PixelWaterfallGame(SSD1306Wire& display);

    void update(float Ax, float Ay);   // Update pixels based on accelerometer input
    void resetPixels();                // Reset pixel positions
    void setInertia(float newInertia); // Set inertia factor
    void setDamping(float newDamping); // Set damping factor

private:
    static constexpr int SCREEN_WIDTH  = 128;
    static constexpr int SCREEN_HEIGHT = 64;
    
    // ↓ Try reducing this number to see clearer motion
    static constexpr int NUM_PIXELS = 2;

    SSD1306Wire& display;
    std::vector<Pixel> pixels;
    float inertia; // Controls how much Ax/Ay affect pixel movement
    float damping; // Controls how quickly velocity decreases

    static PixelWaterfallGame* instance;
};

#endif
