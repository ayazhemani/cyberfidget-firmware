#include "Flashlight.h"
#include "RGBController.h"   // For LED strip control and pixel definitions
#include "globals.h"         // For sliderPosition_8Bits, flashlightStatus, etc.
#include "SSD1306Wire.h"     // For display object

extern SSD1306Wire display;  // Global display object

void flashlightSwitch(bool flashlightEnable) {
    if (flashlightEnable) {
        // Use brightness derived from sliderPosition_8Bits for consistency
        uint8_t brightness = sliderPosition_8Bits;

        // Set specific pixels for flashlight mode using RGBController's strip and pixel definitions
        strip.setPixelColor(pixel_Back, strip.Color(brightness, brightness, brightness, brightness));
        strip.setPixelColor(pixel_Front_Top, strip.Color(brightness, brightness, brightness, brightness));
        strip.setPixelColor(pixel_Front_Middle, strip.Color(brightness, brightness, brightness, brightness));
        strip.setPixelColor(pixel_Front_Bottom, strip.Color(brightness, brightness, brightness, brightness));
        strip.show();
        flashlightStatus = true;
    }
    else if (flashlightStatus) {
        // Turn off LEDs using RGBController's setColorsOff for consistency
        setColorsOff();
        flashlightStatus = false;
    }
}

void drawFlashlight() {
    // Clear and set up the display for the flashlight screen
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 18, "Flashlight");
    display.display();
    
    // Enable the flashlight
    flashlightSwitch(true);
}
