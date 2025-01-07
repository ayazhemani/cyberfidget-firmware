#include "PixelWaterfallGame.h"
#include <esp_task_wdt.h>  // If you need watchdog functionality on ESP

PixelWaterfallGame* PixelWaterfallGame::instance = nullptr;

PixelWaterfallGame::PixelWaterfallGame(SSD1306Wire& disp)
    : display(disp), inertia(1.0f), damping(0.98f)
{
    instance = this;
    resetPixels();
}

void PixelWaterfallGame::update(float Ax, float Ay) {
    float normalizedAx = Ax / 1030.0f;
    float normalizedAy = Ay / 1030.0f;

    for (int i = 0; i < pixels.size(); ++i) {
        auto &pixel = pixels[i];

        // Update velocities
        pixel.vx += normalizedAx * inertia;
        pixel.vy += normalizedAy * inertia;
        pixel.vx *= damping;
        pixel.vy *= damping;

        // Update position
        pixel.x += pixel.vx;
        pixel.y += pixel.vy;

        // Screen boundaries
        // ...
        
        // Debug print for first 5 pixels
        if (i < 5) {
            Serial.print("Pixel "); Serial.print(i);
            Serial.print(": x="); Serial.print(pixel.x);
            Serial.print(", y="); Serial.print(pixel.y);
            Serial.print(", vx2="); Serial.print(pixel.vx);
            Serial.print(", vy="); Serial.println(pixel.vy);
        }
    }

    // Clear, setPixel, display, etc...
}


void PixelWaterfallGame::resetPixels() {
    pixels.clear();
    pixels.reserve(NUM_PIXELS);

    for (int i = 0; i < NUM_PIXELS; ++i) {
        Pixel p;
        p.x = static_cast<float>(random(SCREEN_WIDTH));
        p.y = static_cast<float>(random(SCREEN_HEIGHT));
        // Start with zero velocity or very small velocity
        p.vx = 0.0f;
        p.vy = 0.0f;
        pixels.push_back(p);
    }
}


void PixelWaterfallGame::setInertia(float newInertia) {
    inertia = newInertia;
}

void PixelWaterfallGame::setDamping(float newDamping) {
    damping = newDamping;
}
