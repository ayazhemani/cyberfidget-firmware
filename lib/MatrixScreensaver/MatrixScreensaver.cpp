#include "MatrixScreensaver.h"
#include <SSD1306Wire.h> // or your specific SSD1306 library
#include "fontNotoSansJPReg.h"

// Here’s a small set of random glyphs / ASCII for demonstration
// Add your own or use an extended set for more variety
const char MatrixScreensaver::ALIEN_CHARS[16] = {
  '@', '#', '$', '%', '*', '+', '=','?',
  'Z', 'N', 'J', '!', ':', ';', '(', ')'
};

// const char MatrixScreensaver::ALIEN_CHARS[16] = {
//   'ジ', 'イ', 'キ', 'ョ', 'テ', 'カ', 'ベ','イ',
//   'Z', 'N', 'J', 'ン', 'ハ', 'コ', 'メ', 'W'
// };
// ヨッテ、 ココ ニ、 コクレン ソウカ ハ
// ステ ノ ニンゲン ハ
// イク 、 コクサ レゴウ ト ョウリクシ

MatrixScreensaver::MatrixScreensaver(SSD1306Wire &disp)
: display(disp),
  lastUpdateTime(0),
  scrollInterval(100) // default 250 ms
{
    // columns[] init in begin()
}

void MatrixScreensaver::begin() {
    // Calculate how many pixels each column is wide
    // For a typical 6-wide font, we might space them out a bit
    // Let's define spacing:
    int columnWidth = FONT_WIDTH;

    // We want NUM_COLUMNS across 128 px. 
    // The leftmost column at x=0, next at x=columnWidth, etc.
    // Possibly skip some pixels for spacing if you prefer.
    for (int i = 0; i < NUM_COLUMNS; i++) {
        columns[i].x = i * columnWidth; 
        // random speed 1..3 px per update
        columns[i].speed = random(1, 4);
        // start offset at random negative so columns are staggered
        columns[i].offset = -random(0, 64);

        // Fill with random symbols
        for (int j = 0; j < COLUMN_SIZE; j++) {
            columns[i].symbols[j].ch = randomAlienChar();
            columns[i].symbols[j].life = j; // for variety
        }
    }

    lastUpdateTime = millis(); 
}

void MatrixScreensaver::setScrollInterval(unsigned long intervalMs) {
    scrollInterval = intervalMs;
}

void MatrixScreensaver::setColumnSpeed(int speed) {
    // e.g. apply the same speed to all columns
    for (int i = 0; i < NUM_COLUMNS; i++) {
        columns[i].speed = speed;
    }
}

void MatrixScreensaver::update() {
    unsigned long now = millis();
    // Only scroll if enough time has passed
    if (now - lastUpdateTime < scrollInterval) {
        return; 
    }
    lastUpdateTime = now;

    // Now we do the “scroll” step
    for (int i = 0; i < NUM_COLUMNS; i++) {
        Column &col = columns[i];
        col.offset += col.speed;

        // If offset >= FONT_HEIGHT, shift the column
        while (col.offset >= FONT_HEIGHT) {
            col.offset -= FONT_HEIGHT;
            shiftColumn(col);
        }
    }
}

void MatrixScreensaver::draw() {
    // Clear screen
    display.clear();
    display.setFont(ArialMT_Plain_10);

    // Render each column
    for (int i = 0; i < NUM_COLUMNS; i++) {
        Column &col = columns[i];

        // We place the top symbol near y = -offset 
        // The next symbol is y = -offset + FONT_HEIGHT, etc...
        // But the array is from top->bottom in the sense that array[0]
        // is the top-most symbol. 
        // We'll invert that so array[0] is the "bottom-most" if you prefer,
        // but let's keep it straightforward: array[0] is top.
        // Then the on-screen row for array[k] is: rowY = (k * FONT_HEIGHT) - col.offset

        for (int k = 0; k < COLUMN_SIZE; k++) {
            int rowY = (k * FONT_HEIGHT) - col.offset;

            // Only draw if visible on screen
            if (rowY < -FONT_HEIGHT) {
                continue; // above top
            } 
            if (rowY >= 64) {
                break; // below bottom (no need to keep drawing)
            }

            // Draw char columns[i].symbols[k] at (col.x, rowY) with fade logic
            Symbol &sym = col.symbols[k];
            drawChar(col.x, rowY, sym.ch, sym.life);
        }
    }

    // Finally, push buffer to display
    display.display();
}

char MatrixScreensaver::randomAlienChar() {
    // pick from ALIEN_CHARS
    int idx = random(0, 16);
    return ALIEN_CHARS[idx];
}

void MatrixScreensaver::shiftColumn(Column &col) {
    // shift everything down by 1 
    // i.e., symbols[COLUMN_SIZE-1] is discarded,
    // array[1..COLUMN_SIZE-1] moves to array[2..COLUMN_SIZE], etc.
    // then new random at [0]

    // We can do a reverse loop:
    for (int i = COLUMN_SIZE - 1; i > 0; i--) {
        // Move the symbol below
        col.symbols[i].ch   = col.symbols[i - 1].ch;
        col.symbols[i].life = col.symbols[i - 1].life + 1; 
    }
    // Insert new “head” at top
    col.symbols[0].ch   = randomAlienChar();
    col.symbols[0].life = 0;
}

void MatrixScreensaver::drawChar(int x, int y, char c, uint8_t life) {
    // If your SSD1306Wire library supports built-in text rendering, use that:
    // e.g. display.setFont(Monospaced_plain_8); display.drawString(x, y, String(c));
    // 
    // But if you only have "setPixel", you'll need a minimal bitmap font routine.
    // We'll assume you have a function display.drawString() for single char:
    // 
    // We simulate fade by skipping some draws for older symbols
    // or use random flicker chance. For example:
    // If life < 2, always draw (bright "head")
    // If 2 <= life < 6, skip with small probability
    // If life >= 6, skip with larger probability
    // Tweak to taste
    if (life < 15) {
        // always draw
    } else if (life < 30) {
        // maybe skip 20% of the time
        if (random(100) < 20) {
            return; // skip drawing
        }
    } else {
        // older -> skip 50% of the time
        if (random(100) < 50) {
            return;
        }
    }

    // Actually draw the char. If your library has built-in text:
    String s(c);
    display.drawString(x, y, s);

    // Alternatively, if you only have setPixel, implement a small font.
}