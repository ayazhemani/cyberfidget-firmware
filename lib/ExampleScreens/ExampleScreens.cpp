#include "globals.h"
#include "ExampleScreens.h"
#include "images.h"
#include "RGBController.h"
#include "CFHAL.h"

static auto& strip         = HAL::strip();  // NeoPixels
static auto& display       = HAL::displayProxy();

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

void drawBatteryProgressBar() {
  display.clear();
  // draw the progress bar
  display.drawProgressBar(0, 30, 120, 10, batteryVoltagePercentage);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, "Battery: " + String(batteryVoltagePercentage) + "%");

  // Battery Voltage
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(70, 40, "Battery (V): " + String(batteryVoltage));

  display.display();
}

void drawSliderProgressBar() {
  display.clear();
  display.setFont(ArialMT_Plain_10);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 4, "Slider: " + String(sliderPosition_Percentage_Inverted_Filtered) + "%");

  // draw the progress bar
  display.setFont(ArialMT_Plain_10);
  display.drawProgressBar(9, 18, 108, 10, sliderPosition_Percentage_Inverted_Filtered);

  // Battery Voltage
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 30, "Slider (mV): " + String(sliderPosition_Millivolts));
  display.drawString(64, 40, "Slider (bits): " + String(sliderPosition_12Bits));
  display.drawString(64, 50, "Slider Filt (bits): " + String(sliderPosition_12Bits_Filtered));
  display.display();

  uint8_t red, green, blue;
  mapToRainbow(sliderPosition_12Bits_Filtered, 8, red, green, blue);
  strip.setPixelColor(pixel_Front_Top, strip.Color(red, green, blue, 0));
  strip.setPixelColor(pixel_Front_Middle, strip.Color(8 - red, 8 - green, 8 - blue, 0));
  strip.setPixelColor(pixel_Front_Bottom, strip.Color(red, green, blue, 0));

  updateStrip();
}

void drawAccelerometerScreen() {
  if (accelerometerScreenEnabled) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 20, "X: " + String(accelX) + " Y: " + String(accelY) + " Z: " + String(accelZ));

    uint8_t redMap = map(accelX, -1030, 1030, 0, 255);
    uint8_t greenMap = map(accelY, -1030, 1030, 0, 255);
    uint8_t blueMap = map(accelZ, -1030, 1030, 0, 255);

    setDeterminedColorsFront(redMap, greenMap, blueMap, 0); 

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 10, "R: " + String(redMap) + " G: " + String(greenMap) + " B: " + String(blueMap));
    display.display();
  }
}

void drawButtonCounters() {
  display.clear();
  // Button Counters
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, 
    "Btns: 0="  + String(buttonCounter[0])
    + ", 1=" + String(buttonCounter[1])
    + ", 2=" + String(buttonCounter[2]));
  display.drawString(0, 20, 
    ", 3=" + String(buttonCounter[3])
    + ", 4=" + String(buttonCounter[4])
    + ", 5=" + String(buttonCounter[5]));
  display.display();
}

void drawTimeOnCounter() {
  display.clear();
  // Time On Counter
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 20, String(millis()));
  display.display();
}

void drawProgressBarDemo() {
  display.clear();
  int progress = (buttonCounter[5] / 5) % 100;
  // draw the progress bar
  display.drawProgressBar(0, 30, 120, 10, progress);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(progress) + "%");
  display.display();
}