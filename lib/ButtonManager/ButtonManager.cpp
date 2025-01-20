#include "ButtonManager.h"

static const int buttonPinsLocal[] = {36, 37, 38, 39, 34, 15};

void ButtonManager::init() {
  for(int i=0; i<6; i++){
    pinMode(buttonPinsLocal[i], INPUT_PULLUP);
    lastDebounceTime[i] = 0;
    buttonCounter[i] = 0;
  }
  // attachInterrupts if you prefer
}

void ButtonManager::update(unsigned long now) {
  // poll or do anything you need
  for(int i=0; i<6; i++){
    int reading = digitalRead(buttonPinsLocal[i]);
    // Compare to lastDebounceTime, etc. 
    // If pressed, handleButtonPress(i);
  }
}

void ButtonManager::handleButtonPress(int index) {
  buttonCounter[index]++;
  // Could do callback or store press state for main to read
}
