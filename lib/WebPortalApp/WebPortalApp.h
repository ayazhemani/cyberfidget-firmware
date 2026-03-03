#ifndef WEB_PORTAL_APP_H
#define WEB_PORTAL_APP_H

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

#include "HAL.h"
#include "AppDefs.h"
#include "ButtonManager.h"

class WebPortalApp {
public:
    WebPortalApp(ButtonManager& btnMgr);

    void begin();
    void update();
    void end();

    static void onButtonBack(const ButtonEvent& event);

private:
    static WebPortalApp* instance;
    ButtonManager& buttonManager;

    static constexpr const char* AP_SSID = "CyberFidget";

    // Web server + DNS (heap-allocated server for clean lifecycle)
    AsyncWebServer* server = nullptr;
    DNSServer dnsServer;

    // SD state
    bool sdReady = false;
    int fileCount = 0;

    // Upload tracking (written by async handler, read by render loop)
    volatile bool uploadInProgress = false;
    volatile size_t uploadBytesReceived = 0;
    volatile size_t uploadBytesTotal = 0;
    File uploadFile;

    // SD helpers
    void initSD();
    void countFilesRecursive(const char* dir);
    void invalidateMusicCache();

    // Web server setup
    void setupRoutes();

    // Route handlers
    void handleFileList(AsyncWebServerRequest* req);
    void handleUpload(AsyncWebServerRequest* req, const String& filename,
                      size_t index, uint8_t* data, size_t len, bool final);
    void handleDelete(AsyncWebServerRequest* req);
    void handleMkdir(AsyncWebServerRequest* req);
    void handleMove(AsyncWebServerRequest* req);
    void handleTrackList(AsyncWebServerRequest* req);
    void handleStatus(AsyncWebServerRequest* req);
    void handlePlaylistList(AsyncWebServerRequest* req);
    void handlePlaylistGet(AsyncWebServerRequest* req);
    void handlePlaylistSave(AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total);
    void handlePlaylistDelete(AsyncWebServerRequest* req);

    // OLED rendering
    void render();
};

extern WebPortalApp webPortalApp;

#endif // WEB_PORTAL_APP_H
