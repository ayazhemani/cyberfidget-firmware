#ifndef WIFI_MANAGER_CF_H
#define WIFI_MANAGER_CF_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include "ButtonManager.h"
#include <SSD1306Wire.h>

class WiFiManagerCF {
public:
    WiFiManagerCF();

    void init();
    void process();
    void draw();  // Add this method

    // Button callback methods
    static void bottomLeftButtonCallback(const ButtonEvent &ev);
    static void bottomRightButtonCallback(const ButtonEvent &ev);

    // Methods to handle WiFi operations
    void startWiFiPortal();
    void stopWiFiPortal();
    bool isPortalRunning();
    void stopWebServer();
    void startWebServer();

    void startWiFiConnection();
    void stopWiFiConnection();
    bool isConnecting();

    void setDisplay(SSD1306Wire &disp);

private:
    WiFiManager wifiManager;
    AsyncWebServer server;
    bool isWebServerRunning;
    bool portalRunning;
    bool isTryingToConnect;
    unsigned long connectStartTime;

    // Display reference
    SSD1306Wire *display;

    // Internal state machine
    enum WiFiState {
        STATE_INITIAL,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_CONNECT_FAILED,
        STATE_PORTAL_RUNNING,
        STATE_PORTAL_STOPPED,
        STATE_CONNECTION_CANCELED
    } currentState;

    void handleBottomLeftButton(const ButtonEvent &ev);
    void handleBottomRightButton(const ButtonEvent &ev);

    // Singleton instance pointer for static callbacks
    static WiFiManagerCF* instance;
};

#endif  // WIFI_MANAGER_CF_H