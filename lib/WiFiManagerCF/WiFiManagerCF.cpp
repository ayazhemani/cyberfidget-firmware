// WiFiManagerCF.cpp

#include "WiFiManagerCF.h"
#include "globals.h"
#include <WiFi.h>
#include "HAL.h"

// Initialize static instance pointer
WiFiManagerCF* WiFiManagerCF::instance = nullptr;

// Define the global instance for AppManager
WiFiManagerCF wifiManagerCF_App(HAL::buttonManager());  // <--- This matches the extern in .h

WiFiManagerCF::WiFiManagerCF(ButtonManager& btnMgr)
    : server(80),
      isWebServerRunning(false),
      portalRunning(false),
      isTryingToConnect(false),
      connectStartTime(0),
      display(HAL::displayProxy()),
     // currentState(STATE_INITIAL),
      buttonManager(btnMgr) // AppManager Integration
{
    instance = this; // Set the instance pointer
    // If you want to start up in "initial" only once at boot:
    static bool firstStartup = true;
    if (firstStartup) {
        currentState = STATE_INITIAL;
        firstStartup = false;
    }
}

void WiFiManagerCF::init() {
    WiFi.mode(WIFI_STA);
}

void WiFiManagerCF::draw() {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    
    switch (currentState) {
        case STATE_INITIAL:
            display.drawString(64, 2, "WiFi Controls");
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 12, "Bottom Left:");
            display.drawString(0, 22, "Connect/Cancel WiFi");

            display.drawString(0, 32, "Bottom Right:");
            display.drawString(0, 42, "Start/Stop Portal");
            break;

        case STATE_CONNECTING:
            display.drawString(64, 12, "WiFi Connecting...");
            break;

        case STATE_CONNECTED:
            display.drawString(64, 12, "WiFi Connected!");
            break;

        case STATE_CONNECT_FAILED:
            display.drawString(64, 12, "WiFi Connect Failed");
            break;

        case STATE_CONNECTION_CANCELED:
            display.drawString(64, 12, "WiFi Connect Canceled");
            break;

        case STATE_PORTAL_RUNNING:
            display.drawString(64, 12, "WiFi Portal Started");
            display.drawString(64, 22, wifiAP_SSID);
            break;

        case STATE_PORTAL_STOPPED:
            display.drawString(64, 12, "WiFi Portal Stopped");
            break;
    }

    display.display();
}

/**
 * @brief Called every loop; we simply call process() 
 */
void WiFiManagerCF::update() {
    // Process WiFiManager only if the portal is running
    if (portalRunning) {
        wifiManager.process();

        // Check if the portal has been exited
        if (!wifiManager.getConfigPortalActive()) {
            // Portal has exited
            stopWiFiPortal();
            stopWebServer();
            ESP_LOGI(TAG_MAIN, "Config Portal exited");
            currentState = STATE_PORTAL_STOPPED;
        }
    }

    // Handle WiFi connection status
    if (isTryingToConnect) {
        wl_status_t wifiStatus = WiFi.status();
        if (wifiStatus == WL_CONNECTED) {
            isTryingToConnect = false;
            ESP_LOGI(TAG_MAIN, "Connected to WiFi!");
            currentState = STATE_CONNECTED;
        } else if (wifiStatus == WL_CONNECT_FAILED || wifiStatus == WL_NO_SSID_AVAIL || wifiStatus == WL_CONNECTION_LOST) {
            if (millis() - connectStartTime > 10000) { // 10-second timeout
                isTryingToConnect = false;
                ESP_LOGI(TAG_MAIN, "Failed to Connect to WiFi");
                currentState = STATE_CONNECT_FAILED;
            }
        }
        // Else, still trying to connect
    }

    // Update display
    draw();
}

// -----------------------------
// AppManager Integration
// -----------------------------

/**
 * @brief Called when user enters this app
 */
void WiFiManagerCF::begin() {
    // Register the wifi connect/cancel button => bottomLeft
    buttonManager.registerCallback(button_BottomLeftIndex, bottomLeftButtonCallback);
    // Register the portal start/stop button => bottomRight
    buttonManager.registerCallback(button_BottomRightIndex, bottomRightButtonCallback);

    // Register a "back to menu" button callback (pick any free button).
    // For example, use top-left if you have it:
    buttonManager.registerCallback(button_TopLeftIndex, onButtonBackPressed);

    ESP_LOGI(TAG_MAIN, "[WiFiManagerCF] begin() => callbacks registered");
}

/**
 * @brief Called when user exits this app
 */
void WiFiManagerCF::end() {
    // Unregister all callbacks used by this app
    buttonManager.unregisterCallback(button_BottomLeftIndex);
    buttonManager.unregisterCallback(button_BottomRightIndex);
    buttonManager.unregisterCallback(button_TopLeftIndex);

    ESP_LOGI(TAG_MAIN, "[WiFiManagerCF] end() => callbacks unregistered");
}

// -----------------------------
// Button Callbacks
// -----------------------------
void WiFiManagerCF::bottomLeftButtonCallback(const ButtonEvent &ev) {
    if (instance) {
        instance->handleBottomLeftButton(ev);
    }
}

void WiFiManagerCF::bottomRightButtonCallback(const ButtonEvent &ev) {
    if (instance) {
        instance->handleBottomRightButton(ev);
    }
}

/**
 * @brief Static callback for returning to main menu 
 */
void WiFiManagerCF::onButtonBackPressed(const ButtonEvent &ev) {
    // Only act on release so we don't trigger multiple times
    if (ev.eventType == ButtonEvent_Released) {
        ESP_LOGI(TAG_MAIN, "[WiFiManagerCF] Back button pressed => returning to menu...");
        if (instance) {
            instance->end(); // Stop the app
        }
        // Return to main menu
        MenuManager::instance().returnToMenu();
    }
}

// -----------------------------
// Internal Handler Methods
// -----------------------------
void WiFiManagerCF::handleBottomLeftButton(const ButtonEvent &ev) {
    if (ev.eventType == ButtonEvent_Pressed) {
        if (isTryingToConnect) {
            stopWiFiConnection();
            ESP_LOGI(TAG_MAIN, "WiFi Connection Canceled");
            currentState = STATE_CONNECTION_CANCELED;
        } else {
            startWiFiConnection();
            ESP_LOGI(TAG_MAIN, "Attempting WiFi Connection...");
            currentState = STATE_CONNECTING;
        }
    }
}

void WiFiManagerCF::handleBottomRightButton(const ButtonEvent &ev) {
    if (ev.eventType == ButtonEvent_Pressed) {
        if (isPortalRunning()) {
            stopWiFiPortal();
            stopWebServer();
            ESP_LOGI(TAG_MAIN, "WiFi Portal Stopped");
            currentState = STATE_PORTAL_STOPPED;
        } else {
            startWiFiPortal();
            startWebServer();
            ESP_LOGI(TAG_MAIN, "WiFi Portal Started");
            currentState = STATE_PORTAL_RUNNING;
            ESP_LOGI(TAG_MAIN, "[WiFiManagerCF] 1) handleBottomRightButton() => currentState=%d", (int)currentState);
        }
    }
    ESP_LOGI(TAG_MAIN, "[WiFiManagerCF] 2) handleBottomRightButton() => currentState=%d", (int)currentState);

}

// -----------------------------
// Start/Stop WiFi Portal
// -----------------------------
void WiFiManagerCF::startWiFiPortal() {
    if (!portalRunning) {
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setConfigPortalTimeout(0); // No timeout

        // Optionally, set custom strings or callbacks
        wifiManager.setAPCallback([](WiFiManager *wm) {
            ESP_LOGI(TAG_MAIN, "Config Portal Started");
        });

        wifiManager.startConfigPortal(wifiAP_SSID);
        portalRunning = true;
        ESP_LOGI(TAG_MAIN, "Started Config Portal");
    }
}

void WiFiManagerCF::stopWiFiPortal() {
    if (portalRunning) {
        wifiManager.stopConfigPortal();
        portalRunning = false;
        ESP_LOGI(TAG_MAIN, "Stopped Config Portal");
    }
}

bool WiFiManagerCF::isPortalRunning() {
    return portalRunning;
}

// -----------------------------
// WiFi Connection Start/Stop
// -----------------------------
void WiFiManagerCF::startWiFiConnection() {
    if (!isTryingToConnect) {
        isTryingToConnect = true;
        connectStartTime = millis();
        WiFi.mode(WIFI_STA);
        WiFi.begin();  // Begin with stored credentials
    }
}

void WiFiManagerCF::stopWiFiConnection() {
    if (isTryingToConnect) {
        WiFi.disconnect();
        isTryingToConnect = false;
    }
}

bool WiFiManagerCF::isConnecting() {
    return isTryingToConnect;
}

// -----------------------------
// Web Server Start/Stop
// -----------------------------
void WiFiManagerCF::startWebServer() {
    if (!isWebServerRunning) {
        //disableWatchdog();

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", "<h1>Cyber Fidget Config Portal</h1>");
        });

        server.begin();
        isWebServerRunning = true;
        ESP_LOGI(TAG_MAIN, "Web Server Started");
    }
}

void WiFiManagerCF::stopWebServer() {
    if (isWebServerRunning) {
        server.end();
        isWebServerRunning = false;
        ESP_LOGI(TAG_MAIN, "Web Server Stopped");
        //enableWatchdog();
    }
}
