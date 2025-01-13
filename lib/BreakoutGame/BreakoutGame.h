#ifndef BREAKOUT_GAME_H
#define BREAKOUT_GAME_H

#include "SSD1306Wire.h"
#include <functional> // for std::function

class BreakoutGame {
public:
    // The callback type: no parameters, no return, called on paddle bounce.
    using BounceCallback = std::function<void()>;
    
    BreakoutGame(SSD1306Wire& display);

    // If you want to provide your bounce sound callback after construction:
    void setBounceCallback(BounceCallback cb);

    // Update game logic. Ax is the horizontal acceleration from the IMU.
    void update(float Ax);

    // Reset the game (ball, paddle, bricks, death count).
    void reset();

    /**
     * @brief Specify a button to monitor for game resets.
     * @param buttonPin  The GPIO pin to read.
     * @param activeLow  True if pressing the button pulls the pin LOW.
     */
    void setResetButton(int buttonPin, bool activeLow);

private:
    // -- Reference to the OLED display --
    SSD1306Wire& display;

    // -- Screen dimensions --
    static constexpr int SCREEN_WIDTH  = 128;
    static constexpr int SCREEN_HEIGHT = 64;

    // -- Paddle properties --
    float paddleX;                     // Left edge of the paddle
    static constexpr int PADDLE_Y      = SCREEN_HEIGHT - 8; // near bottom
    static constexpr int PADDLE_WIDTH  = 20;
    static constexpr int PADDLE_HEIGHT = 3;
    float paddleSpeed; // multiplier for how fast we move when Ax changes

    // -- Ball properties --
    float ballX, ballY;    // Ball position
    float ballVX, ballVY;  // Ball velocity
    static constexpr int BALL_SIZE = 2; // 2x2 pixel ball

    // -- Bricks --
    static constexpr int BRICK_ROWS   = 3;
    static constexpr int BRICK_COLS   = 6;
    static constexpr int BRICK_WIDTH  = 16; 
    static constexpr int BRICK_HEIGHT = 5;
    bool bricks[BRICK_ROWS][BRICK_COLS]; // true if brick is active

    // -- Game state --
    int  deathCount;   // How many times the ball hits the bottom
    bool gameWon;      // True if all bricks are destroyed

    // **Timer for how long it takes to destroy all bricks**
    unsigned long startTime; // When the game started/restarted
    unsigned long totalTime; // Final time it took (in ms) once game is won

    // -- Reset Button Info --
    int  resetButtonPin;      
    bool resetButtonActiveLow; 
    unsigned long lastButtonCheckTime;  
    bool lastButtonState;
    static constexpr unsigned long DEBOUNCE_MS = 150;

    // The function we call when the ball bounces off the paddle.
    // Default = nullptr (no sound).
    BounceCallback bounceCallback; 

    // Private methods
    void initResetButton();  
    void checkResetButton(); 
    void movePaddle(float Ax);
    void moveBall();
    void checkCollisions();
    void checkVictory();  
    void drawGame();
};

#endif