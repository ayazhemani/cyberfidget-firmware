#ifndef SERIALDISPLAY_H
#define SERIALDISPLAY_H

extern const int lineHeight;
extern const int displayHeight;
extern int maxScrollOffset;

void drawSerialDataScreenWrapper();
void handleScrollUp();
void handleScrollDown();
void toggleScrollMode();

#endif
