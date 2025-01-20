#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

class BatteryManager {
public:
    BatteryManager();
    void init();
    void update();
    void debug();
private:
    SFE_MAX1704X lipo;
};

extern BatteryManager batteryManager;

#endif // BATTERY_MANAGER_H
