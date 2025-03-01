#include "SliderPosition.h"
#include "globals.h"

/*
 * Kalman Filter Tuning Guidelines:
 *
 * The Kalman filter is tunable through key parameters that influence its performance:
 *
 * 1. Process Noise Covariance (`Q`):
 *    - Represents expected variance in the system model.
 *    - Higher values suggest less trust in the model prediction, increasing reliance on measurements.
 *    - Start with smaller values for predictable systems; increase if the model is too sluggish.
 *
 * 2. Measurement Noise Covariance (`R`):
 *    - Represents expected measurement noise variance.
 *    - Higher values indicate less reliance on measurements, more on predicted states.
 *    - Low values may cause sensitivity to measurement noise, resulting in noisy outputs.
 *
 * 3. Initial Error Covariance (`P`):
 *    - Reflects initial confidence in state estimates.
 *    - Larger values suggest less certainty, allowing quicker initial adjustments.
 *    - `P` will adapt over time; initial settings impact early filter behavior.
 *
 * Tuning Process:
 * - Start with assumed values for `Q`, `R`, and `P`.
 * - Test and observe performance; look for overshoot, sluggishness, or noise.
 * - Adjust: Increase `Q` for prompt response; increase `R` for reducing noise.
 * - Balance: Aim for smoothness and responsiveness. Iterate under real operating conditions.
 */

// Define the Kalman filter state and variables
struct KalmanState {
  float position;
  float velocity;
  float P[2][2]; // Error covariance matrix
};

// Kalman filter parameters
struct KalmanParameters {
  float Q[2][2]; // Process noise covariance
  float R;       // Measurement noise covariance
  float P[2][2]; // Initial error covariance
};

// Define KalmanState for the primary 12-bit signal
KalmanState kalmanState_12Bits;

// Initialize the Kalman filter with parameters
void sliderPositionKalmanFilterInit() {
  KalmanParameters params_12Bits = {
      {{0.1f, 0.01f}, {0.01f, 0.1f}}, // Q
      0.5f,                            // R
      {{1.0f, 0.0f}, {0.0f, 1.0f}}     // Initial P
  };

  kalmanState_12Bits.position = 0.0f;
  kalmanState_12Bits.velocity = 0.0f;
  memcpy(kalmanState_12Bits.P, params_12Bits.P, sizeof(kalmanState_12Bits.P));
}

// Initialize the filtered values
void sliderPositionFilterInit() {
  sliderPosition_12Bits_Filtered = 0.0f;
  sliderPosition_12Bits_Inverted_Filtered = 0.0f;
  sliderPosition_8Bits_Filtered = 0.0f;
  sliderPosition_8Bits_Inverted_Filtered = 0.0f;
  sliderPosition_Percentage_Filtered = 0.0f;

  sliderPositionKalmanFilterInit();
}

// Generic Kalman filter update function
void updateKalmanFilter(KalmanState &state, float measurement, float &filteredOutput, const KalmanParameters &params) {
  const float deltaTime = 0.02f; // assuming 20ms task cycle

  float predicted_position = state.position + state.velocity * deltaTime;
  float predicted_velocity = state.velocity;

  state.P[0][0] += deltaTime * (2 * state.P[0][1] + deltaTime * state.P[1][1]) + params.Q[0][0];
  state.P[0][1] += deltaTime * state.P[1][1];
  state.P[1][0] += deltaTime * state.P[1][1];
  state.P[1][1] += params.Q[1][1];

  float S = state.P[0][0] + params.R;
  float K[2];
  K[0] = state.P[0][0] / S;
  K[1] = state.P[1][0] / S;

  float y = measurement - predicted_position;

  state.position = predicted_position + K[0] * y;
  state.velocity = predicted_velocity + K[1] * y;

  state.P[0][0] -= K[0] * state.P[0][0];
  state.P[0][1] -= K[0] * state.P[0][1];
  state.P[1][0] -= K[1] * state.P[0][0];
  state.P[1][1] -= K[1] * state.P[0][1];

  // Output the filtered position
  filteredOutput = state.position;
}

// Clamp and convert to integer
int clampAndConvertToInt(float value, int minVal, int maxVal) {
  if (value < minVal) value = minVal;
  if (value > maxVal) value = maxVal;
  return static_cast<int>(value);
}

void applyRateLimit(float &currentValue, float newValue, float rateLimit) {
  if (abs(newValue - currentValue) > rateLimit) {
      currentValue += copysign(rateLimit, newValue - currentValue);
  } else {
      currentValue = newValue;
  }
}

void sliderPositionRead() {
  sliderPosition_Millivolts = analogReadMilliVolts(VOLT_READ_PIN);
  sliderPosition_12Bits = analogRead(VOLT_READ_PIN);

  float filtered_12Bits;

  // Define Kalman parameters
  KalmanParameters params_12Bits = {
      {{0.05f, 0.01f}, {0.01f, 0.05f}}, // Q
      50.0f,                            // R - Increase this value to reduce noise
      {{2.0f, 0.0f}, {0.0f, 2.0f}}     // Initial P
  };

  // Apply the Kalman filter to the primary 12-bit variable
  updateKalmanFilter(kalmanState_12Bits, sliderPosition_12Bits, filtered_12Bits, params_12Bits);

  // Apply rate limiting to the filtered value
  //applyRateLimit(sliderPosition_12Bits_Filtered, sliderPosition_12Bits_Filtered, 30);

  // Clamp and convert outputs to integers
  sliderPosition_12Bits_Filtered = clampAndConvertToInt(filtered_12Bits, 0, 4095);
  
  // Derive the filtered signals from the filtered 12-bit signal
  sliderPosition_8Bits_Filtered = clampAndConvertToInt((filtered_12Bits * 255) / 4095, 0, 255);
  sliderPosition_8Bits_Inverted_Filtered = clampAndConvertToInt(255 - ((filtered_12Bits * 255) / 4095), 0, 255);

  // Both normal and inverted percentages
  sliderPosition_Percentage_Filtered = clampAndConvertToInt((filtered_12Bits * 100) / 4095, 0, 100);
  sliderPosition_Percentage_Inverted_Filtered = clampAndConvertToInt(100 - sliderPosition_Percentage_Filtered, 0, 100);

  // Optionally apply EMA filter updates as needed
  //sliderPositionFilterUpdate();
}

void sliderPositionFilterUpdate() {
  // Smoothing factor alpha (range 0.0 - 1.0), increase for faster response
  const float alpha = 1.0f;

  // Apply EMA filter to each reading
  sliderPosition_12Bits_Filtered = alpha * sliderPosition_12Bits + (1.0f - alpha) * sliderPosition_12Bits_Filtered;
  sliderPosition_8Bits_Filtered = alpha * sliderPosition_8Bits + (1.0f - alpha) * sliderPosition_8Bits_Filtered;
  sliderPosition_8Bits_Inverted_Filtered = alpha * sliderPosition_8Bits_Inverted + (1.0f - alpha) * sliderPosition_8Bits_Inverted_Filtered;
  sliderPosition_Percentage_Filtered = alpha * sliderPosition_Percentage + (1.0f - alpha) * sliderPosition_Percentage_Filtered;
}