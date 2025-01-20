#include "ReactionTimeGame.h"
#include <esp_task_wdt.h>  // Include the header for the watchdog functions

extern volatile bool buttonPressed;  // Reference to global variable
//volatile int buttonStateReaction;
//int buttonPin;

ReactionTimeGame* ReactionTimeGame::instance = nullptr;
const unsigned long ReactionTimeGame::debounceDelay;

ReactionTimeGame::ReactionTimeGame(SSD1306Wire& disp, int btnPin)
    : display(disp), buttonPin(btnPin), gameStarted(false), waitingForReaction(false),
      delayActive(false), messageDisplayed(false), lastDebounceTime(0) {
    pinMode(buttonPin, INPUT_PULLUP);
    instance = this;
}

void ReactionTimeGame::handleButtonPressISR() {
    unsigned long currentTime = millis();
    if (instance && (currentTime - instance->lastDebounceTime) > debounceDelay) {
        int buttonStateReaction = digitalRead(instance->buttonPin);
        if (buttonStateReaction == LOW) { // Assuming active LOW buttons
            buttonPressed = true;
        }
        instance->lastDebounceTime = currentTime;
    }
}

void ReactionTimeGame::update(unsigned long millisNow) {
    esp_task_wdt_reset(); // Reset the watchdog during potentially long operations

    if (buttonPressed) {
        buttonPressed = false;
        handleButtonPress();
    }

    if (!gameStarted && !messageDisplayed) {
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        
        display.drawString(0, 0, "Press to start");
        display.display();
        messageDisplayed = true;
    }

    if (delayActive && millisNow >= randomDelayEnd) {
        esp_task_wdt_reset();

        display.clear();
        display.drawString(0, 0, "GO!");
        display.display();

        startTime = millisNow;
        waitingForReaction = true;
        delayActive = false;
    }
}

void ReactionTimeGame::handleButtonPress() {
    if (!gameStarted) {
        startGame(millis());
    } else if (waitingForReaction) {
        reactionTime = millis() - startTime;
        waitingForReaction = false;
        displayReactionTime();
    } else if (gameStarted && !waitingForReaction) {
        resetGame();
    } else {
        // Optional: Handle case where button press happens when it's not expected
        Serial.println("Unexpected button press.");
    }
}

void ReactionTimeGame::startGame(unsigned long millisNow) {
    gameStarted = true;
    display.clear();
    display.drawString(0, 0, "Get Ready...");
    display.display();
    
    unsigned long randomDelay = random(1000, 5000);
    randomDelayEnd = millisNow + randomDelay;
    delayActive = true;
    messageDisplayed = false;

    esp_task_wdt_reset();
}

void ReactionTimeGame::displayReactionTime() {
    display.clear();
    display.drawString(0, 0, "Time: " + String(reactionTime) + " ms");
    display.display();
}

void ReactionTimeGame::resetGame() {
    gameStarted = false;
    waitingForReaction = false;
    delayActive = false;
    messageDisplayed = false;
    
    display.clear();
    display.drawString(0, 0, "Press to start");
    display.display();
}

void ReactionTimeGame::enableISR() {
    attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPressISR, FALLING);
}

void ReactionTimeGame::disableISR() {
    detachInterrupt(digitalPinToInterrupt(buttonPin));
}