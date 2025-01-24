// ReactionTimeGame.cpp
#include "ReactionTimeGame.h"

// Initialize static instance pointer
ReactionTimeGame* ReactionTimeGame::instance = nullptr;

// Implement the callback
void ReactionTimeGame::reactionButtonPressedCallback(const ButtonEvent& ev) {
    // The button has been pressed; delegate to the instance's handler
    if (ReactionTimeGame::instance) {
        ReactionTimeGame::instance->handleButtonPress();
    }
}

ReactionTimeGame::ReactionTimeGame(SSD1306Wire& disp, int btnIndex, ButtonManager& btnManager)
    : display(disp), buttonIndex(btnIndex), buttonManager(btnManager), 
      gameStarted(false), waitingForReaction(false), delayActive(false), messageDisplayed(false) {
    
    // Register the callback for the specific button
    buttonManager.registerCallback(buttonIndex, ReactionTimeGame::reactionButtonPressedCallback);
    
    // Store the instance pointer
    ReactionTimeGame::instance = this;
}

void ReactionTimeGame::update(unsigned long millisNow) {
    // Only proceed if the game is active (callback is registered)
    if (!buttonManager.hasCallback(buttonIndex)) {
        return; // Skip update if the game isn't active
    }

    // Initial screen prompt
    if (!gameStarted && !delayActive && !waitingForReaction && !messageDisplayed) {
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);

        display.drawString(0, 0, "Press to start");
        display.display();
        messageDisplayed = true;
    }

    // Handle the delay logic
    if (delayActive && millisNow >= randomDelayEnd) {
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
        Serial.println("Unexpected button press.");
    }
}

void ReactionTimeGame::startGame(unsigned long millisNow) {
    gameStarted = true;
    display.clear();
    display.drawString(0, 0, "Get Ready...");
    display.display();

    unsigned long randomDelay = random(1000, 5000); // Delay between 1s and 5s
    randomDelayEnd = millisNow + randomDelay;
    delayActive = true;
    messageDisplayed = false;
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
