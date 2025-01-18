#ifndef MATRIX_SCREENSAVER_DITHER_H
#define MATRIX_SCREENSAVER_DITHER_H

#include <Arduino.h>

class SSD1306Wire;  // Forward declaration of your display class

/**
 * @brief A "Matrix" (the movie) style screensaver with time-based scrolling
 *        and per-pixel dithering fade on a 1-bit OLED (SSD1306).
 *
 * Usage:
 *   1) Create an instance: MatrixScreensaverDither matrix(display);
 *   2) Call matrix.begin() in setup().
 *   3) In your loop():
 *        matrix.update(); // moves columns if scrollInterval elapsed
 *        matrix.draw();   // draws them with dithering
 *   4) Optionally set the scroll speed/time: matrix.setScrollInterval(...)
 *      or uniform column speed: matrix.setColumnSpeed(...).
 *
 * Implementation details (font data, bayer matrix, dithering logic)
 * should go into MatrixScreensaverDither.cpp or similar.
 */
class MatrixScreensaverDither {
public:
    /// Width/height of each text cell
    static const int FONT_WIDTH  = 6;  
    static const int FONT_HEIGHT = 8;

    /// Number of columns across the screen
    static const int NUM_COLUMNS = 12;
    /// Vertical “stack size” in each column
    static const int COLUMN_SIZE = 12;

    static const char ALIEN_CHARS[16];

    /**
     * @brief Construct a new MatrixScreensaverDither object.
     * @param disp Reference to your SSD1306Wire display instance.
     */
    MatrixScreensaverDither(SSD1306Wire &disp);

    /**
     * @brief Initialize columns with random data. Call in setup().
     */
    void begin();

    /**
     * @brief Updates the column offsets/time. Only scrolls after
     *        scrollInterval ms have passed (time-based).
     */
    void update();

    /**
     * @brief Renders the columns using dithering for fade-out effect,
     *        then calls display.display().
     */
    void draw();

    /**
     * @brief Sets the time in ms between scroll updates. Default ~250ms.
     * @param intervalMs Time in milliseconds
     */
    void setScrollInterval(unsigned long intervalMs);

    /**
     * @brief Sets the same pixel scroll speed for all columns.
     *        If you want random speeds, skip this.
     * @param speed pixels per scroll step
     */
    void setColumnSpeed(int speed);

private:
    /// Reference to the OLED
    SSD1306Wire &display;

    /// Last time we scrolled
    unsigned long lastUpdateTime;
    /// Min ms between scroll steps
    unsigned long scrollInterval;

    /// Each symbol has a character + life for fading
    struct Symbol {
        char ch;
        uint8_t life; // grows as symbol shifts down
    };

    struct Column {
        int x;         // left pixel coordinate
        int speed;     // vertical speed in px per step
        int offset;    // current vertical offset in px
        Symbol symbols[COLUMN_SIZE];
    };

    Column columns[NUM_COLUMNS];

    /**
     * @brief Randomly pick a new “alien” character (from your custom set).
     * @return char The chosen symbol.
     */
    char randomAlienChar();

    /**
     * @brief Shift the column content down by one symbol, discard bottom, spawn new top.
     * @param col The column to shift.
     */
    void shiftColumn(Column &col);

    /**
     * @brief Draw a single character at (x,y) with dithering to simulate fade.
     * @param x Left coordinate
     * @param y Top coordinate
     * @param c The character to draw
     * @param fadeAlpha "Brightness" in [0..16], where 16=full white, 0=off.
     *
     * Implementation will use a 6×8 font, the Bayer 4×4 matrix,
     * and do per-pixel setPixel() if bayer < fadeAlpha.
     */
    void drawDitheredChar(int x, int y, char c, int fadeAlpha);
};

#endif // MATRIX_SCREENSAVER_DITHER_H
