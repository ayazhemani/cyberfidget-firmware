#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H

#include <Arduino.h>

/**
 * @brief Toggle the flashlight LEDs on or off.
 * @param flashlightEnable Set true to turn on the flashlight, false to turn it off.
 */
void flashlightController(bool flashlightEnable);

/**
 * @brief Display the flashlight screen and enable the flashlight.
 */
void drawFlashlight();

extern bool flashlightStatus;

#endif  // FLASHLIGHT_H
