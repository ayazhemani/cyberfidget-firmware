#ifndef WIFI_MANAGER_CF_H
#define WIFI_MANAGER_CF_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>

class WiFiManagerCF {
public:
    WiFiManagerCF();
    void init();
    void process();
    void startWebServer();
    void stopWebServer();
    void startWiFiPortal();

private:
    WiFiManager wifiManager;
    AsyncWebServer server;
    bool isWebServerRunning;
};

extern WiFiManagerCF WiFiManagerCFObject;

#endif  // WIFI_MANAGER_CF_H
