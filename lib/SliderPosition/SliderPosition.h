#ifndef SLIDERPOSITION_H
#define SLIDERPOSITION_H

// Function declarations for filter initialization and update
void sliderPositionFilterInit();
void sliderPositionFilterUpdate();

// Function declarations for reading and Kalman filter operations
void sliderPositionRead();
void sliderPositionKalmanFilterInit();

// Utility function for clamping and integer conversion
int clampAndConvertToInt(float value, int minVal, int maxVal);
void applyRateLimit(float &currentValue, float newValue, float rateLimit);

#endif
