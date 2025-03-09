/**

License Placeholder

*/

// // Main Include
#include <Arduino.h>  // Required for Arduino-specific types
#include "AppManager.h"


void setup() {
  AppManager::setup();
}
void loop() {
  AppManager::loop();
}

// //Comment out above code and remove comments below for One Application setup
// #include "globals.h"
// #include "HAL.h"

// void setup() {
//   HAL::initEasyEverything();
// }

// void loop() {
//   HAL::loopHardware();

//   // Update the display based on the slider position
//   HAL::clearDisplay();
//   HAL::drawString(0, 0, "Hello, World!");
//   HAL::drawString(0, 10, "Slider: " + String(sliderPosition_Percentage_Inverted_Filtered) + "%");
//   HAL::updateDisplay();

//   // Change the brightness of the RGB LED based on the slider position
//   int value = map(sliderPosition_Percentage_Inverted_Filtered, 0, 100, 0, 255);
//   HAL::setRgbLed(pixel_Front_Top, value, 0, 0, 0);
// }
