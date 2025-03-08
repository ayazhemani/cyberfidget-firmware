/**

License Placeholder

*/

// Main Include
#include <Arduino.h>  // Required for Arduino-specific types
#include "globals.h"
#include "AppManager.h"


void setup() {
  AppManager::setup();
}
void loop() {
  AppManager::loop();
}

// Comment out above code and remove comments below for One Application setup
/*
#include "HAL.h"

void setup() {
  HAL::initHardware();
  // ... do your game setup
}

void loop() {
  HAL::loopHardware();
  // ... your app logic
}
*/