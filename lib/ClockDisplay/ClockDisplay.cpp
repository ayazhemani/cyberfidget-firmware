#include "ClockDisplay.h"
#include "globals.h"        // For global variables like clockScreenEnabled, currentTime
#include <time.h>
#include "SSD1306Wire.h"    // For display object

extern SSD1306Wire display;
extern bool clockScreenEnabled;
extern struct tm currentTime;
extern void connectToWiFi();

void drawClockDemo() {
  if (!clockScreenEnabled) {
    clockScreenEnabled = true;
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(suiGenerisRg_20);
    display.drawString(64, 22, "--:--:--");
    display.display();
    
    connectToWiFi();
    configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org", "time.nist.gov");
    
    for (int i = 0; i < 5; i++) {
      if (getLocalTime(&currentTime)) {
        Serial.println("Successfully obtained time.");
        break;
      } else {
        Serial.println("Failed to obtain time, retrying...");
        delay(1000);
      }
    }
  }

  if (getLocalTime(&currentTime)) {
    char timeString[9];
    strftime(timeString, sizeof(timeString), "%I:%M:%S", &currentTime);

    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(suiGenerisRg_20);
    display.drawString(64, 22, String(timeString));
    display.display();
  }
}

// Function to set the time from structure
void updateTime(struct tm *currentTime) {
  time_t now = mktime(currentTime);
  timeval epoch = { .tv_sec = now, .tv_usec = 0 };
  settimeofday(&epoch, NULL);
}

// Increase time by 1 minute
void increaseMinute(struct tm *currentTime) {
  currentTime->tm_min += 1;
  updateTime(currentTime);
}

// Decrease time by 1 minute
void decreaseMinute(struct tm *currentTime) {
  currentTime->tm_min -= 1;
  updateTime(currentTime);
}