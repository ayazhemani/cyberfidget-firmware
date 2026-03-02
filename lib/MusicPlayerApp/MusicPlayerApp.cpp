#include "MusicPlayerApp.h"
#include <SD.h>
#include <SPI.h>

using namespace audio_tools;

// SD Card SPI Pins - TODO: verify against CyberFidget schematic
static const int PIN_SD_CLK  = 5;
static const int PIN_SD_MISO = 21;
static const int PIN_SD_MOSI = 19;
static const int PIN_SD_CS   = 8;

MusicPlayerApp* MusicPlayerApp::instance = nullptr;
MusicPlayerApp musicPlayerApp(HAL::buttonManager());
static auto& display = HAL::displayProxy();

// -------------------------------------------------------------------------
// Lifecycle
// -------------------------------------------------------------------------
MusicPlayerApp::MusicPlayerApp(ButtonManager& btnMgr)
    : buttonManager(btnMgr),
      a2dpStream(),
      decoder()
{
    instance = this;
    // Audio pipeline NOT initialized here - deferred to initAudioPipeline()
}

void MusicPlayerApp::begin() {
    currentState = STATE_BT_SCAN;
    menuCursorIndex = 0;
    menuScrollOffset = 0;
    isPlaying = false;
    isConnected = false;

    buttonManager.registerCallback(button_UpIndex, onButtonUpPressed);
    buttonManager.registerCallback(button_DownIndex, onButtonDownPressed);
    buttonManager.registerCallback(button_EnterIndex, onButtonSelectPressed);
    buttonManager.registerCallback(button_SelectIndex, onButtonBackPressed);

    scanForDevices();
}

void MusicPlayerApp::end() {
    stopPlayback();
    if (a2dpStream.isConnected()) {
        a2dpStream.end();
    }
    // Clean up heap-allocated audio objects
    if (pPlayer) { delete pPlayer; pPlayer = nullptr; }
    if (pSourceSD) { delete pSourceSD; pSourceSD = nullptr; }
    audioPipelineReady = false;

    buttonManager.unregisterCallback(button_UpIndex);
    buttonManager.unregisterCallback(button_DownIndex);
    buttonManager.unregisterCallback(button_EnterIndex);
    buttonManager.unregisterCallback(button_SelectIndex);
}

void MusicPlayerApp::update() {
    if (audioPipelineReady && pPlayer) {
        updateVolumeFromSlider();
        pPlayer->copy();

        if (currentState == STATE_PLAYER && isPlaying) {
            if (!pPlayer->isActive()) {
                isPlaying = false;
            }
        }
    }

    display.clear();
    switch (currentState) {
        case STATE_BT_SCAN:       renderScanMenu();   break;
        case STATE_BT_CONNECTING: renderConnecting();  break;
        case STATE_FILE_BROWSER:  drawFileBrowser();   break;
        case STATE_PLAYER:        renderPlayer();      break;
    }
    display.display();
}

// -------------------------------------------------------------------------
// Audio Pipeline Init (deferred - called when BT connects)
// -------------------------------------------------------------------------
bool MusicPlayerApp::initAudioPipeline() {
    if (audioPipelineReady) return true;

    // Init SPI for SD card
    SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    if (!SD.begin(PIN_SD_CS)) {
        Serial.println("[MusicPlayer] SD init failed - no card?");
        // Continue anyway - we can still connect BT, just no files to browse
    }

    // Create audio source and player on the heap
    pSourceSD = new AudioSourceIdxSD("/", "mp3", PIN_SD_CS);
    pPlayer = new AudioPlayer(*pSourceSD, a2dpStream, decoder);

    pPlayer->setSilenceOnInactive(true);
    pPlayer->setVolume(0.1);
    pPlayer->setActive(false);

    audioPipelineReady = true;
    return true;
}

// -------------------------------------------------------------------------
// Logic
// -------------------------------------------------------------------------
void MusicPlayerApp::scanForDevices() {
    deviceList.clear();
    // TODO Phase 2: Real BT scanning + NVS remembered devices
    deviceList.push_back({"Scan for devices...", {0}});
    menuCursorIndex = 0;
    menuScrollOffset = 0;
}

void MusicPlayerApp::connectToDevice(int index) {
    if (index >= (int)deviceList.size()) return;
    BTDeviceEntry& dev = deviceList[index];

    currentState = STATE_BT_CONNECTING;
    display.clear(); renderConnecting(); display.display();

    // Init the audio pipeline if not already done
    initAudioPipeline();

    auto cfg = a2dpStream.defaultConfig(TX_MODE);
    cfg.name = (char*)dev.name.c_str();
    cfg.silence_on_nodata = true;

    a2dpStream.begin(cfg);
    isConnected = true;

    menuCursorIndex = 0;
    menuScrollOffset = 0;
    currentState = STATE_FILE_BROWSER;
}

void MusicPlayerApp::drawFileBrowser() {
    drawHeader("Select Music");

    if (!pSourceSD) {
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 30, "No SD card");
        return;
    }

    int totalFiles = pSourceSD->size();

    int yStart = 16; int h = 12; int maxItems = 4;
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    if (totalFiles == 0) {
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 30, "No MP3 files found");
        return;
    }

    for (int i = 0; i < maxItems; i++) {
        int idx = i + menuScrollOffset;
        if (idx >= totalFiles) break;

        int y = yStart + (i * h);
        String displayName = "Track " + String(idx + 1);

        if (idx == menuCursorIndex) {
            display.fillRect(0, y, 128, h);
            display.setColor(BLACK);
            display.drawString(4, y - 1, displayName);
            display.setColor(WHITE);
        } else {
            display.drawString(4, y - 1, displayName);
        }
    }
}

void MusicPlayerApp::playFileAtIndex(int index) {
    if (!pPlayer) return;
    pPlayer->setIndex(index);
    pPlayer->begin();
    isPlaying = true;
    currentState = STATE_PLAYER;
}

void MusicPlayerApp::stopPlayback() {
    isPlaying = false;
    if (pPlayer) pPlayer->stop();
}

void MusicPlayerApp::updateVolumeFromSlider() {
    if (!a2dpStream.isConnected()) return;
    float vol = sliderPosition_Percentage_Filtered / 100.0f;
    static float lastVol = -1.0;
    if (abs(vol - lastVol) > 0.05) {
        a2dpStream.setVolume(vol);
        lastVol = vol;
    }
}

// -------------------------------------------------------------------------
// UI Rendering
// -------------------------------------------------------------------------
void MusicPlayerApp::drawHeader(String title) {
    display.setColor(WHITE);
    display.fillRect(0, 0, 128, 14);
    display.setColor(BLACK);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 1, title);
    display.setColor(WHITE);
}

void MusicPlayerApp::renderScanMenu() {
    drawHeader("Connect Device");
    int yStart = 16; int h = 12; int maxItems = 4;
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    for (int i = 0; i < maxItems; i++) {
        int idx = i + menuScrollOffset;
        if (idx >= (int)deviceList.size()) break;
        int y = yStart + (i * h);
        if (idx == menuCursorIndex) {
            display.fillRect(0, y, 128, h);
            display.setColor(BLACK);
            display.drawString(4, y - 1, deviceList[idx].name);
            display.setColor(WHITE);
        } else {
            display.drawString(4, y - 1, deviceList[idx].name);
        }
    }
}

void MusicPlayerApp::renderConnecting() {
    drawHeader("Bluetooth");
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 30, "Connecting...");
}

void MusicPlayerApp::renderPlayer() {
    drawHeader("Now Playing");
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, isPlaying ? ">" : "||");
    display.drawProgressBar(10, 50, 108, 8, (int)sliderPosition_Percentage_Filtered);
}

// -------------------------------------------------------------------------
// Input Handlers
// -------------------------------------------------------------------------
void MusicPlayerApp::onButtonUpPressed(const ButtonEvent& event) { if (event.eventType == ButtonEvent_Pressed) instance->handleNavUp(); }
void MusicPlayerApp::onButtonDownPressed(const ButtonEvent& event) { if (event.eventType == ButtonEvent_Pressed) instance->handleNavDown(); }
void MusicPlayerApp::onButtonSelectPressed(const ButtonEvent& event) { if (event.eventType == ButtonEvent_Released) instance->handleNavSelect(); }
void MusicPlayerApp::onButtonBackPressed(const ButtonEvent& event) { if (event.eventType == ButtonEvent_Released) instance->handleNavBack(); }

void MusicPlayerApp::handleNavUp() {
    if (currentState == STATE_PLAYER) return;
    if (menuCursorIndex > 0) { menuCursorIndex--; if (menuCursorIndex < menuScrollOffset) menuScrollOffset--; }
}

void MusicPlayerApp::handleNavDown() {
    if (currentState == STATE_PLAYER) return;
    int listSize = (currentState == STATE_BT_SCAN) ? (int)deviceList.size() :
                   (pSourceSD ? pSourceSD->size() : 0);
    if (menuCursorIndex < listSize - 1) { menuCursorIndex++; if (menuCursorIndex >= menuScrollOffset + 4) menuScrollOffset++; }
}

void MusicPlayerApp::handleNavSelect() {
    if (currentState == STATE_BT_SCAN) {
        if (!deviceList.empty()) connectToDevice(menuCursorIndex);
    } else if (currentState == STATE_FILE_BROWSER) {
        playFileAtIndex(menuCursorIndex);
    } else if (currentState == STATE_PLAYER) {
        isPlaying = !isPlaying;
        if (pPlayer) pPlayer->setActive(isPlaying);
    }
}

void MusicPlayerApp::handleNavBack() {
    if (currentState == STATE_PLAYER) { stopPlayback(); currentState = STATE_FILE_BROWSER; }
    else if (currentState == STATE_FILE_BROWSER) { end(); MenuManager::instance().returnToMenu(); }
    else { end(); MenuManager::instance().returnToMenu(); }
}
