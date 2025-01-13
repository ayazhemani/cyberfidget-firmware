#include "BreakoutGame.h"
#include <Arduino.h> // for pinMode, millis, digitalRead, etc.

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
BreakoutGame::BreakoutGame(SSD1306Wire& disp)
    : display(disp), bounceCallback(nullptr)
{
    resetButtonPin       = -1;
    resetButtonActiveLow = true;
    lastButtonCheckTime  = 0;
    lastButtonState      = false;

    reset();
}

//----------------------------------------------------------------------------------------
// Reset game to initial state
//----------------------------------------------------------------------------------------
void BreakoutGame::reset() {
    // Paddle
    paddleX     = (SCREEN_WIDTH - PADDLE_WIDTH) / 2.0f; 
    paddleSpeed = 2.0f;  

    // Ball
    ballX  = SCREEN_WIDTH / 2.0f;
    ballY  = SCREEN_HEIGHT / 2.0f;
    ballVX = 0.8f;
    ballVY = -0.8f;

    // Bricks: reset all to active
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            bricks[r][c] = true;
        }
    }

    // Game state
    deathCount = 0;
    gameWon    = false;

    // **Start timing now**
    startTime  = millis();
    totalTime  = 0;
}

//----------------------------------------------------------------------------------------
// Set the reset button pin and active logic
//----------------------------------------------------------------------------------------
void BreakoutGame::setResetButton(int buttonPin, bool activeLow) {
    resetButtonPin       = buttonPin;
    resetButtonActiveLow = activeLow;
    initResetButton();
}

//----------------------------------------------------------------------------------------
// Configure the reset button pinMode
//----------------------------------------------------------------------------------------
void BreakoutGame::initResetButton() {
    if (resetButtonPin >= 0) {
        // If it's active low, use INPUT_PULLUP
        // If it's active high, use INPUT
        if (resetButtonActiveLow) {
            pinMode(resetButtonPin, INPUT_PULLUP);
        } else {
            pinMode(resetButtonPin, INPUT);
        }

        bool currentVal = digitalRead(resetButtonPin);
        lastButtonState = currentVal;
    }
}

//----------------------------------------------------------------------------------------
// Checks if the button is pressed, performs software debounce, and resets the game
//----------------------------------------------------------------------------------------
void BreakoutGame::checkResetButton() {
    if (resetButtonPin < 0) return;

    unsigned long now = millis();
    if (now - lastButtonCheckTime < DEBOUNCE_MS) return;

    bool rawVal  = digitalRead(resetButtonPin);
    bool pressed = (resetButtonActiveLow) ? (rawVal == LOW) : (rawVal == HIGH);

    if (pressed && !lastButtonState) {
        // Button was just pressed
        reset(); 
    }

    lastButtonState = rawVal;
    lastButtonCheckTime = now;
}

//----------------------------------------------------------------------------------------
// Main update loop
//----------------------------------------------------------------------------------------
void BreakoutGame::update(float Ax) {
    // 1) Check if the reset button is pressed
    checkResetButton();

    // 2) If the game is already won, just draw the "YOU WIN" screen
    if (gameWon) {
        drawGame();
        return;
    }

    // 3) Move the paddle based on Ax
    movePaddle(Ax);

    // 4) Move the ball
    moveBall();

    // 5) Check collisions (with walls, paddle, bricks)
    checkCollisions();

    // 6) Check if all bricks are destroyed
    checkVictory();

    // 7) Draw everything on the display
    drawGame();
}

//----------------------------------------------------------------------------------------
// Move paddle with accelerometer
//----------------------------------------------------------------------------------------
void BreakoutGame::movePaddle(float Ax) {
    // Tweak the multiplier or sign as needed
    paddleX += Ax * paddleSpeed * -0.01f; 

    // Keep paddle within screen bounds
    if (paddleX < 0) {
        paddleX = 0;
    } else if (paddleX + PADDLE_WIDTH > SCREEN_WIDTH) {
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
// Set a user-provided callback for paddle bounces
//----------------------------------------------------------------------------------------
void BreakoutGame::setBounceCallback(BounceCallback cb) {
    bounceCallback = cb;
}

//----------------------------------------------------------------------------------------
// Check for collisions with walls, paddle, bricks
//----------------------------------------------------------------------------------------
void BreakoutGame::checkCollisions() {
    // Left/right walls
    if (ballX <= 0) {
        ballX = 0;
        ballVX *= -1;
    } else if (ballX + BALL_SIZE >= SCREEN_WIDTH) {
        ballX = SCREEN_WIDTH - BALL_SIZE;
        ballVX *= -1;
    }

    // Top wall
    if (ballY <= 0) {
        ballY = 0;
        ballVY *= -1;
    }

    // Bottom wall -> "death"
    if (ballY + BALL_SIZE >= SCREEN_HEIGHT) {
        deathCount++;
        ballX  = SCREEN_WIDTH / 2.0f;
        ballY  = SCREEN_HEIGHT / 2.0f;
        ballVX = (random(2) ? 1.2f : -1.2f);
        ballVY = -1.2f;
        return;
    }

    // Paddle
    if ((ballY + BALL_SIZE) >= PADDLE_Y && ballY <= (PADDLE_Y + PADDLE_HEIGHT)) {
        if ((ballX + BALL_SIZE) >= paddleX && ballX <= (paddleX + PADDLE_WIDTH)) {
            ballY = PADDLE_Y - BALL_SIZE;
            ballVY *= -1;

            // **Play bounce sound** if callback is provided
            if (bounceCallback) {
                bounceCallback();
            }
        }
    }

    // Bricks
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (!bricks[r][c]) continue;

            int brickX = c * BRICK_WIDTH;
            int brickY = r * BRICK_HEIGHT;
            if ((ballX + BALL_SIZE) >= brickX &&
                ballX <= (brickX + BRICK_WIDTH) &&
                (ballY + BALL_SIZE) >= brickY &&
                ballY <= (brickY + BRICK_HEIGHT))
            {
                bricks[r][c] = false;
                ballVY *= -1;
                if (ballVY > 0) {
                    ballY = brickY - BALL_SIZE;
                } else {
                    ballY = brickY + BRICK_HEIGHT;
                }
                return; 
            }
        }
    }
}

//----------------------------------------------------------------------------------------
// Check if all bricks are gone
//----------------------------------------------------------------------------------------
void BreakoutGame::checkVictory() {
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c]) {
                // Found an active brick
                return; 
            }
        }
    }
    // No bricks found: user wins!
    gameWon    = true;
    totalTime  = millis() - startTime; // how long it took, in ms
}

//----------------------------------------------------------------------------------------
// Draw everything on the display
//----------------------------------------------------------------------------------------
void BreakoutGame::drawGame() {
    display.clear();
    display.setFont(ArialMT_Plain_10);

    // If the user has won, display a "YOU WIN" message, death count, and time
    if (gameWon) {
        display.drawString(32, 24, "YOU WIN!");

        String deathMsg = "Deaths: ";
        deathMsg += deathCount;
        display.drawString(32, 34, deathMsg);

        // Show how many seconds it took
        float seconds = totalTime / 1000.0f;
        String timeMsg = "Time: ";
        timeMsg += String(seconds, 2); // 2 decimal places
        timeMsg += "s";
        display.drawString(32, 44, timeMsg);

        display.drawString(8, 54, "Press Btn to Reset");
        display.display();
        return;
    }

    // Draw the paddle
    for (int px = 0; px < PADDLE_WIDTH; px++) {
        for (int py = 0; py < PADDLE_HEIGHT; py++) {
            display.setPixel((int)paddleX + px, PADDLE_Y + py);
        }
    }

    // Draw the ball
    for (int bx = 0; bx < BALL_SIZE; bx++) {
        for (int by = 0; by < BALL_SIZE; by++) {
            display.setPixel((int)ballX + bx, (int)ballY + by);
        }
    }

    // Draw bricks
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (!bricks[r][c]) continue;
            int brickX = c * BRICK_WIDTH;
            int brickY = r * BRICK_HEIGHT;
            for (int x = 0; x < BRICK_WIDTH; x++) {
                for (int y = 0; y < BRICK_HEIGHT; y++) {
                    display.setPixel(brickX + x, brickY + y);
                }
            }
        }
    }

    // Show current death count
    String deathMsg = "Deaths: ";
    deathMsg += deathCount;
    display.drawString(0, 0, deathMsg);

    display.display();
}