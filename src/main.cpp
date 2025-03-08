/**

License Placeholder

*/

// Main Include
#include <Arduino.h>  // Required for Arduino-specific types
#include "AppManager.h"


void setup() {
  AppManager::setup();
}
void loop() {
  AppManager::loop();
}

// Comment out above code and remove comments below for One Application setup
// #include "globals.h"
// #include "CFHAL.h"

// void setup() {
//   HAL::configureWakeupPins();
//   esp_log_level_set("*", ESP_LOG_VERBOSE);
//   esp_log_level_set(TAG_MAIN, ESP_LOG_VERBOSE);
//   HAL::initHardware();
//   ESP_LOGI(TAG_MAIN, "Setup() complete");

//   HAL::realDisplay().init();
//   HAL::drawText(0, 0, "Hello, World!");
//   HAL::updateDisplay();
// }

// void loop() {
//   HAL::loopHardware();
//   // ... your app logic
// }
