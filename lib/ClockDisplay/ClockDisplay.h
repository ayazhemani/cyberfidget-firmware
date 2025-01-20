#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

void drawClockDemo();
void updateTime(struct tm *currentTime);
void increaseMinute(struct tm *currentTime);
void decreaseMinute(struct tm *currentTime);

#endif
