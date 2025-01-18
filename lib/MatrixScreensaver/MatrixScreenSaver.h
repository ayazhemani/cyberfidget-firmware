#ifndef MATRIX_SCREENSAVER_H
#define MATRIX_SCREENSAVER_H

#include <Arduino.h>

class SSD1306Wire; // forward declaration

class MatrixScreensaver {
public:
    static const int SCREEN_HEIGHT = 64; // or 32, etc.
    static const int FONT_HEIGHT   = 12;  // each char row is 8 px tall
    static const int NUM_ROWS      = SCREEN_HEIGHT / FONT_HEIGHT; // e.g. 8 if 64 px tall
    static const int FONT_WIDTH    = 10;  // or 8, depending on your font
    static const int NUM_COLUMNS   = 16; // how many columns horizontally

    static const char ALIEN_CHARS[16];

    MatrixScreensaver(SSD1306Wire &disp);

    void begin();
    void update();
    void draw();

private:
    SSD1306Wire &display;

    // We'll run a basic “step” every frameInterval ms
    unsigned long lastUpdateTime;
    unsigned long frameInterval;

    // Rate at which we turn on/off each row from top to bottom
    unsigned long rowTransitionDelay;

    // For flicker updates, how often we check if a row can change char
    // (We can either do it in the main update() or track times per row.)
    // We'll track times per row for more randomness.

    enum ColumnState {
        OFF,
        TURNING_ON,
        ON,
        TURNING_OFF
    };

    struct RowInfo {
        char  ch;                 // current character in this row
        unsigned long nextChange; // next time in ms we flicker to a new char
    };

    struct Column {
        ColumnState state;

        // row indices for the “lit” region
        int topLitRow;      // which row index is the topmost lit?
        int bottomLitRow;   // which row index is the bottommost lit?

        // next time we do a row on/off step
        unsigned long nextRowStepTime;

        // durations for how long we remain fully ON or fully OFF
        unsigned long onDuration;
        unsigned long offDuration;
        unsigned long nextStateChangeTime; // when we leave ON or OFF state

        // each row holds a RowInfo
        RowInfo rows[NUM_ROWS];
    };

    Column columns[NUM_COLUMNS];

    // Helper methods
    void startTurningOn(Column &col, unsigned long now);
    void startTurningOff(Column &col, unsigned long now);
    void randomizeColumnSymbols(Column &col);
    char randomAlienChar();
    void drawChar(int x, int y, char c);
};

#endif
