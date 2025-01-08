#include "BreakoutGame.h"
#include <Arduino.h> // for random(), etc.

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
BreakoutGame::BreakoutGame(SSD1306Wire& disp)
    : display(disp)
{
    reset();
}

//----------------------------------------------------------------------------------------
// Reset game to initial state
//----------------------------------------------------------------------------------------
void BreakoutGame::reset() {
    // Paddle
    paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2.0f; // Start centered
    paddleSpeed = 1.5f; // Tweak to control how fast paddle moves with Ax

    // Ball
    ballX = SCREEN_WIDTH / 2.0f;
    ballY = SCREEN_HEIGHT / 2.0f;
    ballVX = 1.5f; // horizontal velocity
    ballVY = -1.5f; // vertical velocity

    // Bricks: reset all to active
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            bricks[r][c] = true;
        }
    }
}

//----------------------------------------------------------------------------------------
// Main update loop
//----------------------------------------------------------------------------------------
void BreakoutGame::update(float Ax) {
    // 1) Move the paddle based on Ax
    movePaddle(Ax);

    // 2) Move the ball
    moveBall();

    // 3) Check collisions (with walls, paddle, bricks)
    checkCollisions();

    // 4) Draw everything on the display
    drawGame();
}

//----------------------------------------------------------------------------------------
// Move paddle with accelerometer
//----------------------------------------------------------------------------------------
void BreakoutGame::movePaddle(float Ax) {
    // Example scale: paddleX += Ax * paddleSpeed
    // If Ax is small, you may want to amplify it. Tune as needed.
    // E.g., if Ax in [-1, 1], then it might be too small, so multiply further.

    paddleX += Ax * paddleSpeed * -0.10f; 
    // Keep paddle within screen
    if (paddleX < 0) {
        paddleX = 0;
    } else if (paddleX + PADDLE_WIDTH >= SCREEN_WIDTH) {
        paddleX = SCREEN_WIDTH - PADDLE_WIDTH;
    }
}

//----------------------------------------------------------------------------------------
// Move the ball
//----------------------------------------------------------------------------------------
void BreakoutGame::moveBall() {
    ballX += ballVX;
    ballY += ballVY;
}

//----------------------------------------------------------------------------------------
// Check for collisions with walls, paddle, bricks
//----------------------------------------------------------------------------------------
void BreakoutGame::checkCollisions() {
    // -- Wall collisions --
    // Left/right
    if (ballX <= 0) {
        ballX = 0;
        ballVX *= -1;
    } else if (ballX + BALL_SIZE >= SCREEN_WIDTH) {
        ballX = SCREEN_WIDTH - BALL_SIZE;
        ballVX *= -1;
    }
    // Top
    if (ballY <= 0) {
        ballY = 0;
        ballVY *= -1;
    }
    // Bottom (game over scenario?)
    if (ballY + BALL_SIZE >= SCREEN_HEIGHT) {
        // Let’s just bounce for now. You could treat it as “lose a life.”
        ballY = SCREEN_HEIGHT - BALL_SIZE;
        ballVY *= -1;
    }

    // -- Paddle collision --
    // If the ball is near the paddle’s row
    if ((ballY + BALL_SIZE) >= PADDLE_Y && ballY <= (PADDLE_Y + PADDLE_HEIGHT)) {
        // Check horizontal overlap
        if ((ballX + BALL_SIZE) >= paddleX && ballX <= (paddleX + PADDLE_WIDTH)) {
            // Bounce up
            ballY = PADDLE_Y - BALL_SIZE;
            ballVY *= -1;
        }
    }

    // -- Brick collisions --
    // If ball is within brick region (top rows)
    // Brick layout: 3 rows, 6 columns
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (!bricks[r][c]) continue; // Already destroyed

            // Each brick’s top-left
            int brickX = c * BRICK_WIDTH;
            int brickY = r * BRICK_HEIGHT;

            // If ball intersects the brick’s bounding box:
            if ((ballX + BALL_SIZE) >= brickX && 
                ballX <= (brickX + BRICK_WIDTH) &&
                (ballY + BALL_SIZE) >= brickY &&
                ballY <= (brickY + BRICK_HEIGHT))
            {
                // Destroy the brick
                bricks[r][c] = false;

                // Simple bounce: Flip vertical velocity
                // (Better collision detection can flip VX or VY depending on which
                // edge you hit.)
                ballVY *= -1;

                // Adjust ball position so it doesn’t “stick” in the brick
                // Move just outside the brick. This is a simplistic approach.
                if (ballVY > 0) {
                    ballY = brickY - BALL_SIZE;
                } else {
                    ballY = brickY + BRICK_HEIGHT;
                }
                return; // Only handle one brick collision per frame
            }
        }
    }
}

//----------------------------------------------------------------------------------------
// Draw the game objects on the display
//----------------------------------------------------------------------------------------
void BreakoutGame::drawGame() {
    display.clear();

    // 1) Draw the paddle
    for (int px = 0; px < PADDLE_WIDTH; px++) {
        for (int py = 0; py < PADDLE_HEIGHT; py++) {
            display.setPixel(static_cast<int>(paddleX) + px, PADDLE_Y + py);
        }
    }

    // 2) Draw the ball
    for (int bx = 0; bx < BALL_SIZE; bx++) {
        for (int by = 0; by < BALL_SIZE; by++) {
            display.setPixel(static_cast<int>(ballX) + bx, static_cast<int>(ballY) + by);
        }
    }

    // 3) Draw bricks
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (!bricks[r][c]) continue;
            int brickX = c * BRICK_WIDTH;
            int brickY = r * BRICK_HEIGHT;

            // Fill the rectangle for the brick
            for (int x = 0; x < BRICK_WIDTH; x++) {
                for (int y = 0; y < BRICK_HEIGHT; y++) {
                    display.setPixel(brickX + x, brickY + y);
                }
            }
        }
    }

    display.display();
}
