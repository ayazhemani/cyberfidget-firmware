#include "WiFiManagerCF.h"
#include "globals.h"  // For global variables like wifiAP_SSID, wifimanager_nonblocking, disableWatchdog(), enableWatchdog()

WiFiManagerCF WiFiManagerCFObject;

WiFiManagerCF::WiFiManagerCF() 
  : server(80), isWebServerRunning(false) {
}

void WiFiManagerCF::init() {
    WiFi.mode(WIFI_STA);
}

void WiFiManagerCF::process() {
    if (wifimanager_nonblocking) {
        wifiManager.process();
    }
}

void WiFiManagerCF::startWebServer() {
    if (!isWebServerRunning) {
        disableWatchdog();
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", "<h1>Cyber Fidget Config Portal</h1>");
        });

        wifiManager.setConfigPortalTimeout(60);
        wifiManager.setConfigPortalBlocking(wifimanager_nonblocking);
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
        enableWatchdog();
    }
}

void WiFiManagerCF::startWiFiPortal() {
    if (!wifiManager.startConfigPortal(wifiAP_SSID)) {
         Serial.println("Failed to connect to WiFi");
    } else {
         Serial.println("WiFi Connected");
         startWebServer();
    }
}
