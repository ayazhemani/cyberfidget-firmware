#ifndef STRATAGEM_GAME_H
#define STRATAGEM_GAME_H

#include <Arduino.h>
#include <DisplayProxy.h>
#include "ButtonManager.h"
#include "AudioManager.h"
#include "MenuManager.h"

/**
 * @brief Stratagem struct to hold the name, the arrow-sequence, and an image reference.
 * 
 * You can add more fields (e.g., audio filenames) if necessary.
 */
struct Stratagem {
    const char* name;
    const char* sequence; // e.g. "UDRLU"
    const char* image;    // e.g. "reinforce.svg" or "placeholder"
};

/**
 * @brief The StratagemGame class replicates the logic of the JavaScript Stratagem Hero
 * game, adapted for this Cyber Fidget environment.
 */
class StratagemGame {
public:
    /**
     * @brief States for the game
     */
    enum State {
        WAIT_START,
        RUNNING,
        HITLAG,
        GAME_OVER
    };

    /**
     * @brief Constructor
     */
    StratagemGame(ButtonManager &buttonMgr, AudioManager &audioMgr);

    /**
     * @brief Initialize the game and register callbacks
     */
    void begin();

    /**
     * @brief Main loop update
     */
    void update();

    /**
     * @brief Cleanup / end the game
     */
    void end();

private:
    // References
    DisplayProxy &display;
    ButtonManager &buttonManager;
    AudioManager &audioManager;

    // Singleton instance pointer for static callbacks
    static StratagemGame* instance;

    // Game state
    State currentState;

    // Score
    int score;

    // Time tracking
    static const unsigned long TOTAL_TIME      = 10000UL; // in ms
    static const unsigned long HITLAG_TIMEOUT  = 200UL;   // in ms (time after finishing a stratagem)
    static const unsigned long TIME_BONUS      = 500UL;   // in ms each time a sequence is completed
    unsigned long timeRemaining;              // e.g. in ms
    unsigned long lastUpdateTime;             // for delta T
    unsigned long hitlagStartTime;            // when we entered HITLAG

    // The chosen “active” stratagem 
    // (or the first one in a short list if you want multiple at once)
    // For simplicity, we’ll just keep one active at a time.
    Stratagem currentStratagem;

    // Sequence progress
    // E.g. if the sequence is “UDRLU”,
    // we store how many correct steps have been entered so far.
    int currentSequenceIndex;

    // The random list of all possible stratagems is compiled in. 
    // If you want to pick a new one each time, you can do that via random().
    static const int MAX_STRATAGEMS_ONSCREEN = 1; // if you want multiple, adjust logic
    static const int NUM_STRATAGEMS;             // declared below

    // Callback for direction button events
    static void onButtonEvent(const ButtonEvent& event);

    // Callback for the “back” button
    static void onButtonBackPressed(const ButtonEvent& event);

    // Helpers
    void pickRandomStratagem();
    void resetGame();
    void checkInput(char direction);
    void gameOver();
    void handleHitlag();
    void drawScreen();
    void drawGameOver();
    void drawRunning();
    void playDirectionSound(char direction);
};

extern StratagemGame stratagemGame;

#endif // STRATAGEM_GAME_H
