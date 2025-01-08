#ifndef BREAKOUT_GAME_H
#define BREAKOUT_GAME_H

#include "SSD1306Wire.h"

class BreakoutGame {
public:
    BreakoutGame(SSD1306Wire& display);

    // Update game logic. Ax is the horizontal acceleration from the IMU.
    void update(float Ax);

    // Reset the game (ball, paddle, bricks).
    void reset();

private:
    // -- Reference to the OLED display --
    SSD1306Wire& display;

    // -- Screen dimensions --
    static constexpr int SCREEN_WIDTH  = 128;
    static constexpr int SCREEN_HEIGHT = 64;

    // -- Paddle properties --
    float paddleX;                 // Left edge of the paddle
    static constexpr int PADDLE_Y = SCREEN_HEIGHT - 8; // Keep paddle near bottom
    static constexpr int PADDLE_WIDTH = 20;
    static constexpr int PADDLE_HEIGHT = 3;
    float paddleSpeed; // multiplier for how fast we move when Ax changes

    // -- Ball properties --
    float ballX, ballY;    // Ball position
    float ballVX, ballVY;  // Ball velocity
    static constexpr int BALL_SIZE = 2; // 2x2 pixel ball

    // -- Bricks --
    static constexpr int BRICK_ROWS = 3;
    static constexpr int BRICK_COLS = 6;
    static constexpr int BRICK_WIDTH = 16;   // Width in pixels
    static constexpr int BRICK_HEIGHT = 5;   // Height in pixels
    bool bricks[BRICK_ROWS][BRICK_COLS];     // Brick active/inactive

    // Helpers
    void movePaddle(float Ax);
    void moveBall();
    void checkCollisions();
    void drawGame();
};

#endif
