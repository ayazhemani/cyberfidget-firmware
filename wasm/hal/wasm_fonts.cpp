// Minimal bitmap fonts in ThingPulse OLEDDisplay format for the WASM emulator.
//
// These provide readable text using a classic 5x7 pixel font. For pixel-perfect
// rendering matching the real device, replace with the full ThingPulse
// OLEDDisplayFonts.h arrays (MIT licensed, available from their GitHub repo).
//
// ThingPulse font format:
//   [0] max width, [1] height, [2] first char (32), [3] char count (95)
//   Jump table: 4 bytes per char → [pos_hi, pos_lo, size, width]
//   Bitmap data: column-major, 1 byte per column (for height <= 8)

#include <cstdint>
#include <cstring>

// Raw 5x7 glyph data — one byte per column, MSB=bottom
// Standard ASCII 32-126 (95 characters)
struct GlyphDef { const uint8_t* data; uint8_t size; uint8_t width; };

// Individual glyph column data
static const uint8_t g33[]  = {0x5F};
static const uint8_t g34[]  = {0x03, 0x00, 0x03};
static const uint8_t g35[]  = {0x14, 0x7F, 0x14, 0x7F, 0x14};
static const uint8_t g36[]  = {0x24, 0x2A, 0x7F, 0x2A, 0x12};
static const uint8_t g37[]  = {0x23, 0x13, 0x08, 0x64, 0x62};
static const uint8_t g38[]  = {0x36, 0x49, 0x55, 0x22, 0x50};
static const uint8_t g39[]  = {0x03};
static const uint8_t g40[]  = {0x1C, 0x22, 0x41};
static const uint8_t g41[]  = {0x41, 0x22, 0x1C};
static const uint8_t g42[]  = {0x14, 0x08, 0x3E, 0x08, 0x14};
static const uint8_t g43[]  = {0x08, 0x08, 0x3E, 0x08, 0x08};
static const uint8_t g44[]  = {0x80, 0x60};
static const uint8_t g45[]  = {0x08, 0x08, 0x08, 0x08};
static const uint8_t g46[]  = {0x60};
static const uint8_t g47[]  = {0x20, 0x10, 0x08, 0x04, 0x02};
static const uint8_t g48[]  = {0x3E, 0x51, 0x49, 0x45, 0x3E};
static const uint8_t g49[]  = {0x42, 0x7F, 0x40};
static const uint8_t g50[]  = {0x42, 0x61, 0x51, 0x49, 0x46};
static const uint8_t g51[]  = {0x21, 0x41, 0x45, 0x4B, 0x31};
static const uint8_t g52[]  = {0x18, 0x14, 0x12, 0x7F, 0x10};
static const uint8_t g53[]  = {0x27, 0x45, 0x45, 0x45, 0x39};
static const uint8_t g54[]  = {0x3C, 0x4A, 0x49, 0x49, 0x30};
static const uint8_t g55[]  = {0x01, 0x71, 0x09, 0x05, 0x03};
static const uint8_t g56[]  = {0x36, 0x49, 0x49, 0x49, 0x36};
static const uint8_t g57[]  = {0x06, 0x49, 0x49, 0x29, 0x1E};
static const uint8_t g58[]  = {0x36};
static const uint8_t g59[]  = {0x80, 0x56};
static const uint8_t g60[]  = {0x08, 0x14, 0x22, 0x41};
static const uint8_t g61[]  = {0x14, 0x14, 0x14, 0x14, 0x14};
static const uint8_t g62[]  = {0x41, 0x22, 0x14, 0x08};
static const uint8_t g63[]  = {0x02, 0x01, 0x51, 0x09, 0x06};
static const uint8_t g64[]  = {0x3E, 0x41, 0x5D, 0x55, 0x1E};
static const uint8_t g65[]  = {0x7E, 0x09, 0x09, 0x09, 0x7E};
static const uint8_t g66[]  = {0x7F, 0x49, 0x49, 0x49, 0x36};
static const uint8_t g67[]  = {0x3E, 0x41, 0x41, 0x41, 0x22};
static const uint8_t g68[]  = {0x7F, 0x41, 0x41, 0x22, 0x1C};
static const uint8_t g69[]  = {0x7F, 0x49, 0x49, 0x49, 0x41};
static const uint8_t g70[]  = {0x7F, 0x09, 0x09, 0x09, 0x01};
static const uint8_t g71[]  = {0x3E, 0x41, 0x49, 0x49, 0x3A};
static const uint8_t g72[]  = {0x7F, 0x08, 0x08, 0x08, 0x7F};
static const uint8_t g73[]  = {0x41, 0x7F, 0x41};
static const uint8_t g74[]  = {0x20, 0x40, 0x41, 0x3F, 0x01};
static const uint8_t g75[]  = {0x7F, 0x08, 0x14, 0x22, 0x41};
static const uint8_t g76[]  = {0x7F, 0x40, 0x40, 0x40, 0x40};
static const uint8_t g77[]  = {0x7F, 0x02, 0x04, 0x02, 0x7F};
static const uint8_t g78[]  = {0x7F, 0x04, 0x08, 0x10, 0x7F};
static const uint8_t g79[]  = {0x3E, 0x41, 0x41, 0x41, 0x3E};
static const uint8_t g80[]  = {0x7F, 0x09, 0x09, 0x09, 0x06};
static const uint8_t g81[]  = {0x3E, 0x41, 0x51, 0x21, 0x5E};
static const uint8_t g82[]  = {0x7F, 0x09, 0x19, 0x29, 0x46};
static const uint8_t g83[]  = {0x46, 0x49, 0x49, 0x49, 0x31};
static const uint8_t g84[]  = {0x01, 0x01, 0x7F, 0x01, 0x01};
static const uint8_t g85[]  = {0x3F, 0x40, 0x40, 0x40, 0x3F};
static const uint8_t g86[]  = {0x1F, 0x20, 0x40, 0x20, 0x1F};
static const uint8_t g87[]  = {0x3F, 0x40, 0x30, 0x40, 0x3F};
static const uint8_t g88[]  = {0x63, 0x14, 0x08, 0x14, 0x63};
static const uint8_t g89[]  = {0x07, 0x08, 0x70, 0x08, 0x07};
static const uint8_t g90[]  = {0x61, 0x51, 0x49, 0x45, 0x43};
static const uint8_t g91[]  = {0x7F, 0x41, 0x41};
static const uint8_t g92[]  = {0x02, 0x04, 0x08, 0x10, 0x20};
static const uint8_t g93[]  = {0x41, 0x41, 0x7F};
static const uint8_t g94[]  = {0x04, 0x02, 0x01, 0x02, 0x04};
static const uint8_t g95[]  = {0x40, 0x40, 0x40, 0x40, 0x40};
static const uint8_t g96[]  = {0x01, 0x02};
static const uint8_t g97[]  = {0x20, 0x54, 0x54, 0x54, 0x78};
static const uint8_t g98[]  = {0x7F, 0x48, 0x44, 0x44, 0x38};
static const uint8_t g99[]  = {0x38, 0x44, 0x44, 0x44, 0x20};
static const uint8_t g100[] = {0x38, 0x44, 0x44, 0x48, 0x7F};
static const uint8_t g101[] = {0x38, 0x54, 0x54, 0x54, 0x18};
static const uint8_t g102[] = {0x08, 0x7E, 0x09, 0x01, 0x02};
static const uint8_t g103[] = {0x18, 0xA4, 0xA4, 0xA4, 0x7C};
static const uint8_t g104[] = {0x7F, 0x08, 0x04, 0x04, 0x78};
static const uint8_t g105[] = {0x44, 0x7D, 0x40};
static const uint8_t g106[] = {0x40, 0x80, 0x84, 0x7D};
static const uint8_t g107[] = {0x7F, 0x10, 0x28, 0x44};
static const uint8_t g108[] = {0x41, 0x7F, 0x40};
static const uint8_t g109[] = {0x7C, 0x04, 0x18, 0x04, 0x78};
static const uint8_t g110[] = {0x7C, 0x08, 0x04, 0x04, 0x78};
static const uint8_t g111[] = {0x38, 0x44, 0x44, 0x44, 0x38};
static const uint8_t g112[] = {0xFC, 0x24, 0x24, 0x24, 0x18};
static const uint8_t g113[] = {0x18, 0x24, 0x24, 0x24, 0xFC};
static const uint8_t g114[] = {0x7C, 0x08, 0x04, 0x04, 0x08};
static const uint8_t g115[] = {0x48, 0x54, 0x54, 0x54, 0x20};
static const uint8_t g116[] = {0x04, 0x3F, 0x44, 0x40, 0x20};
static const uint8_t g117[] = {0x3C, 0x40, 0x40, 0x20, 0x7C};
static const uint8_t g118[] = {0x1C, 0x20, 0x40, 0x20, 0x1C};
static const uint8_t g119[] = {0x3C, 0x40, 0x30, 0x40, 0x3C};
static const uint8_t g120[] = {0x44, 0x28, 0x10, 0x28, 0x44};
static const uint8_t g121[] = {0x1C, 0xA0, 0xA0, 0xA0, 0x7C};
static const uint8_t g122[] = {0x44, 0x64, 0x54, 0x4C, 0x44};
static const uint8_t g123[] = {0x08, 0x36, 0x41};
static const uint8_t g124[] = {0x7F};
static const uint8_t g125[] = {0x41, 0x36, 0x08};
static const uint8_t g126[] = {0x08, 0x04, 0x08, 0x10, 0x08};

static const GlyphDef glyphs[95] = {
    {nullptr,0, 3}, {g33,1,2}, {g34,3,4}, {g35,5,6}, {g36,5,6}, {g37,5,6},
    {g38,5,6}, {g39,1,2}, {g40,3,4}, {g41,3,4}, {g42,5,6}, {g43,5,6},
    {g44,2,3}, {g45,4,5}, {g46,1,2}, {g47,5,6}, {g48,5,6}, {g49,3,4},
    {g50,5,6}, {g51,5,6}, {g52,5,6}, {g53,5,6}, {g54,5,6}, {g55,5,6},
    {g56,5,6}, {g57,5,6}, {g58,1,2}, {g59,2,3}, {g60,4,5}, {g61,5,6},
    {g62,4,5}, {g63,5,6}, {g64,5,6}, {g65,5,6}, {g66,5,6}, {g67,5,6},
    {g68,5,6}, {g69,5,6}, {g70,5,6}, {g71,5,6}, {g72,5,6}, {g73,3,4},
    {g74,5,6}, {g75,5,6}, {g76,5,6}, {g77,5,6}, {g78,5,6}, {g79,5,6},
    {g80,5,6}, {g81,5,6}, {g82,5,6}, {g83,5,6}, {g84,5,6}, {g85,5,6},
    {g86,5,6}, {g87,5,6}, {g88,5,6}, {g89,5,6}, {g90,5,6}, {g91,3,4},
    {g92,5,6}, {g93,3,4}, {g94,5,6}, {g95,5,6}, {g96,2,3}, {g97,5,6},
    {g98,5,6}, {g99,5,6}, {g100,5,6}, {g101,5,6}, {g102,5,6}, {g103,5,6},
    {g104,5,6}, {g105,3,4}, {g106,4,5}, {g107,4,5}, {g108,3,4}, {g109,5,6},
    {g110,5,6}, {g111,5,6}, {g112,5,6}, {g113,5,6}, {g114,5,6}, {g115,5,6},
    {g116,5,6}, {g117,5,6}, {g118,5,6}, {g119,5,6}, {g120,5,6}, {g121,5,6},
    {g122,5,6}, {g123,3,4}, {g124,1,2}, {g125,3,4}, {g126,5,6},
};

// Build a font buffer in ThingPulse format.
// For 16px and 24px, we scale the base 8px glyphs by repeating rows.
static void buildFontBuffer(uint8_t* buf, int bufSize, uint8_t height, uint8_t maxWidth) {
    memset(buf, 0, bufSize);
    buf[0] = maxWidth;
    buf[1] = height;
    buf[2] = 32; // first char
    buf[3] = 95; // char count

    uint8_t colBytes = (height + 7) / 8;

    // First pass: compute data offsets and total size
    int headerSize = 4 + 95 * 4;
    uint16_t dataPos = 0;

    for (int i = 0; i < 95; i++) {
        uint8_t* jt = buf + 4 + i * 4;
        const GlyphDef& g = glyphs[i];

        if (g.size == 0) {
            jt[0] = 0xFF; jt[1] = 0xFF; jt[2] = 0; jt[3] = g.width;
            continue;
        }

        uint8_t scaledSize = g.size * colBytes; // columns * bytes_per_column
        jt[0] = (dataPos >> 8) & 0xFF;
        jt[1] = dataPos & 0xFF;
        jt[2] = scaledSize;
        jt[3] = g.width;

        // Write scaled bitmap data
        uint8_t* dst = buf + headerSize + dataPos;
        for (uint8_t col = 0; col < g.size; col++) {
            uint8_t srcByte = g.data[col];
            // Scale 7 rows to 'height' rows
            for (uint8_t row = 0; row < height; row++) {
                uint8_t srcRow = row * 7 / height;
                if (srcRow > 6) srcRow = 6;
                if (srcByte & (1 << srcRow)) {
                    uint8_t byteIdx = col * colBytes + row / 8;
                    dst[byteIdx] |= (1 << (row & 7));
                }
            }
        }
        dataPos += scaledSize;
    }
}

// Static font buffers (header + jump table + glyph data)
static uint8_t font10_buf[4 + 95*4 + 95*5*1 + 64]; // height=8, 1 byte/col
static uint8_t font16_buf[4 + 95*4 + 95*5*2 + 64]; // height=14, 2 bytes/col
static uint8_t font24_buf[4 + 95*4 + 95*5*3 + 64]; // height=21, 3 bytes/col

static bool s_fontsReady = false;

static void initFonts() {
    if (s_fontsReady) return;
    s_fontsReady = true;
    buildFontBuffer(font10_buf, sizeof(font10_buf), 8, 6);
    buildFontBuffer(font16_buf, sizeof(font16_buf), 14, 10);
    buildFontBuffer(font24_buf, sizeof(font24_buf), 21, 14);
}

// Global font arrays matching the ThingPulse extern declarations.
// Apps reference these as: display.setFont(ArialMT_Plain_10);
// They're const uint8_t[] in the original library.
// Here they point to our runtime-built buffers via a thin indirection.

const uint8_t* ArialMT_Plain_10 = font10_buf;
const uint8_t* ArialMT_Plain_16 = font16_buf;
const uint8_t* ArialMT_Plain_24 = font24_buf;

namespace { struct FontInit_ { FontInit_() { initFonts(); } } s_fontInit; }
