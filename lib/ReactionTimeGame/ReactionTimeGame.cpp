// ReactionTimeGame.cpp
#include "ReactionTimeGame.h"

// Initialize static instance pointer
ReactionTimeGame* ReactionTimeGame::instance = nullptr;

// Implement the callback with event type filtering
void ReactionTimeGame::reactionButtonPressedCallback(const ButtonEvent& ev) {
    Serial.println("ReactionTimeGame callback fired for button " + String(ev.buttonIndex) + " with event type " + String(ev.eventType));
    
    // Handle only the Pressed event
    if (ev.eventType == ButtonEvent_Pressed) {
        if (ReactionTimeGame::instance) {
            ReactionTimeGame::instance->handleButtonPress();
        }
    }
}

ReactionTimeGame::ReactionTimeGame(SSD1306Wire& disp, int btnIndex, ButtonManager& btnManager)
    : display(disp), buttonIndex(btnIndex), buttonManager(btnManager), 
      gameStarted(false), waitingForReaction(false), delayActive(false), messageDisplayed(false) {
    
    // Register the callback for the specific button
    // Do NOT register the callback here
    //buttonManager.registerCallback(buttonIndex, ReactionTimeGame::reactionButtonPressedCallback);
    
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
        Serial.println("Displaying 'Press to start...' screen.");
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);

        display.drawString(0, 0, "Press to start");
        display.display();
        messageDisplayed = true;
    }

    // Handle the delay logic
    if (delayActive && millisNow >= randomDelayEnd) {
        Serial.println("Displaying 'GO!' screen.");
        display.clear();
        display.drawString(0, 0, "GO!");
        display.display();

        startTime = millisNow;
        waitingForReaction = true;
        delayActive = false;
    }
}

void ReactionTimeGame::handleButtonPress() {
    Serial.println("handleButtonPress called.");
    if (!gameStarted) {
        Serial.println("Starting game...");
        startGame(millis());
    } else if (waitingForReaction) {
        Serial.println("Reaction time measured.");
        reactionTime = millis() - startTime;
        waitingForReaction = false;
        displayReactionTime();
    } else if (gameStarted && !waitingForReaction) {
        Serial.println("Resetting game...");
        resetGame();
    } else {
        Serial.println("Unexpected button press.");
    }
}

void ReactionTimeGame::startGame(unsigned long millisNow) {
    Serial.println("Starting Reaction Time Game.");
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
    Serial.println("Displaying reaction time.");
    display.clear();
    display.drawString(0, 0, "Time: " + String(reactionTime) + " ms");
    display.display();
}

void ReactionTimeGame::resetGame() {
    Serial.println("Resetting Reaction Time Game.");
    gameStarted = false;
    waitingForReaction = false;
    delayActive = false;
    messageDisplayed = false;

    display.clear();
    display.drawString(0, 0, "Press to start");
    display.display();
}
