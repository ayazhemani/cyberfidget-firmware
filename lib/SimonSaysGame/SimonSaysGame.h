#ifndef SIMON_SAYS_GAME_H
#define SIMON_SAYS_GAME_H

#include <Arduino.h>

// Forward declaration if you're using a specific SSD1306 library:
class SSD1306Wire;

/**
 * A simple Simon Says game with 4 buttons:
 *   0 -> MiddleLeft  (top-left square)
 *   1 -> MiddleRight (top-right square)
 *   2 -> BottomLeft  (bottom-left square)
 *   3 -> BottomRight (bottom-right square)
 *
 * The game displays a 2x2 grid on a 128x64 screen and blinks the square 
 * corresponding to each pattern step.
 */
class SimonSaysGame {
public:
    // Maximum length of the pattern
    static const int MAX_PATTERN = 32;

    // Game States
    enum State {
        WAIT_START,    // idle, waiting for user to press a button to start
        SHOW_PATTERN,  // automatically blinking squares to show pattern
        WAIT_USER,     // waiting for user to press the correct sequence
        GAME_OVER      // user failed or completed (could reset after delay)
    };

    /**
     * @param display Reference to your SSD1306 display object
     */
    SimonSaysGame(SSD1306Wire &display);

    // Call once in setup
    void begin();

    // Call in loop; pass the button index pressed (0..3) or -1 if none
    void update(int pressedButton);

    // Reset the entire game state/pattern
    void resetGame();

private:
    // Reference to the OLED
    SSD1306Wire &display;

    // The pattern array (no dynamic memory)
    int pattern[MAX_PATTERN];

    // Current length of the pattern
    int patternLength;

    // Index of the step currently being shown or checked
    int patternIndex;

    // Current state
    State currentState;

    // Timing helpers for showing the pattern
    unsigned long showStartTime; 
    unsigned long showStepTime;  
    static const unsigned long SHOW_STEP_ON  = 500; // ms to fill the square
    static const unsigned long SHOW_STEP_OFF = 200; // ms gap after clearing
    bool showingSquare;  // true if currently “on” for the step
    bool gapPhase;       // true if we’re in the “off” phase between steps

    // For game-over auto-reset or delay
    unsigned long gameOverStartTime;

    // Private methods
    void startRound();
    void extendPattern();
    void showPattern();
    void checkUserInput(int pressedButton);
    void gameOver();

    // ---- Drawing Helpers ----
    // Draws the 2x2 grid. highlightIndex = which square to fill (0..3) or -1 for none
    void drawGrid(int highlightIndex);

    // Fills the correct region of the display for the given square
    void fillSquare(int sq);
};

#endif
