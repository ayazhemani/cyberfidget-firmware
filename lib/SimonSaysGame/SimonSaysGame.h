#ifndef SIMON_SAYS_GAME_H
#define SIMON_SAYS_GAME_H

#include <Arduino.h>

class SSD1306Wire;

/**
 * Simon Says with:
 *  - 4 buttons (0..3) mapped to a 2×2 grid
 *  - Score display when user loses (how many patterns were completed)
 *  - No dynamic memory
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
     * @param display Reference to SSD1306 display
     * @param beepForSquareFn Called when the game shows a pattern step
     * @param beepOnUserPressFn Called when user presses a correct button
     */
    SimonSaysGame(
        SSD1306Wire &display,
        void (*beepForSquareFn)(int),
        void (*beepOnUserPressFn)(int)
    );

    // Initialize the game (call in setup())
    void begin();

    // Update the game logic (call in loop()). 
    //  pressedButton = 0..3 if pressed, or -1 if no button pressed.
    void update(int pressedButton);

    // Resets the entire game
    void resetGame();

private:
    // Reference to the OLED
    SSD1306Wire &display;

    // Audio callbacks (function pointers)
    void (*beepForSquare)(int);
    void (*beepOnUserPress)(int);

    // Pattern data
    int  pattern[MAX_PATTERN];
    int  patternLength;  // how many steps in the current pattern
    int  patternIndex;   // which step we’re currently showing or checking

    // Keep track of how many full patterns the user has matched
    int  score;

    // Game flow state
    State currentState;

    // SHOW_PATTERN timing
    unsigned long showStartTime; 
    unsigned long showStepTime;  
    static const unsigned long SHOW_STEP_ON  = 500; 
    static const unsigned long SHOW_STEP_OFF = 200; 
    bool showingSquare;  
    bool gapPhase;       

    // GAME_OVER timing
    unsigned long gameOverStartTime;

    // Private methods
    void startRound();
    void extendPattern();
    void showPattern();
    void checkUserInput(int pressedButton);
    void gameOver();

    // Helpers to draw the 2×2 grid
    void drawGrid(int highlightIndex);
    void fillSquare(int sq);

    // Display a “Game Over” message and the final score
    void drawGameOver();
};

#endif
