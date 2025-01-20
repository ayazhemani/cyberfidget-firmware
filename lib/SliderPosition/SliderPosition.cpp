#include "SliderPosition.h"
#include "globals.h"

void sliderPositionRead(){
  sliderPosition_Millivolts = analogReadMilliVolts(VOLT_READ_PIN); // Read through ADC with calibrated return
  sliderPosition_12Bits = analogRead(VOLT_READ_PIN); // Read through 12-bit ADC as raw bits into 16-bit var
  sliderPosition_12Bits_Inverted =  4095 - sliderPosition_12Bits;
  
  sliderPosition_8Bits =  255 - ((sliderPosition_12Bits * 255) / 4095); // Map 12-bit to 8-bit variable
  sliderPosition_8Bits_Inverted = (sliderPosition_12Bits * 255) / 4095; // Map 12-bit to 8-bit variable, inverted
    
  sliderPosition_Percentage = 100 - ((sliderPosition_12Bits * 100) / 4095); // Map 12-bit to percentage (0-100), inverted
}
