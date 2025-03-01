#ifndef SIMON_SAYS_GAME_H
#define SIMON_SAYS_GAME_H

#include <Arduino.h>
#include <SSD1306Wire.h>     // Include your OLED library
#include "ButtonManager.h"   // Include the ButtonManager

/**
 * @brief Simon Says Game Class
 * 
 * Manages the Simon Says game logic, including displaying patterns,
 * handling user input, and updating the game state. Integrates with the
 * ButtonManager to handle button events efficiently.
 */
class SimonSaysGame {
public:
    static const int MAX_PATTERN = 32;

    enum State {
        WAIT_START,
        SHOW_PATTERN,
        WAIT_USER,
        GAME_OVER
    };

    /**
     * @brief Constructor for SimonSaysGame
     * 
     * @param display Reference to the OLED display
     * @param buttonMgr Reference to the ButtonManager
     * @param beepForSquareFn Callback for beeping when showing a pattern square
     * @param beepOnUserPressFn Callback for beeping when the user presses a button
     */
    SimonSaysGame(
        SSD1306Wire &display,
        ButtonManager &buttonMgr,
        void (*beepForSquareFn)(int),
        void (*beepOnUserPressFn)(int)
    );

    /**
     * @brief Initialize the game and register button callbacks
     */
    void begin();

    /**
     * @brief Update the game logic (call in loop())
     */
    void update();

    /**
     * @brief Cleanup and unregister button callbacks
     */
    void end();

    /**
     * @brief Handle button events from the ButtonManager
     * 
     * @param event The button event to process
     */
    static void onButtonEvent(const ButtonEvent& event);

private:
    // Reference to the OLED display
    SSD1306Wire &display;

    // Reference to the ButtonManager
    ButtonManager &buttonManager;

    // Audio callbacks
    void (*beepForSquare)(int);
    void (*beepOnUserPress)(int);

    // Pattern data
    int  pattern[MAX_PATTERN];
    int  patternLength;
    int  patternIndex;
    int  score;

    // Game state
    State currentState;
    int currentlyPressedButton;

    // Timing variables for pattern display
    unsigned long showStartTime; 
    unsigned long showStepTime;  
    static const unsigned long SHOW_STEP_ON  = 500; 
    static const unsigned long SHOW_STEP_OFF = 200; 
    bool showingSquare;  
    bool gapPhase;       

    // Timing for game over display
    unsigned long gameOverStartTime;

    // Static instance pointer for callbacks
    static SimonSaysGame* instance;

    // Button indices for the Simon Says game
    static const int buttonMappings[4];

    // Private methods
    void startRound();
    void extendPattern();
    void showPattern();
    void checkUserInput(int pressedButton);
    void gameOver();
    void resetGame();

    // Display helpers
    void drawGrid(int highlightIndex);
    void fillSquare(int sq);
    void drawGameOver();
};

#endif