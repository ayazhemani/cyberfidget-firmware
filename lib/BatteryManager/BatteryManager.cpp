#include "BatteryManager.h"
#include "globals.h"
#include "HAL.h"

BatteryManager::BatteryManager() : lipo(MAX1704X_MAX17048) {}

BatteryManager batteryManager;

void BatteryManager::init() {
    lipo.enableDebugging();
    if (!lipo.begin()) {
        Serial.println(F("MAX17048 not detected. Please check wiring. Freezing."));
        while(1);
    }

    // Read and print the device ID
    Serial.print(F("Device ID: 0x"));
    uint8_t id = lipo.getID(); // Read the device ID
    if (id < 0x10) Serial.print(F("0")); // Print the leading zero if required
    Serial.println(id, HEX); // Print the ID as hexadecimal

    // Read and print the device version
    Serial.print(F("Device version: 0x"));
    uint8_t ver = lipo.getVersion(); // Read the device version
    if (ver < 0x10) Serial.print(F("0")); // Print the leading zero if required
    Serial.println(ver, HEX); // Print the version as hexadecimal

    Serial.print(F("Reset Indicator was: ")); // Read and print the reset indicator
    bool RI = lipo.isReset(true); // Read the RI flag and clear it automatically if it is set
    Serial.println(RI); // If RI was set, check it is now clear
    if (RI) {
        Serial.print(F("Reset Indicator is now: "));
        RI = lipo.isReset();
        Serial.println(RI);
    }
      
    // To quick-start or not to quick-start? That is the question!
    // Read the following and then decide if you do want to quick-start the fuel gauge.
    // "Most systems should not use quick-start because the ICs handle most startup problems transparently,
    //  such as intermittent battery-terminal connection during insertion. If battery voltage stabilizes
    //  faster than 17ms then do not use quick-start. The quick-start command restarts fuel-gauge calculations
    //  in the same manner as initial power-up of the IC. If the system power-up sequence is so noisy that the
    //  initial estimate of SOC has unacceptable error, the system microcontroller might be able to reduce the
    //  error by using quick-start."
    // If you still want to try a quick-start then uncomment the next line:
	//lipo.quickStart();

    // Set alert thresholds and print statuses
    // We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% and 32%:
    lipo.setThreshold(20); // Set alert threshold to 20%.
    Serial.print(F("Battery empty threshold is now: ")); // Read and print the battery empty threshold
    Serial.print(lipo.getThreshold());
    Serial.println(F("%"));

    float highVoltage = ((float)lipo.getVALRTMax()) * 0.02; // 1 LSb is 20mV. Convert to Volts.
    Serial.print(F("High voltage threshold is currently: ")); // Read and print the high voltage threshold
    Serial.print(highVoltage, 2);
    Serial.println(F("V"));

    lipo.setVALRTMax((float)4.1); // Set the high voltage threshold
    Serial.print(F("High voltage threshold is now: "));
    highVoltage = ((float)lipo.getVALRTMax()) * 0.02; // 1 LSb is 20mV. Convert to Volts.
    Serial.print(highVoltage, 2);
    Serial.println(F("V"));

    float lowVoltage = ((float)lipo.getVALRTMin()) * 0.02;
    Serial.print(F("Low voltage threshold is currently: "));
    Serial.print(lowVoltage, 2);
    Serial.println(F("V"));

    lipo.setVALRTMin((float)3.9); // Set the low voltage threshold
    Serial.print(F("Low voltage threshold is now: "));  // Read and print the low voltage threshold
    lowVoltage = ((float)lipo.getVALRTMin()) * 0.02; // 1 LSb is 20mV. Convert to Volts.
    Serial.print(lowVoltage, 2);
    Serial.println(F("V"));

    if(lipo.enableSOCAlert()) { // Enable the State Of Change alert
        Serial.println(F("Enabling the 1% State Of Change alert: success."));
    } else {
        Serial.println(F("Enabling the 1% State Of Change alert: FAILED!"));
    }

    float actThr = ((float)lipo.getHIBRTActThr()) * 0.00125; // Read and print the HIBRT Active Threshold
    Serial.print(F("Hibernate active threshold is: "));
    Serial.print(actThr, 5);
    Serial.println(F("V"));

    float hibThr = ((float)lipo.getHIBRTHibThr()) * 0.208; // Read and print the HIBRT Hibernate Threshold
    Serial.print(F("Hibernate hibernate threshold is: "));
    Serial.print(hibThr, 3);
    Serial.println(F("%/h"));
}

void BatteryManager::update() {
    // Update global battery status variables
    batteryVoltagePercentage = lipo.getSOC();
    batteryVoltage = lipo.getVoltage();
    batteryChangeRate = lipo.getChangeRate();

    // Manage LiPo charger based on SOC and change rate
    // Requires Jumper on R64 to be soldered
    // RED LED on front stays on when charging is blocked
    if(enableBatterySOCCutoff) {
      if ((batteryVoltagePercentage > batterySOCCutoff) && (lipo.getChangeRate() > sleepChargingChangeThreshold)) {
        HAL::chargingDisable();
      } else {
        HAL::chargingEnable();
      }
    }
}

void BatteryManager::debug() {
    // Print the battery status
    Serial.print(F("Voltage: "));
    Serial.print(batteryVoltage);  // Print the battery voltage
    Serial.print(F("V"));

    Serial.print(F(" Percentage: "));
    Serial.print(batteryVoltagePercentage, 2); // Print the battery state of charge with 2 decimal places
    Serial.print(F("%"));

    Serial.print(F(" Change Rate: "));
    Serial.print(batteryChangeRate, 2); // Print the battery change rate with 2 decimal places
    Serial.print(F("%/hr"));

    Serial.print(F(" Alert: "));
    Serial.print(lipo.getAlert()); // Print the generic alert flag

    Serial.print(F(" Voltage High Alert: "));
    Serial.print(lipo.isVoltageHigh()); // Print the alert flag

    Serial.print(F(" Voltage Low Alert: "));
    Serial.print(lipo.isVoltageLow()); // Print the alert flag

    Serial.print(F(" Empty Alert: "));
    Serial.print(lipo.isLow()); // Print the alert flag

    Serial.print(F(" SOC 1% Change Alert: "));
    Serial.print(lipo.isChange()); // Print the alert flag
    
    Serial.print(F(" Hibernating: "));
    Serial.print(lipo.isHibernating()); // Print the alert flag
    
    Serial.println();
}
