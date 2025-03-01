#include "SimonSaysGame.h"
#include "globals.h"  // Include this to use the global button indices

// Initialize static instance pointer
SimonSaysGame* SimonSaysGame::instance = nullptr;

// Define the button mappings using the global indices
const int SimonSaysGame::buttonMappings[4] = {
    button_MiddleLeftIndex,  // Map logical button 0 to actual button index
    button_MiddleRightIndex, // Map logical button 1 to actual button index
    button_BottomLeftIndex,  // Map logical button 2 to actual button index
    button_BottomRightIndex  // Map logical button 3 to actual button index
};

SimonSaysGame::SimonSaysGame(
    SSD1306Wire &disp,
    ButtonManager &buttonMgr,
    void (*beepForSq)(int),
    void (*beepOnUserPressSq)(int)
)
    : display(disp),
      buttonManager(buttonMgr),
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
      gameOverStartTime(0),
      currentlyPressedButton(-1) // Initialize with no button pressed
{
    for (int i = 0; i < MAX_PATTERN; i++) {
        pattern[i] = -1;
    }

    // Set the static instance pointer to this instance
    SimonSaysGame::instance = this;
}

void SimonSaysGame::begin() {
    // Register button callbacks with defined button mappings
    for (int i = 0; i < 4; i++) {
        buttonManager.registerCallback(buttonMappings[i], SimonSaysGame::onButtonEvent);
    }

    // Reset the game state
    resetGame();
}

void SimonSaysGame::end() {
    // Unregister button callbacks
    for (int i = 0; i < 4; i++) {
        buttonManager.unregisterCallback(buttonMappings[i]);
    }
}

void SimonSaysGame::update() {
    switch (currentState) {
    case WAIT_START:
        drawGrid(-1); // No highlight
        break;

    case SHOW_PATTERN:
        showPattern();
        break;

    case WAIT_USER:
        drawGrid(currentlyPressedButton); // Highlight button if pressed
        break;

    case GAME_OVER:
        if (millis() - gameOverStartTime > 2000) {
            resetGame();
        } else {
            drawGameOver();
        }
        break;
    }
}

void SimonSaysGame::onButtonEvent(const ButtonEvent& event) {
    if (instance) {
        int pressedButtonIndex = event.buttonIndex;

        // Map the button index to 0..3 for the game logic
        int gameButtonIndex = -1;
        for (int i = 0; i < 4; i++) {
            if (pressedButtonIndex == buttonMappings[i]) {
                gameButtonIndex = i;
                break;
            }
        }

        if (gameButtonIndex == -1) {
            return; // Button not relevant to SimonSaysGame
        }

        if (event.eventType == ButtonEvent_Pressed) {
            if (instance->currentState == WAIT_START) {
                instance->startRound();
            } else if (instance->currentState == WAIT_USER) {
                instance->currentlyPressedButton = gameButtonIndex;
                instance->checkUserInput(gameButtonIndex);
            }
        } else if (event.eventType == ButtonEvent_Released) {
            if (instance->currentlyPressedButton == gameButtonIndex) {
                instance->currentlyPressedButton = -1;
            }
        }
    }
}

// Ensure resetGame() is declared in the header file
void SimonSaysGame::resetGame() {
    patternLength = 0;
    patternIndex = 0;
    score = 0;
    currentState = WAIT_START;
    currentlyPressedButton = -1; // No button is pressed
    for (int i = 0; i < MAX_PATTERN; i++) {
        pattern[i] = -1;
    }
}

void SimonSaysGame::startRound() {
    extendPattern();
    patternIndex = 0;
    showingSquare = false;
    gapPhase = false;
    showStartTime = millis();
    showStepTime = showStartTime;
    currentState = SHOW_PATTERN;
}

void SimonSaysGame::extendPattern() {
    if (patternLength < MAX_PATTERN) {
        pattern[patternLength] = random(0, 4); // 0..3
        patternLength++;
    }
}

void SimonSaysGame::showPattern() {
    unsigned long now = millis();

    if (patternIndex >= patternLength) {
        currentState = WAIT_USER;
        patternIndex = 0;
        drawGrid(-1);
        return;
    }

    if (!showingSquare && !gapPhase) {
        int sq = pattern[patternIndex];
        drawGrid(sq);
        showingSquare = true;
        showStepTime = now;

        if (beepForSquare) {
            beepForSquare(sq);
        }
    } else if (showingSquare && (now - showStepTime > SHOW_STEP_ON)) {
        drawGrid(-1);
        showingSquare = false;
        gapPhase = true;
        showStepTime = now;
    } else if (gapPhase && (now - showStepTime > SHOW_STEP_OFF)) {
        gapPhase = false;
        patternIndex++;
    }
}

void SimonSaysGame::checkUserInput(int pressedButton) {
    if (pressedButton == pattern[patternIndex]) {
        patternIndex++;

        if (beepOnUserPress) {
            beepOnUserPress(pressedButton);
        }

        if (patternIndex >= patternLength) {
            score = patternLength;
            startRound();
        }
        // No need to handle button highlight here; it's managed in update()
    } else {
        gameOver();
    }
}

void SimonSaysGame::gameOver() {
    currentState = GAME_OVER;
    gameOverStartTime = millis();
    currentlyPressedButton = -1; // Ensure no button is shown as pressed
}

void SimonSaysGame::drawGameOver() {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 20, "GAME OVER");
    String msg = "Score: ";
    msg += score;
    display.drawString(64, 40, msg);
    display.display();
}

void SimonSaysGame::drawGrid(int highlightIndex) {
    display.clear();

    // Draw grid lines
    display.drawLine(0, 31, 127, 31);  // Horizontal line
    display.drawLine(63, 0, 63, 63);   // Vertical line

    // Highlight a square if specified
    if (highlightIndex != -1) {
        fillSquare(highlightIndex);
    }

    display.display();
}

void SimonSaysGame::fillSquare(int sq) {
    int xStart = 0, yStart = 0, width = 63, height = 31;

    switch (sq) {
        case 0: // Top-left
            xStart = 0;    yStart = 0;
            break;
        case 1: // Top-right
            xStart = 64;   yStart = 0;
            break;
        case 2: // Bottom-left
            xStart = 0;    yStart = 32;
            break;
        case 3: // Bottom-right
            xStart = 64;   yStart = 32;
            break;
        default:
            return; // Invalid index
    }

    // Fill the specified square
    display.fillRect(xStart, yStart, width, height);
}