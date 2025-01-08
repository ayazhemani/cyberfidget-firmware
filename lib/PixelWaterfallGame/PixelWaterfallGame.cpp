#include "PixelWaterfallGame.h"
#include <esp_task_wdt.h>  // If you need watchdog functionality on ESP

PixelWaterfallGame* PixelWaterfallGame::instance = nullptr;

PixelWaterfallGame::PixelWaterfallGame(SSD1306Wire& disp)
    : display(disp), inertia(.01f), damping(0.98f)
{
    instance = this;
    resetPixels();
}

void PixelWaterfallGame::update(float Ax, float Ay) {
    float clampedAx = constrain(Ax, -32768.0f, 32767.0f);
    float clampedAy = constrain(Ay, -32768.0f, 32767.0f);
    // Then scale or normalize
    float normalizedAx = clampedAx / 1030.0f;
    float normalizedAy = clampedAy / 1030.0f;


    // Update positions
    for (auto& pixel : pixels) {
        pixel.vx += normalizedAx * inertia;
        pixel.vy += normalizedAy * inertia;

        pixel.vx *= damping;
        pixel.vy *= damping;

        pixel.x += pixel.vx;
        pixel.y += pixel.vy;

        // Boundary checks
        if (pixel.x < 0) {
            pixel.x = 0;
            pixel.vx *= -0.5f;
        } else if (pixel.x >= SCREEN_WIDTH) {
            pixel.x = SCREEN_WIDTH - 1;
            pixel.vx *= -0.5f;
        }

        if (pixel.y < 0) {
            pixel.y = 0;
            pixel.vy *= -0.5f;
        } else if (pixel.y >= SCREEN_HEIGHT) {
            pixel.y = SCREEN_HEIGHT - 1;
            pixel.vy *= -0.5f;
        }
    }

    // Clear display once
    display.clear();

    // Make sure the color is set to WHITE (or whatever the library needs)
    // If your library has "setColor()", do it here:
    // display.setColor(WHITE);

    // Draw everything
    for (const auto& pixel : pixels) {
        int px = static_cast<int>(pixel.x);
        int py = static_cast<int>(pixel.y);

        if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
            // If your library has "drawPixel(x, y)", use that
            // If it only has "setPixel(x, y)", that's okay—just confirm doc
            display.setPixel(px, py);
        }
    }

    display.display();
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
