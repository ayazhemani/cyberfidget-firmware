#include "SliderPosition.h"
#include "globals.h"

extern void drawSerialDataScreen();
extern const int maxLinesOnScreen;

void updateScrollPositionFromSlider() {
    if (currentScrollMode == LINE_SCROLL) {
        int newLine = map(sliderPosition_12Bits, 0, 4095, 0, max(lineCount - maxLinesOnScreen, 0));
        if (newLine != previousLine) {
            currentLine = newLine;
            drawSerialDataScreen();
            isScreenUpdated = true;
            previousLine = newLine;
        }
    } else if (currentScrollMode == PIXEL_SCROLL) {
        int totalScrollablePixels = max(0, (lineCount * 10) - 64);
        int scrollPosition = map(sliderPosition_12Bits, 0, 4095, 0, totalScrollablePixels);
        int newLine = scrollPosition / 10;
        int newScrollOffset = scrollPosition % 10;
        if (newLine != previousLine || newScrollOffset != previousScrollOffset) {
            currentLine = newLine;
            scrollOffset = newScrollOffset;
            drawSerialDataScreen();
            isScreenUpdated = true;
            previousLine = newLine;
            previousScrollOffset = newScrollOffset;
        }
    }
}

void sliderPositionRead(){
  sliderPosition_Millivolts = analogReadMilliVolts(VOLT_READ_PIN); // Read through ADC with calibrated return
  sliderPosition_12Bits = analogRead(VOLT_READ_PIN); // Read through 12-bit ADC as raw bits into 16-bit var
  sliderPosition_12Bits_Inverted =  4095 - sliderPosition_12Bits;
  
  sliderPosition_8Bits =  255 - ((sliderPosition_12Bits * 255) / 4095); // Map 12-bit to 8-bit variable
  sliderPosition_8Bits_Inverted = (sliderPosition_12Bits * 255) / 4095; // Map 12-bit to 8-bit variable, inverted
    
  sliderPosition_Percentage = 100 - ((sliderPosition_12Bits * 100) / 4095); // Map 12-bit to percentage (0-100), inverted
}
