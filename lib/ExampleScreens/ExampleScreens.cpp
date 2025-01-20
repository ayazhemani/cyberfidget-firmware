#include "globals.h"
#include "SSD1306Wire.h"
#include "ExampleScreens.h"
#include "images.h"

extern SSD1306Wire display;

void drawFontFaceDemo() {
  display.clear();
  // Font Demo1
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  //display.setFont(ArialMT_Plain_10);
  //display.drawString(0, 0, "Hello world");
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 18, "Hello world");
  //display.setFont(ArialMT_Plain_24);
  //display.drawString(0, 26, "Hello world");
  display.display();
}

void drawTextFlowDemo() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawStringMaxWidth(0, 0, 128,
  "Lorem ipsum confused the non-super nerds so here is some plain english filler to demo text wrapping" );
  display.display();
}

void drawTextAlignmentDemo() {
  display.clear();
  // Text alignment demo
  display.setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 33, "Right aligned (128,33)");
  display.display();
}

void drawRectDemo() {
  display.clear();
  // Draw a pixel at given position
  for (int i = 0; i < 10; i++) {
    display.setPixel(i, i);
    display.setPixel(10 - i, i);
  }
  display.drawRect(12, 12, 20, 20);

  // Fill the rectangle
  display.fillRect(14, 14, 17, 17);

  // Draw a line horizontally
  display.drawHorizontalLine(0, 40, 20);

  // Draw a line horizontally
  display.drawVerticalLine(40, 0, 20);
  display.display();
}

void drawCircleDemo() {
  display.clear();
  for (int i = 1; i < 8; i++) {
    display.setColor(WHITE);
    display.drawCircle(32, 32, i * 3);
    if (i % 2 == 0) {
      display.setColor(BLACK);
    }
    display.fillCircle(96, 32, 32 - i * 3);
  }
  display.display();
}

void drawImageDemo_1() {
  display.clear();
  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html or https://github.com/ThingPulse/esp8266-oled-ssd1306
  // on how to create xbm files
  //display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.drawXbm(40, 0, Logo_Round_Tilted_width, Logo_Round_Tilted_height, Logo_Round_Tilted_bits);
  //display.drawXbm(0, 0, 128, 64, epd_bitmap_cf_logo_w_icon);
  display.display();
}

void drawImageDemo_2() {
  display.clear();
  display.drawXbm(0, 0, 128, 64, epd_bitmap_cf_logo_w_icon);
  display.display();
}

void drawImageDemo_3() {
  display.clear();
  display.drawXbm(0, 0, 128, 64, epd_bitmap_cf_logo);
  display.display();
}

void drawImageDemo_4() {
  display.clear();
  display.drawXbm(0, 0, 128, 64, epd_bitmap_retro_car_sunset);
  display.display();
}