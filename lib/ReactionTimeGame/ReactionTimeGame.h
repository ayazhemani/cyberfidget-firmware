#ifndef REACTION_TIME_GAME_H
#define REACTION_TIME_GAME_H

#include "SSD1306Wire.h"
#include "ButtonManager.h"

class ReactionTimeGame {
public:
    ReactionTimeGame(SSD1306Wire& display, int buttonIndex, ButtonManager& buttonManager);

    void update(unsigned long millisNow);

    static void reactionButtonPressedCallback(const ButtonEvent& ev);

    static ReactionTimeGame* instance;

private:
    SSD1306Wire& display;
    ButtonManager& buttonManager;
    int buttonIndex; // Index of the button used for this game

    unsigned long startTime;
    unsigned long reactionTime;
    unsigned long randomDelayEnd;
    bool gameStarted;
    bool waitingForReaction;
    bool delayActive;
    bool messageDisplayed;

    void handleButtonPress();
    void startGame(unsigned long millisNow);
    void displayReactionTime();
    void resetGame();
};

#endif
