#include "SerialDisplay.h"
#include "globals.h"
#include "SSD1306Wire.h"

extern SSD1306Wire display;
extern String incomingData;
extern unsigned long lastDataTime;
extern const unsigned long dataTimeout;
extern bool newDataReceived;
extern int lineCount;
extern int currentLine;
extern bool isScreenUpdated;
extern const int maxLinesOnScreen;
extern ScrollMode currentScrollMode;
extern int previousLine;
extern int scrollOffset;
extern int previousScrollOffset;
extern String dataLines[];

int maxScrollOffset =  0;            // Maximum pixel offset for scrolling
const int lineHeight = 13;            // Height of each line in pixels 13px = AerialMT_Plain_10
const int displayHeight = 64;         // Height of your display in pixel

extern void drawSerialDataScreen();
extern void drawDefaultInfoScreen();

void drawSerialDataScreenWrapper() {
    newDataReceived = false;

    while (Serial.available() > 0) {
        char incomingChar = Serial.read();
        newDataReceived = true;

        if (incomingChar == '\n') {
            String parsedData[10];
            int parsedLineCount = 0;
            int startIndex = 0;
            int commaIndex;
            while ((commaIndex = incomingData.indexOf(",", startIndex)) != -1 && parsedLineCount < 10) {
                String segment = incomingData.substring(startIndex, commaIndex);
                segment.trim();
                parsedData[parsedLineCount++] = segment;
                startIndex = commaIndex + 1;
            }
            String lastSegment = incomingData.substring(startIndex);
            lastSegment.trim();
            parsedData[parsedLineCount++] = lastSegment; 

            // Always update dataLines
            lineCount = parsedLineCount;
            for (int i = 0; i < lineCount; i++) {
                dataLines[i] = parsedData[i];
            }
            // After updating lineCount:
            maxScrollOffset = max(0, lineCount * lineHeight - displayHeight);
            if (maxScrollOffset < 0) {
                maxScrollOffset = 0;  // If fewer lines than fit on screen, no need to scroll
            }

            Serial.printf("Parsed lineCount: %d\n", lineCount);
            for (int i = 0; i < lineCount; i++) {
                Serial.printf("Line %d: %s\n", i, dataLines[i].c_str());
            }

            // Instead of resetting currentLine and scrollOffset, preserve them:
            //currentLine = 0;
            //scrollOffset = 0;

            drawSerialDataScreen();
            isScreenUpdated = true;
            incomingData = "";
            lastDataTime = millis();
            Serial.println(", Data processed.");
        } else {
            // Append non-newline characters to incomingData
            incomingData += incomingChar;
            Serial.print(incomingChar);  // Debug print to show character accumulation
        }
    }

    if (millis() - lastDataTime > dataTimeout && !newDataReceived && lineCount == 0) {
        drawDefaultInfoScreen();
        isScreenUpdated = true;
    }
}

void handleScrollUp() {
  if (currentScrollMode == LINE_SCROLL) {
    // Line-based scrolling
    if (currentLine > 0) {
      currentLine--;
      drawSerialDataScreen();
    }
  } else if (currentScrollMode == PIXEL_SCROLL) {
    // Pixel-based scrolling
    if (scrollOffset > 0) {
        scrollOffset--;
        drawSerialDataScreen();
    }
  }
}

void handleScrollDown() {
    if (currentScrollMode == LINE_SCROLL) {
        // Line-based scrolling
        if (currentLine < lineCount - maxLinesOnScreen) {
            currentLine++;
            drawSerialDataScreen();
        }
    } else if (currentScrollMode == PIXEL_SCROLL) {
        // Pixel-based scrolling
        if (scrollOffset < maxScrollOffset) {
            scrollOffset++;
            drawSerialDataScreen();
        }
    }
}

void toggleScrollMode() {
    if (currentScrollMode == LINE_SCROLL) {
        currentScrollMode = PIXEL_SCROLL;
    } else {
        currentScrollMode = LINE_SCROLL;
        scrollOffset = 0; // Reset pixel offset when switching to line scroll mode
    }
    drawSerialDataScreen(); // Update screen to reflect new scroll mode
}

