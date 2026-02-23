#include "HAL.h"
#include "globals.h"
#include "DisplayProxy.h"
#include "ButtonManager.h"
#include "RGBController.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// ---- App selection (set via CMake -DWASM_APP=xxx) ----
#if defined(WASM_APP_DINOGAME)
    #include "DinoGame.h"
    #define APP_INSTANCE  dinoGame
#elif defined(WASM_APP_FLASHLIGHT)
    #include "Flashlight.h"
    #define APP_INSTANCE  flashlight
#elif defined(WASM_APP_CUSTOM)
    #include "app_include.h"
    #define APP_INSTANCE  WASM_CUSTOM_INSTANCE
#endif

extern uint8_t wasm_button_states[];
extern int wasm_analog_values[];

// ---- Exported C functions for JS bridge ----
extern "C" {

void wasm_button_press(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < 6) {
        wasm_button_states[buttonIndex] = 1;
    }
}

void wasm_button_release(int buttonIndex) {
    if (buttonIndex >= 0 && buttonIndex < 6) {
        wasm_button_states[buttonIndex] = 0;
    }
}

void wasm_set_slider(int value) {
    wasm_analog_values[0] = 4095 - value;
}

const uint8_t* wasm_get_framebuffer() {
    return HAL::realDisplay().getBuffer();
}

int wasm_get_framebuffer_size() {
    return HAL::realDisplay().getBufferSize();
}

void wasm_stop() {
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
}

} // extern "C"

// ---- Default demo (used when no app is selected) ----
#ifndef APP_INSTANCE

static int demoFrame = 0;

static void demoBegin() { demoFrame = 0; }

static void demoUpdate() {
    auto& display = HAL::displayProxy();
    display.clear();

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 5, "CYBER FIDGET");
    display.drawString(64, 20, "WASM Emulator");

    display.drawRect(10, 38, 108, 12);
    int fill = (demoFrame % 100);
    display.fillRect(12, 40, fill, 8);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(10, 52, "Slider: " + String(sliderPosition_8Bits_Filtered));

    display.display();
    demoFrame++;
}

#endif // !APP_INSTANCE

// ---- Main loop called by Emscripten ----
static void mainLoop() {
    HAL::loopHardware();
    updateStrip();

    ButtonEvent ev;
    while (HAL::buttonManager().getNextEvent(ev)) {
        if (HAL::buttonManager().hasCallback(ev.buttonIndex)) {
            auto cb = HAL::buttonManager().getCallback(ev.buttonIndex);
            if (cb) cb(ev);
        }
    }

#ifdef APP_INSTANCE
    APP_INSTANCE.update();
#else
    demoUpdate();
#endif
}

int main() {
    HAL::initEasyEverything();

#ifdef APP_INSTANCE
    APP_INSTANCE.begin();
#else
    demoBegin();
#endif

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 50, 1);
#else
    for (int i = 0; i < 300; i++) mainLoop();
#endif

    return 0;
}
