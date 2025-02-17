#ifndef SERIALDISPLAY_H
#define SERIALDISPLAY_H

extern const int lineHeight;
extern const int displayHeight;
extern int maxScrollOffset;

void serialDataScreenProcessor();
void handleScrollUp();
void handleScrollDown();
void toggleScrollMode();
void updateScrollPositionFromSlider();
void drawSerialDataScreen();
void drawDefaultInfoScreen();

#endif
