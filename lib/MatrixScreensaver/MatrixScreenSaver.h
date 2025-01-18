#ifndef MATRIX_SCREENSAVER_H
#define MATRIX_SCREENSAVER_H

#include <Arduino.h>

class SSD1306Wire; // forward declaration of your OLED display class

class MatrixScreensaver {
public:
    // Adjust these to taste
    static const int FONT_WIDTH  = 8;  // width of each character in pixels
    static const int FONT_HEIGHT = 8;  // height of each character in pixels

    // The 128×64 screen has 128 / 6 = ~21 columns if using a 6-wide font
    // We'll pick fewer columns to keep spacing. For example, 16 columns:
    static const int NUM_COLUMNS = 16;

    // How many "characters" can be in each column at once? 
    // The screen is 64 px high → 8 lines of text if each line is 8 px tall. 
    // We'll store a bit extra. Let’s do 12 slots so we can rotate them.
    static const int COLUMN_SIZE = 12;

    // Each column's vertical speed can vary slightly
    // We'll store a velocity in "pixels per update" or so
    // For finer control, we might store a float and accumulate partial movement.
    // We'll keep it simple as an integer for demonstration.

    // Our "weird alien" symbols are random from a small set. 
    // You can add your own custom set or entire ASCII range.
    // For a quick "Matrix" vibe, let's pick ~16 random glyphs.
    // You can replace with Katakana or ASCII range as needed.
    static const char ALIEN_CHARS[16];

    MatrixScreensaver(SSD1306Wire &disp);

    void begin();                  // Initialize columns
    void update();                 // Move the drip
    void draw();                   // Render on the display

    /**
     * @brief Sets the time in ms between scroll updates (default e.g. 250).
     * @param intervalMs time in milliseconds
    */
    void setScrollInterval(unsigned long intervalMs);

    /**
     * @brief If you prefer all columns to have the same speed, set here. 
     *        If you want random speeds, keep using random(1,4) in begin().
    */
    void setColumnSpeed(int speed);

private:
    SSD1306Wire &display;

    // Time-based scrolling
    unsigned long lastUpdateTime;
    unsigned long scrollInterval; // e.g. 250 ms

    // Column struct with fade info
    struct Symbol {
        char ch;
        uint8_t life; // how long since it was spawned (could be 0..255)
    };

    struct Column {
        int x;                       // x position (left in pixels)
        int speed;                   // vertical speed in pixels per step
        int offset;                  // fractional offset for vertical scrolling
        Symbol symbols[COLUMN_SIZE];   // the stored alien chars
    };

    // We store all columns in a static array
    Column columns[NUM_COLUMNS];

    // Generate a new random symbol from ALIEN_CHARS
    char randomAlienChar();

    // Shift the column’s symbols down by 1 
    // (discard bottom, insert new symbol at top)
    void shiftColumn(Column &col);

    // Draw a single character at (x, y)
    void drawChar(int x, int y, char c, uint8_t life);
};

#endif