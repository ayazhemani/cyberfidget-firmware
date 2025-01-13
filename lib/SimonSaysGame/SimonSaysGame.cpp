#include "SimonSaysGame.h"
#include <SSD1306Wire.h> // or whatever your OLED library header is

SimonSaysGame::SimonSaysGame(SSD1306Wire &disp)
    : display(disp),
      patternLength(0),
      patternIndex(0),
      currentState(WAIT_START),
      showStartTime(0),
      showStepTime(0),
      showingSquare(false),
      gapPhase(false),
      gameOverStartTime(0)
{
    // Initialize pattern array
    for (int i = 0; i < MAX_PATTERN; i++) {
        pattern[i] = -1;
    }
}

void SimonSaysGame::begin() {
    // If you want truly random each reset, call randomSeed(...) in your setup
    resetGame();
}

void SimonSaysGame::update(int pressedButton) {
    switch (currentState) {
    case WAIT_START:
        // Pressing any button starts the game
        if (pressedButton != -1) {
            startRound();
        } else {
            // Just draw an idle 2x2 grid with no highlight
            drawGrid(-1);
        }
        break;

    case SHOW_PATTERN:
        showPattern();
        break;

    case WAIT_USER:
        if (pressedButton != -1) {
            checkUserInput(pressedButton);
        } else {
            // Keep showing the grid with no highlight
            drawGrid(-1);
        }
        break;

    case GAME_OVER:
        // Wait ~2s then reset
        if (millis() - gameOverStartTime > 2000) {
            resetGame();
        } else {
            // Possibly show something indicating game over
            // We'll just show an empty grid
            drawGrid(-1);
        }
        break;
    }
}

void SimonSaysGame::resetGame() {
    patternLength = 0;
    for (int i = 0; i < MAX_PATTERN; i++) {
        pattern[i] = -1;
    }
    currentState = WAIT_START;
}

void SimonSaysGame::startRound() {
    extendPattern();
    patternIndex   = 0;
    showingSquare  = false;
    gapPhase       = false;
    showStartTime  = millis();
    showStepTime   = showStartTime;
    currentState   = SHOW_PATTERN;
}

void SimonSaysGame::extendPattern() {
    if (patternLength < MAX_PATTERN) {
        pattern[patternLength] = random(0, 4); // 0..3
        patternLength++;
    }
}

// -----------------------------------------------------------------------------
// Show the pattern one step at a time:
// 1) Turn on the square for SHOW_STEP_ON ms
// 2) Turn it off for SHOW_STEP_OFF ms
// 3) Move to the next pattern step
// -----------------------------------------------------------------------------
void SimonSaysGame::showPattern() {
    unsigned long now = millis();

    // Are we done showing all steps?
    if (patternIndex >= patternLength) {
        // Move on to user input
        currentState = WAIT_USER;
        patternIndex = 0;
        drawGrid(-1);
        return;
    }

    if (!showingSquare && !gapPhase) {
        // Turn on the square for the current step
        drawGrid(pattern[patternIndex]);
        showingSquare = true;
        showStepTime = now;
    } else if (showingSquare && (now - showStepTime > SHOW_STEP_ON)) {
        // Time to turn it off
        drawGrid(-1);
        showingSquare = false;
        gapPhase = true;
        showStepTime = now; // start gap timer
    } else if (gapPhase && (now - showStepTime > SHOW_STEP_OFF)) {
        // Gap ended, move to next step
        gapPhase = false;
        patternIndex++;
    }
}

// -----------------------------------------------------------------------------
// Once in WAIT_USER, check pressedButton vs. pattern
// -----------------------------------------------------------------------------
void SimonSaysGame::checkUserInput(int pressedButton) {
    if (pressedButton == pattern[patternIndex]) {
        // Correct
        patternIndex++;
        if (patternIndex >= patternLength) {
            // Completed this round: start next round
            startRound();
        } else {
            // Optionally, you can briefly highlight the user’s press:
            drawGrid(pressedButton);
            delay(100);
            drawGrid(-1);
        }
    } else {
        // Wrong -> game over
        gameOver();
    }
}

// -----------------------------------------------------------------------------
// Game Over
// -----------------------------------------------------------------------------
void SimonSaysGame::gameOver() {
    currentState = GAME_OVER;
    gameOverStartTime = millis();

    // Optionally highlight the wrong button or beep
    // We'll just show a blank grid right now:
    drawGrid(-1);
}

// -----------------------------------------------------------------------------
// Render a 2x2 grid. highlightIndex is which square to fill (-1 for none).
// -----------------------------------------------------------------------------
void SimonSaysGame::drawGrid(int highlightIndex) {
    display.clear();

    // If highlightIndex != -1, fill that region, else just outline, or do nothing
    // Let’s fill it with a rectangle for a strong “blink”.

    // We can outline the 2x2 squares first:
    // top-left square boundary lines
    for (int x = 0; x < 128; x++) {
        // horizontal lines
        if ( (x >= 0 && x < 128) ) {
            display.setPixel(x, 31);  // horizontal line between top and bottom
        }
    }
    for (int y = 0; y < 64; y++) {
        // vertical lines
        if ( (y >= 0 && y < 64) ) {
            display.setPixel(63, y);  // vertical line between left and right
        }
    }

    // If highlightIndex != -1, fill that square
    if (highlightIndex != -1) {
        fillSquare(highlightIndex);
    }

    display.display();
}

// -----------------------------------------------------------------------------
// Fill the rectangle corresponding to the given button (0..3).
// -----------------------------------------------------------------------------
void SimonSaysGame::fillSquare(int sq) {
    // define bounding boxes for each button ID
    // 0 (top-left)     : x from 0..63,   y from 0..31
    // 1 (top-right)    : x from 64..127, y from 0..31
    // 2 (bottom-left)  : x from 0..63,   y from 32..63
    // 3 (bottom-right) : x from 64..127, y from 32..63

    int xStart = 0, xEnd = 63;
    int yStart = 0, yEnd = 31;

    switch (sq) {
        case 0: // top-left
            xStart = 0;    xEnd = 63;
            yStart = 0;    yEnd = 31;
            break;
        case 1: // top-right
            xStart = 64;   xEnd = 127;
            yStart = 0;    yEnd = 31;
            break;
        case 2: // bottom-left
            xStart = 0;    xEnd = 63;
            yStart = 32;   yEnd = 63;
            break;
        case 3: // bottom-right
            xStart = 64;   xEnd = 127;
            yStart = 32;   yEnd = 63;
            break;
        default:
            return; // do nothing
    }

    // Fill the region
    for (int x = xStart; x <= xEnd; x++) {
        for (int y = yStart; y <= yEnd; y++) {
            display.setPixel(x, y);
        }
    }
}