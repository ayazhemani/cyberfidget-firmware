// ReactionTimeGame.h
#ifndef REACTION_TIME_GAME_H
#define REACTION_TIME_GAME_H

#include "SSD1306Wire.h"
#include "ButtonManager.h"

/**
 * @brief Reaction Time Game Class
 */
class ReactionTimeGame {
public:
    /**
     * @brief Constructor for ReactionTimeGame
     * 
     * @param display Reference to the SSD1306 display
     * @param buttonIndex Index of the button assigned to this game
     * @param buttonManager Reference to the ButtonManager instance
     */
    ReactionTimeGame(SSD1306Wire& display, int buttonIndex, ButtonManager& buttonManager);

    /**
     * @brief Update method to be called periodically
     * 
     * @param millisNow Current time in milliseconds
     */
    void update(unsigned long millisNow);

    /**
     * @brief Static callback function to handle button presses
     * 
     * @param ev ButtonEvent struct containing event details
     */
    static void reactionButtonPressedCallback(const ButtonEvent& ev);

    static ReactionTimeGame* instance;

    /**
     * @brief Getter for buttonIndex
     * 
     * @return int Index of the button used for this game
     */
    int getButtonIndex() const { return buttonIndex; }

    /**
     * @brief Reset the game to its initial state
     */
    void resetGame();

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

    /**
     * @brief Handle a button press event
     */
    void handleButtonPress();

    /**
     * @brief Start the reaction time game
     * 
     * @param millisNow Current time in milliseconds
     */
    void startGame(unsigned long millisNow);

    /**
     * @brief Display the reaction time to the user
     */
    void displayReactionTime();
};

#endif
