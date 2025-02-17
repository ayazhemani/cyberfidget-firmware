#include "SimonSaysGame.h"
#include <SSD1306Wire.h>  // or your specific OLED library

SimonSaysGame::SimonSaysGame(
    SSD1306Wire &disp,
    void (*beepForSq)(int),
    void (*beepOnUserPressSq)(int)
)
    : display(disp),
      beepForSquare(beepForSq),
      beepOnUserPress(beepOnUserPressSq),
      patternLength(0),
      patternIndex(0),
      score(0),
      currentState(WAIT_START),
      showStartTime(0),
      showStepTime(0),
      showingSquare(false),
      gapPhase(false),
      gameOverStartTime(0)
{
    for (int i = 0; i < MAX_PATTERN; i++) {
        pattern[i] = -1;
    }
}

void SimonSaysGame::begin() {
    resetGame();
}

void SimonSaysGame::update(int pressedButton) {
    switch (currentState) {
    case WAIT_START:
        // Press any button to start
        if (pressedButton != -1) {
            startRound();
        } else {
            drawGrid(-1); // idle grid
        }
        break;

    case SHOW_PATTERN:
        showPattern();
        break;

    case WAIT_USER:
        if (pressedButton != -1) {
            checkUserInput(pressedButton);
        } else {
            drawGrid(-1); // no highlight
        }
        break;

    case GAME_OVER:
        // Display "Game Over" with final score for 2s, then reset
        if (millis() - gameOverStartTime > 2000) {
            resetGame();
        } else {
            drawGameOver();
        }
        break;
    }
}

void SimonSaysGame::resetGame() {
    patternLength = 0;
    patternIndex  = 0;
    score         = 0;
    currentState  = WAIT_START;

    for (int i = 0; i < MAX_PATTERN; i++) {
        pattern[i] = -1;
    }
}

void SimonSaysGame::startRound() {
    extendPattern();
    patternIndex  = 0;
    showingSquare = false;
    gapPhase      = false;
    showStartTime = millis();
    showStepTime  = showStartTime;
    currentState  = SHOW_PATTERN;
}

void SimonSaysGame::extendPattern() {
    if (patternLength < MAX_PATTERN) {
        pattern[patternLength] = random(0, 4); // 0..3
        patternLength++;
    }
    // Otherwise the user might have “won” everything
}

void SimonSaysGame::showPattern() {
    unsigned long now = millis();

    // Done showing? Move to WAIT_USER
    if (patternIndex >= patternLength) {
        currentState = WAIT_USER;
        patternIndex = 0;
        drawGrid(-1);
        return;
    }

    if (!showingSquare && !gapPhase) {
        // Turn on the current square
        int sq = pattern[patternIndex];
        drawGrid(sq);
        showingSquare = true;
        showStepTime  = now;

        // Call beep callback for pattern step
        if (beepForSquare) {
            beepForSquare(sq);
        }
    }
    else if (showingSquare && (now - showStepTime > SHOW_STEP_ON)) {
        // Turn off
        drawGrid(-1);
        showingSquare = false;
        gapPhase      = true;
        showStepTime  = now;
    }
    else if (gapPhase && (now - showStepTime > SHOW_STEP_OFF)) {
        // Move to next step
        gapPhase = false;
        patternIndex++;
    }
}

void SimonSaysGame::checkUserInput(int pressedButton) {
    // Check if correct button
    if (pressedButton == pattern[patternIndex]) {
        patternIndex++;

        // (Optional) beep for correct user press
        if (beepOnUserPress) {
            beepOnUserPress(pressedButton);
        }

        // If the user matched the full pattern
        if (patternIndex >= patternLength) {
            // They completed this entire round
            score = patternLength; 
            // Start next round
            startRound();
        }
        else {
            // Optionally flash their press
            drawGrid(pressedButton);
            delay(100);
            drawGrid(-1);
        }
    } else {
        // Wrong press => game over
        gameOver();
    }
}

void SimonSaysGame::gameOver() {
    currentState = GAME_OVER;
    gameOverStartTime = millis();
}

// -----------------------------------------------------------------------------
// Draw "Game Over" with final score
// -----------------------------------------------------------------------------
void SimonSaysGame::drawGameOver() {
    display.clear();

    // You can use display.drawString(...) calls if your library supports text.
    // For example, with SSD1306Wire, you might do:
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10); // or your chosen font
    display.drawString(30, 20, "GAME OVER");

    // Show the final 'score'
    String msg = "Score: ";
    msg += score;
    display.drawString(30, 35, msg);

    display.display();
}

// -----------------------------------------------------------------------------
// Draw a 2×2 grid, highlightIndex = which square to fill (-1 if none)
// -----------------------------------------------------------------------------
void SimonSaysGame::drawGrid(int highlightIndex) {
    display.clear();

    // Draw horizontal line at y=31, vertical line at x=63
    for (int x = 0; x < 128; x++) {
        display.setPixel(x, 31);
    }
    for (int y = 0; y < 64; y++) {
        display.setPixel(63, y);
    }

    // Fill highlight if requested
    if (highlightIndex != -1) {
        fillSquare(highlightIndex);
    }

    display.display();
}

void SimonSaysGame::fillSquare(int sq) {
    // 0: top-left, 1: top-right, 2: bottom-left, 3: bottom-right
    int xStart = 0, xEnd = 63;
    int yStart = 0, yEnd = 31;

    switch (sq) {
        case 0:
            xStart = 0;   xEnd = 63;
            yStart = 0;   yEnd = 31;
            break;
        case 1:
            xStart = 64;  xEnd = 127;
            yStart = 0;   yEnd = 31;
            break;
        case 2:
            xStart = 0;   xEnd = 63;
            yStart = 32;  yEnd = 63;
            break;
        case 3:
            xStart = 64;  xEnd = 127;
            yStart = 32;  yEnd = 63;
            break;
        default:
            return;
    }

    for (int x = xStart; x <= xEnd; x++) {
        for (int y = yStart; y <= yEnd; y++) {
            display.setPixel(x, y);
        }
    }
}
