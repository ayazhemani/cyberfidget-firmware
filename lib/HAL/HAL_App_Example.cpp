// // AppManager.cpp

// #include "HAL.h"
// #include "AppManager.h"

// void AppManagerSetup()
// {
//     // Instead of a big chain of pinMode(...) calls here,
//     // you just say:
//     HAL::initAll();

//     // The rest of your normal app setup
//     // ...
// }

// void AppManagerLoop()
// {
//     // Example: read slider
//     float sliderVal = HAL::readSliderPercentage();

//     // Example: read accel
//     float ax = HAL::getAccelerometerX();
//     float ay = HAL::getAccelerometerY();

//     // If you want to show text
//     HAL::clearDisplay();
//     HAL::drawString(0, 0, String("Slider: ") + sliderVal);
//     HAL::drawString(0, 10, String("ax: ") + ax);
//     HAL::updateDisplay();

//     // ...
// }
