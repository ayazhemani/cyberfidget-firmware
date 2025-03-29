#ifndef WIFI_MANAGER_CF_H
#define WIFI_MANAGER_CF_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include "ButtonManager.h"
#include <DisplayProxy.h>
#include "MenuManager.h" // For returning to menu

class WiFiManagerCF {
public:
    WiFiManagerCF(ButtonManager& buttonManager);

    void init();
    void draw();  // Draws the current WiFi status on the display
    
    /**
     * @brief AppManager integration:
     *        Called when user enters this app
     */
    void begin();

    /**
     * @brief AppManager integration:
     *        Called when user exits this app
     */
    void end();

    /**
     * @brief AppManager integration:
     *        Called every loop; calls process() internally
     */
    void update();

    // Button callback methods
    static void bottomLeftButtonCallback(const ButtonEvent &ev);
    static void bottomRightButtonCallback(const ButtonEvent &ev);

    /**
     * @brief Static callback for returning to main menu (the 'back' button)
     */
    static void onButtonBackPressed(const ButtonEvent &ev);

    // Methods to handle WiFi operations
    void startWiFiPortal();
    void stopWiFiPortal();
    bool isPortalRunning();
    void stopWebServer();
    void startWebServer();

    void startWiFiConnection();
    void stopWiFiConnection();
    bool isConnecting();

private:
    ButtonManager& buttonManager;

    WiFiManager wifiManager;
    AsyncWebServer server;
    bool isWebServerRunning;
    bool portalRunning;
    bool isTryingToConnect;
    unsigned long connectStartTime;

    // Display reference
    DisplayProxy& display;

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

// ------------------------------------------------------------------
// Global instance for AppManager integration
// ------------------------------------------------------------------
extern WiFiManagerCF wifiManagerCF_App;

#endif  // WIFI_MANAGER_CF_H
