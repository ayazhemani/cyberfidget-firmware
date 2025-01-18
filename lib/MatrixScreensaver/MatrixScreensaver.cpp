#include "MatrixScreensaver.h"
#include <SSD1306Wire.h>

// Example set, replace with your own
const char MatrixScreensaver::ALIEN_CHARS[16] = {
    '@', '#', '$', '%', '*', '+', '=','?',
    'Z', 'N', 'J', '!', ':', ';', '(', ')'
};

MatrixScreensaver::MatrixScreensaver(SSD1306Wire &disp)
: display(disp)
{
    // Adjust these as needed
    frameInterval = 40;         // ms between each update step
    rowTransitionDelay = 200;   // ms per row to turn on/off
}

void MatrixScreensaver::begin() {
    unsigned long now = millis();

    // Initialize columns
    for (int i = 0; i < NUM_COLUMNS; i++) {
        Column &col = columns[i];

        // Start all columns in OFF state
        col.state = OFF;
        col.topLitRow = 1;       // if topLitRow > bottomLitRow => no rows lit
        col.bottomLitRow = 0;

        // random OFF duration (time to remain fully off)
        col.offDuration = random(500, 2000); 
        col.nextStateChangeTime = now + col.offDuration;

        // random ON duration
        col.onDuration  = random(1500, 4000);

        // nextRowStepTime = not relevant in OFF state
        col.nextRowStepTime = 0;

        // Initialize row info
        for (int r = 0; r < NUM_ROWS; r++) {
            col.rows[r].ch = ' ';  
            col.rows[r].nextChange = 0; // no changes yet
        }
    }

    lastUpdateTime = now;
}

void MatrixScreensaver::update() {
    unsigned long now = millis();
    // Only do logic steps every frameInterval ms
    if (now - lastUpdateTime < frameInterval) {
        return;
    }
    lastUpdateTime = now;

    // For each column, run the state machine
    for (int i = 0; i < NUM_COLUMNS; i++) {
        Column &col = columns[i];

        switch (col.state) {
            case OFF: {
                // Check if time to start turning on
                if (now >= col.nextStateChangeTime) {
                    // We transition OFF -> TURNING_ON
                    startTurningOn(col, now);
                }
            } break;

            case TURNING_ON: {
                // We turn on one row at a time from the top to bottom
                // bottomLitRow goes from -1 up to NUM_ROWS-1
                if (now >= col.nextRowStepTime) {
                    col.bottomLitRow++;
                    col.nextRowStepTime = now + rowTransitionDelay;

                    if (col.bottomLitRow >= NUM_ROWS - 1) {
                        // Reached bottom => fully ON
                        col.bottomLitRow = NUM_ROWS - 1;
                        col.state = ON;
                        col.nextStateChangeTime = now + col.onDuration;
                    }
                }
            } break;

            case ON: {
                // While ON, rows from topLitRow=0 to bottomLitRow=NUM_ROWS-1 are lit
                // We can do random flickers in each row
                // Check if time to go TURNING_OFF
                if (now >= col.nextStateChangeTime) {
                    startTurningOff(col, now);
                }
            } break;

            case TURNING_OFF: {
                // We turn off one row at a time from the top down
                // topLitRow goes from 0 up to bottomLitRow
                if (now >= col.nextRowStepTime) {
                    col.topLitRow++;
                    col.nextRowStepTime = now + rowTransitionDelay;

                    // If topLitRow > bottomLitRow => no rows lit => OFF
                    if (col.topLitRow > col.bottomLitRow) {
                        col.state = OFF;
                        // Random OFF duration
                        col.offDuration = random(500, 2000);
                        col.nextStateChangeTime = now + col.offDuration;
                    }
                }
            } break;
        }

        // **Random Flicker**: if a row is lit, possibly change its char
        if (col.state == TURNING_ON || col.state == ON || col.state == TURNING_OFF) {
            // lit rows are from col.topLitRow..col.bottomLitRow
            int litTop = col.topLitRow;
            int litBot = col.bottomLitRow;
            // If we are turningOn, topLitRow=0 by default, bottomLitRow < topLitRow if we haven't lit anything?
            // Actually, we should ensure that topLitRow=0 from the moment we start turning on.
            // Similarly for turning off. Let's ensure that in startTurningOn/off.

            for (int r = litTop; r <= litBot; r++) {
                if (r < 0 || r >= NUM_ROWS) continue; // out of bounds
                // row is lit, check flicker
                RowInfo &rw = col.rows[r];
                if (now >= rw.nextChange) {
                    // pick a new char
                    rw.ch = randomAlienChar();
                    // next change in random 300..800 ms
                    rw.nextChange = now + random(300, 800);
                }
            }
        }
    }
}

void MatrixScreensaver::draw() {
    display.clear();
    // Use your real font. This is just a default example
    display.setFont(ArialMT_Plain_10);

    // For each column => x position
    for (int i = 0; i < NUM_COLUMNS; i++) {
        int colX = i * (FONT_WIDTH + 1); // a bit of horizontal spacing if desired
        Column &col = columns[i];

        int litTop = col.topLitRow;
        int litBot = col.bottomLitRow;
        // Draw rows in [litTop .. litBot]
        for (int r = litTop; r <= litBot; r++) {
            if (r < 0 || r >= NUM_ROWS) continue;
            int rowY = r * FONT_HEIGHT;
            char c   = col.rows[r].ch;
            drawChar(colX, rowY, c);
        }
    }

    display.display();
}

/*--- State transition helpers ---*/

// Called when transitioning from OFF -> TURNING_ON
void MatrixScreensaver::startTurningOn(Column &col, unsigned long now) {
    col.state = TURNING_ON;
    // Re-randomize the entire column’s symbols once we begin turning on
    randomizeColumnSymbols(col);

    // We start with topLitRow=0 always (lighting from top row #0)
    col.topLitRow = 0;
    // bottomLitRow = -1 means no rows are lit yet
    col.bottomLitRow = -1;
    // We will light the first row
    col.bottomLitRow++;
    col.nextRowStepTime = now + rowTransitionDelay;

    // We also know once fully ON, we remain on for col.onDuration
    // (that gets set in the main loop once bottomLitRow == NUM_ROWS-1)
}

// Called when transitioning from ON -> TURNING_OFF
void MatrixScreensaver::startTurningOff(Column &col, unsigned long now) {
    col.state = TURNING_OFF;
    // We turn off from the top down
    col.topLitRow = 0;
    col.bottomLitRow = NUM_ROWS - 1;
    col.nextRowStepTime = now + rowTransitionDelay;
}

/*--- Randomizing the column’s symbols all at once ---*/
void MatrixScreensaver::randomizeColumnSymbols(Column &col) {
    for (int r = 0; r < NUM_ROWS; r++) {
        col.rows[r].ch = randomAlienChar();
        // set an initial random flicker time so it can flicker eventually
        col.rows[r].nextChange = millis() + random(300, 800);
    }
}

char MatrixScreensaver::randomAlienChar() {
    int idx = random(0, 16);
    return ALIEN_CHARS[idx];
}

/*--- Actually draw a character at (x,y) ---*/
void MatrixScreensaver::drawChar(int x, int y, char c) {
    // If using an internal function that uses your custom font data, do so here.
    // Example with SSD1306Wire:
    String s(c);
    display.drawString(x, y, s);
}
