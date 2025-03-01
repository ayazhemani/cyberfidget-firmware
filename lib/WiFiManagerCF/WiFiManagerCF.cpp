// WiFiManagerCF.cpp (Updated)

#include "WiFiManagerCF.h"
#include "globals.h"
#include <WiFi.h>

// Initialize static instance pointer
WiFiManagerCF* WiFiManagerCF::instance = nullptr;

WiFiManagerCF::WiFiManagerCF()
    : server(80),
      isWebServerRunning(false),
      portalRunning(false),
      isTryingToConnect(false),
      connectStartTime(0),
      display(nullptr),
      currentState(STATE_INITIAL)
{
    instance = this; // Set the instance pointer
}

void WiFiManagerCF::init() {
    WiFi.mode(WIFI_STA);
}

void WiFiManagerCF::setDisplay(SSD1306Wire &disp) {
    display = &disp;
}

void WiFiManagerCF::draw() {
    if (!display) return;

    display->clear();
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_10);

    switch (currentState) {
        case STATE_INITIAL:
            display->drawString(64, 2, "WiFi Controls");
            
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->drawString(0, 12, "Bottom Left:");
            display->drawString(0, 22, "Connect/Cancel WiFi");
            
            display->drawString(0, 32, "Bottom Right:");
            display->drawString(0, 42, "Start/Stop Portal");
            break;

        case STATE_CONNECTING:
            display->drawString(64, 12, "WiFi Connecting...");
            break;

        case STATE_CONNECTED:
            display->drawString(64, 12, "WiFi Connected!");
            break;

        case STATE_CONNECT_FAILED:
            display->drawString(64, 12, "WiFi Connect Failed");
            break;

        case STATE_CONNECTION_CANCELED:
            display->drawString(64, 12, "WiFi Connect Canceled");
            break;

        case STATE_PORTAL_RUNNING:
            display->drawString(64, 12, "WiFi Portal Started");
            display->drawString(64, 22, wifiAP_SSID);
            break;

        case STATE_PORTAL_STOPPED:
            display->drawString(64, 12, "WiFi Portal Stopped");
            break;
    }

    display->display();
}

void WiFiManagerCF::process() {
    // Process WiFiManager only if the portal is running
    if (portalRunning) {
        wifiManager.process();

        // Check if the portal has been exited
        if (!wifiManager.getConfigPortalActive()) {
            // Portal has exited
            stopWiFiPortal();
            stopWebServer();
            Serial.println("Config Portal exited");
            currentState = STATE_PORTAL_STOPPED;
        }
    }

    // Handle WiFi connection status
    if (isTryingToConnect) {
        wl_status_t wifiStatus = WiFi.status();
        if (wifiStatus == WL_CONNECTED) {
            isTryingToConnect = false;
            Serial.println("Connected to WiFi!");
            currentState = STATE_CONNECTED;
        } else if (wifiStatus == WL_CONNECT_FAILED || wifiStatus == WL_NO_SSID_AVAIL || wifiStatus == WL_CONNECTION_LOST) {
            if (millis() - connectStartTime > 10000) { // 10-second timeout
                isTryingToConnect = false;
                Serial.println("Failed to Connect to WiFi");
                currentState = STATE_CONNECT_FAILED;
            }
        }
        // Else, still trying to connect
    }

    // Update display
    draw();
}

// Static button callback methods

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

void WiFiManagerCF::handleBottomLeftButton(const ButtonEvent &ev) {
    if (ev.eventType == ButtonEvent_Pressed) {
        if (isTryingToConnect) {
            stopWiFiConnection();
            Serial.println("WiFi Connection Canceled");
            currentState = STATE_CONNECTION_CANCELED;
        } else {
            startWiFiConnection();
            Serial.println("Attempting WiFi Connection...");
            currentState = STATE_CONNECTING;
        }
    }
}

void WiFiManagerCF::handleBottomRightButton(const ButtonEvent &ev) {
    if (ev.eventType == ButtonEvent_Pressed) {
        if (isPortalRunning()) {
            stopWiFiPortal();
            stopWebServer();
            Serial.println("WiFi Portal Stopped");
            currentState = STATE_PORTAL_STOPPED;
        } else {
            startWiFiPortal();
            startWebServer();
            Serial.println("WiFi Portal Started");
            currentState = STATE_PORTAL_RUNNING;
        }
    }
}

// Start and stop methods

void WiFiManagerCF::startWiFiPortal() {
    if (!portalRunning) {
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setConfigPortalTimeout(0); // No timeout

        // Optionally, set custom strings or callbacks
        wifiManager.setAPCallback([](WiFiManager *wm) {
            Serial.println("Config Portal Started");
        });

        wifiManager.startConfigPortal(wifiAP_SSID);
        portalRunning = true;
        Serial.println("Started Config Portal");
    }
}

void WiFiManagerCF::stopWiFiPortal() {
    if (portalRunning) {
        wifiManager.stopConfigPortal();
        portalRunning = false;
        Serial.println("Stopped Config Portal");
    }
}

bool WiFiManagerCF::isPortalRunning() {
    return portalRunning;
}

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

void WiFiManagerCF::startWebServer() {
    if (!isWebServerRunning) {
        //disableWatchdog();

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", "<h1>Cyber Fidget Config Portal</h1>");
        });

        server.begin();
        isWebServerRunning = true;
        Serial.println("Web Server Started");
    }
}

void WiFiManagerCF::stopWebServer() {
    if (isWebServerRunning) {
        server.end();
        isWebServerRunning = false;
        Serial.println("Web Server Stopped");
        //enableWatchdog();
    }
}