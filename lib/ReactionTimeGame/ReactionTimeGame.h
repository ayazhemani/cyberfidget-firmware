#ifndef REACTION_TIME_GAME_H
#define REACTION_TIME_GAME_H

#include "SSD1306Wire.h"

class ReactionTimeGame {
public:
    ReactionTimeGame(SSD1306Wire& display, int buttonPin);

    void update(unsigned long millisNow);
    static void handleButtonPressISR();
    void enableISR();
    void disableISR();

private:
    static ReactionTimeGame* instance;
    SSD1306Wire& display;
    unsigned long startTime;
    unsigned long reactionTime;
    unsigned long randomDelayEnd;
    bool gameStarted;
    bool waitingForReaction;
    bool delayActive;
    bool messageDisplayed;
    int buttonPin;

    unsigned long lastDebounceTime;  // For debouncing
    static const unsigned long debounceDelay = 20;  // 50ms debounce delay

    void handleButtonPress();
    void startGame(unsigned long millisNow);
    void displayReactionTime();
    void resetGame();
};

#endif