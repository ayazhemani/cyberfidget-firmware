#include "MusicPlayerApp.h"
#include "globals.h"        // millis_APP_LASTINTERACTION (sleep prevention)
#include <SD.h>
#include <SPI.h>
#include <esp_a2dp_api.h>  // esp_a2d_source_disconnect() — the correct API for source mode

using namespace audio_tools;

// ---------------------------------------------------------------------------
// Debug logging — set to 1 to enable verbose serial output for BT lifecycle,
// audio pipeline, and state machine transitions. Invaluable for diagnosing
// the many edge cases in ESP32-A2DP + audio-tools interaction.
//
// Key findings documented here for future developers:
//
// ESP32-A2DP Library Constraints:
//   - BluetoothA2DPSource's internal FreeRTOS task (BtAppT) and heartbeat
//     timer are never shut down. The global `self_BluetoothA2DPSource` pointer
//     is never cleared. Deleting A2DPStream after playback causes crashes.
//   - The heartbeat timer fires every ~10s. In UNCONNECTED state, it
//     unconditionally calls esp_a2d_connect(peer_bd_addr). The library's
//     set_auto_reconnect(false) flag is NOT checked by this handler.
//     We patched BluetoothA2DPSource.cpp to add the missing guard.
//   - BluetoothA2DPCommon::disconnect() calls esp_a2d_sink_disconnect() —
//     the WRONG API for source mode. Use esp_a2d_source_disconnect() directly.
//   - A2DPStream, AudioPlayer, AudioSourceIdxSD, and the decoder must all
//     be kept alive across app enter/exit cycles. Recreating any of them
//     after active playback causes crashes due to intertwined internal state.
//
// A2DPStream BufferRTOS Blocking:
//   - The shared a2dp_buffer uses portMAX_DELAY for both read and write.
//     When is_a2dp_active is true and the buffer is empty (player stopped),
//     the data callback blocks indefinitely on xStreamBufferReceive(),
//     tying up the BTC task. Since esp_a2d_source_disconnect() dispatches
//     through the same BTC task, it hangs.
//     Fix: temporarily reduce readMaxWait to 50ms (so the callback returns
//     quickly on empty buffer), write silence to wake any currently-pending
//     read, then disconnect. Restore portMAX_DELAY on reconnect.
//     Do NOT use clear() — it sets is_a2dp_active=false, which stops the
//     callback from draining the buffer, causing writes to block on reconnect.
//
// AudioPlayer::stop() / setActive(false):
//   - When auto-fade is enabled (default), stop() writes 2KB+ of silence
//     to the output stream (A2DPStream). After multiple BT disconnect/
//     reconnect cycles, these writes hang due to accumulated BT stack state
//     corruption. Fix: disable auto-fade before stopping.
//
// AppManager Integration:
//   - AppManager::switchToApp() calls endFunc() on the old app automatically.
//     Don't call end() manually in exitApp() — double end() causes double
//     stop()/disconnect() which accelerates the hang.
// ---------------------------------------------------------------------------
#define MUSIC_PLAYER_DEBUG 1

// Disconnect BT on normal app exit? The ESP32-A2DP BT stack accumulates
// internal state corruption over repeated disconnect/reconnect cycles,
// eventually causing pPlayer->copy() to hang when writing to A2DPStream.
// Set to 0 (default) to keep BT connected on exit — stable and fast re-entry.
// Set to 1 to disconnect on exit — cleaner but may hang after ~5 cycles.
// Explicit disconnect via BT submenu always works regardless of this flag.
#define MUSIC_PLAYER_DISCONNECT_ON_EXIT 0

#if MUSIC_PLAYER_DEBUG
  #define MPLAYER_LOG(msg) Serial.println("[MusicPlayer] " msg)
  #define MPLAYER_LOGF(fmt, ...) Serial.printf("[MusicPlayer] " fmt "\n", ##__VA_ARGS__)
#else
  #define MPLAYER_LOG(msg) ((void)0)
  #define MPLAYER_LOGF(fmt, ...) ((void)0)
#endif

// SD Card SPI Pins
static const int PIN_SD_CLK  = 5;
static const int PIN_SD_MISO = 21;
static const int PIN_SD_MOSI = 19;
static const int PIN_SD_CS   = 8;

static const char* CACHE_PATH = "/music.idx";

// Bluetooth rune icon (7x10, XBM format) — shown in player header when connected
#define BT_ICON_WIDTH 7
#define BT_ICON_HEIGHT 10
static const uint8_t bt_icon_bits[] PROGMEM = {
    0x08,  // ...X....
    0x0C,  // ..XX....
    0x2A,  // .X.X.X..
    0x1C,  // ..XXX...
    0x08,  // ...X....
    0x08,  // ...X....
    0x1C,  // ..XXX...
    0x2A,  // .X.X.X..
    0x0C,  // ..XX....
    0x08,  // ...X....
};

// Lightning bolt icon (5x8, XBM format) — shown next to battery when charging
#define BOLT_ICON_WIDTH 5
#define BOLT_ICON_HEIGHT 8
static const uint8_t bolt_icon_bits[] PROGMEM = {
    0x10,  // ....X
    0x08,  // ...X.
    0x0C,  // ..XX.
    0x1F,  // XXXXX
    0x1F,  // XXXXX
    0x06,  // .XX..
    0x02,  // .X...
    0x01,  // X....
};
static const int MAX_VISIBLE_ITEMS = 4;
static const int ITEM_HEIGHT = 12;
static const int LIST_Y_START = 16;

MusicPlayerApp* MusicPlayerApp::instance = nullptr;
MusicPlayerApp musicPlayerApp(HAL::buttonManager());
static auto& display = HAL::displayProxy();

// =========================================================================
// Constructor
// =========================================================================
MusicPlayerApp::MusicPlayerApp(ButtonManager& btnMgr)
    : buttonManager(btnMgr), decoder()
{
    instance = this;
}

// =========================================================================
// Lifecycle
// =========================================================================
void MusicPlayerApp::begin() {
    currentState = STATE_DEVICE_MENU;
    menuCursorIndex = 0;
    menuScrollOffset = 0;
    isPlaying = false;
    btConnected = false;
    // NOTE: audioPipelineReady is NOT reset — the pipeline persists across
    // app enter/exit cycles. Recreating it after active playback crashes.
    nowPlayingTitle = "";
    nowPlayingArtist = "";
    connectingDeviceName = "";
    connectedDeviceName = "";
    currentTrackIndex = -1;
    marqueeOffset = 0;

    // Register all 6 button callbacks
    buttonManager.registerCallback(button_UpIndex, onButtonUp);
    buttonManager.registerCallback(button_DownIndex, onButtonDown);
    buttonManager.registerCallback(button_LeftIndex, onButtonLeft);
    buttonManager.registerCallback(button_RightIndex, onButtonRight);
    buttonManager.registerCallback(button_EnterIndex, onButtonEnter);
    buttonManager.registerCallback(button_SelectIndex, onButtonBack);

    // Init SD card
    initSD();

    // Load saved BT devices from NVS
    loadSavedDevices();

    // Always start at device menu — user picks which device to connect to.
    // Auto-reconnect on startup was causing BT stack crashes when the
    // target device wasn't in pairing mode.
    setState(STATE_DEVICE_MENU);
}

void MusicPlayerApp::end() {
    MPLAYER_LOG("end: enter");
    stopPlayback();
    destroyAudioPipeline();

    if (scannerActive) {
        btScanner.end();
        scannerActive = false;
    }

    // Unregister all button callbacks
    buttonManager.unregisterCallback(button_UpIndex);
    buttonManager.unregisterCallback(button_DownIndex);
    buttonManager.unregisterCallback(button_LeftIndex);
    buttonManager.unregisterCallback(button_RightIndex);
    buttonManager.unregisterCallback(button_EnterIndex);
    buttonManager.unregisterCallback(button_SelectIndex);
}

void MusicPlayerApp::update() {
    // Keep device awake during music playback (AppManager sleeps after 60s inactivity)
    if (isPlaying) {
        millis_APP_LASTINTERACTION = millis_NOW;
    }

    // Detect BT reconnect (heartbeat auto-reconnect recovered the connection)
    if (pA2dpStream && pA2dpStream->isConnected() && !btConnected) {
        MPLAYER_LOG("update: BT reconnected (heartbeat recovery)");
        btConnected = true;
    }

    // Drive audio pipeline only while playing
    if (isPlaying && audioPipelineReady && pPlayer) {
        // Detect BT disconnect during playback — stop before copy() blocks
        if (pA2dpStream && !pA2dpStream->isConnected()) {
            MPLAYER_LOG("update: BT disconnected during playback, stopping");
            stopPlayback();
            btConnected = false;
        } else {
            updateVolumeFromSlider();
            pPlayer->copy();

            // Check if track ended
            if (!pPlayer->isActive()) {
                isPlaying = false;
                // Auto-advance
                if (shuffleEnabled) {
                    int total = trackLibrary.size();
                    if (total > 1) {
                        int next = random(0, total);
                        if (next == currentTrackIndex) next = (next + 1) % total;
                        playTrack(next);
                    }
                } else if (currentTrackIndex + 1 < (int)trackLibrary.size()) {
                    playTrack(currentTrackIndex + 1);
                }
            }
        }
    }

    // State-specific polling
    switch (currentState) {
        case STATE_BT_CONNECTING:
            if (pA2dpStream && pA2dpStream->isConnected()) {
                btConnected = true;
                connectedDeviceName = connectingDeviceName;
                // Save address for proper source-mode disconnect later
                for (auto& d : savedDevices) {
                    if (d.name == connectingDeviceName) {
                        memcpy(connectedAddress, d.address, ESP_BD_ADDR_LEN);
                        break;
                    }
                }
                setState(STATE_MAIN_MENU);
            } else if (millis() - connectStartTime > CONNECT_TIMEOUT_MS) {
                // Timeout — clean up and show failure
                destroyAudioPipeline();
                setState(STATE_CONNECT_FAIL);
            }
            break;

        case STATE_BT_SWITCHING:
            // Phase 1: wait for old connection to drop
            if (switchWaitingForDisconnect) {
                if (!pA2dpStream || !pA2dpStream->isConnected()) {
                    MPLAYER_LOG("update: switch — old device disconnected");
                    switchWaitingForDisconnect = false;
                    switchStartTime = millis(); // start settle timer
                } else if (millis() - switchStartTime > 3000) {
                    MPLAYER_LOG("update: switch — disconnect timeout, proceeding anyway");
                    switchWaitingForDisconnect = false;
                    switchStartTime = millis();
                }
            }
            // Phase 2: settle time (let BT stack fully clean up)
            else if (millis() - switchStartTime >= 1000) {
                MPLAYER_LOG("update: switch — settle complete, connecting to new device");
                // Restore blocking read timeout for the new connection
                auto& buf = static_cast<audio_tools::BufferRTOS<uint8_t>&>(pA2dpStream->buffer());
                buf.setReadMaxWait(portMAX_DELAY);
                // Point library at new address and reconnect
                pA2dpStream->source().set_auto_reconnect(
                    const_cast<uint8_t*>(switchTargetDevice.address));
                pA2dpStream->source().reconnect();
                connectingDeviceName = switchTargetDevice.name;
                createAudioPipeline();
                connectStartTime = millis();
                setState(STATE_BT_CONNECTING);
            }
            break;

        default:
            break;
    }

    // Render
    display.clear();
    switch (currentState) {
        case STATE_DEVICE_MENU:      renderDeviceMenu();      break;
        case STATE_BT_SCANNING:      renderBtScanning();      break;
        case STATE_BT_CONNECTING:    renderConnecting();      break;
        case STATE_CONNECT_FAIL:     renderConnectFail();     break;
        case STATE_MAIN_MENU:        renderMainMenu();        break;
        case STATE_BT_SUBMENU:       renderBtSubMenu();       break;
        case STATE_FILE_BROWSER:     renderFileBrowser();     break;
        case STATE_PLAYER:           renderPlayer();          break;
        case STATE_SCANNING_LIBRARY: renderScanningLibrary(); break;
        case STATE_BT_SWITCHING:     renderSwitching();       break;
    }
    display.display();
}

// =========================================================================
// SD Card Init
// =========================================================================
void MusicPlayerApp::initSD() {
    if (sdAvailable) return;  // Already initialized from a previous session
    SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    sdAvailable = SD.begin(PIN_SD_CS);
    if (!sdAvailable) {
        MPLAYER_LOG("SD init failed");
    }
}

// =========================================================================
// Connection Lifecycle
// =========================================================================

void MusicPlayerApp::startConnectingByAddress(const SavedDevice& dev) {
    MPLAYER_LOG("startConnectingByAddress: enter");
    if (scannerActive) {
        btScanner.end();
        scannerActive = false;
    }

    connectingDeviceName = dev.name;
    btConnected = false;

    if (pA2dpStream) {
        // Reuse the existing stream. The library's internal task, timer, and
        // global pointer survive across disconnect/reconnect cycles.
        if (pA2dpStream->isConnected()) {
            // Check if we're already connected to the REQUESTED device
            if (memcmp(connectedAddress, dev.address, ESP_BD_ADDR_LEN) == 0) {
                // Same device — skip straight to menu
                MPLAYER_LOG("startConnectingByAddress: already connected, fast path");
                btConnected = true;
                connectedDeviceName = dev.name;
                createAudioPipeline();
                setState(STATE_MAIN_MENU);
                return;
            }
            // Connected to a DIFFERENT device — show "Switching..." screen
            // while disconnecting and settling. Direct disconnect+reconnect
            // fails (~2-3s connection drop) so we add a 1s settle phase.
            MPLAYER_LOG("startConnectingByAddress: different device, switching");
            stopPlayback();
            disconnectBT();
            switchTargetDevice = dev;
            switchStartTime = millis();
            switchWaitingForDisconnect = true;
            setState(STATE_BT_SWITCHING);
            return;
        }
        // Point the library at the new address and reconnect
        MPLAYER_LOG("startConnectingByAddress: reconnecting existing stream");
        // Restore blocking read timeout — disconnectBT() reduces it so the
        // BTC task doesn't block during disconnect. Restore portMAX_DELAY
        // for seamless audio playback (callback waits for data instead of
        // returning silence on empty buffer).
        auto& buf = static_cast<audio_tools::BufferRTOS<uint8_t>&>(pA2dpStream->buffer());
        buf.setReadMaxWait(portMAX_DELAY);
        pA2dpStream->source().set_auto_reconnect(const_cast<uint8_t*>(dev.address));
        pA2dpStream->source().reconnect();
    } else {
        // First connection ever — create the stream and full BT stack
        pA2dpStream = new A2DPStream();
        pA2dpStream->source().set_auto_reconnect(const_cast<uint8_t*>(dev.address));

        auto cfg = pA2dpStream->defaultConfig(TX_MODE);
        cfg.name = connectingDeviceName.c_str();
        cfg.auto_reconnect = true;
        cfg.wait_for_connection = false;
        // Prevent A2DPStream::write() from blocking forever when BT drops.
        // The write polling loop checks availableForWrite() every 5ms;
        // with timeout=200, it gives up after 200ms instead of looping forever.
        cfg.tx_write_timeout_ms = 200;
        pA2dpStream->begin(cfg);
    }

    createAudioPipeline();

    connectStartTime = millis();
    setState(STATE_BT_CONNECTING);
}

void MusicPlayerApp::createAudioPipeline() {
    if (audioPipelineReady || !pA2dpStream) return;

    if (sdAvailable && !pSourceSD) {
        pSourceSD = new AudioSourceIdxSD("/", "mp3", PIN_SD_CS);
    }

    if (pSourceSD) {
        pPlayer = new AudioPlayer(*pSourceSD, *pA2dpStream, decoder);
        pPlayer->setVolume(1.0);
        pPlayer->setMetadataCallback(onMetadata);
        // Initialize the decoder pipeline and SD source without selecting a stream.
        // begin(-1) skips initial file selection; playTrack() uses setPath() later.
        pPlayer->begin(-1, false);
        audioPipelineReady = true;
    }
}

void MusicPlayerApp::disconnectBT() {
    if (!pA2dpStream) return;
    MPLAYER_LOG("disconnectBT: disabling auto-reconnect");
    pA2dpStream->source().set_auto_reconnect(false);

    // The A2DPStream data callback runs on the BTC task and calls
    // a2dp_buffer.readArray() with portMAX_DELAY (xStreamBufferReceive).
    // When the AudioPlayer is stopped and the buffer is empty, this blocks
    // the BTC task forever. Since esp_a2d_source_disconnect() dispatches
    // through the same BTC task, it hangs waiting for the blocked task.
    //
    // Fix: temporarily reduce the read timeout so the callback returns
    // quickly on an empty buffer instead of blocking forever. Then write
    // silence to wake the currently-pending read (started with the old
    // portMAX_DELAY). After a brief delay the BTC task is free.
    //
    // We do NOT call pA2dpStream->clear() because that sets is_a2dp_active
    // to false. With is_a2dp_active=false, the callback stops draining
    // the buffer, so writes fill it to 100% and block on reconnect.
    // The read timeout is restored in startConnectingByAddress().
    MPLAYER_LOG("disconnectBT: unblocking A2DP data callback");
    auto& buf = static_cast<audio_tools::BufferRTOS<uint8_t>&>(pA2dpStream->buffer());
    buf.setReadMaxWait(pdMS_TO_TICKS(50));
    uint8_t silence[512] = {0};
    buf.writeArray(silence, sizeof(silence));
    delay(100);  // let BTC task process with the new short timeout

    esp_bd_addr_t empty = {0};
    if (memcmp(connectedAddress, empty, ESP_BD_ADDR_LEN) != 0) {
        MPLAYER_LOG("disconnectBT: calling esp_a2d_source_disconnect");
        esp_a2d_source_disconnect(connectedAddress);
        MPLAYER_LOG("disconnectBT: disconnect sent");
    }
    btConnected = false;
}

void MusicPlayerApp::destroyAudioPipeline() {
    MPLAYER_LOG("destroyAudioPipeline: enter");
    isPlaying = false;
    // NOTE: Don't call pPlayer->stop() here — stopPlayback() handles it.

#if MUSIC_PLAYER_DISCONNECT_ON_EXIT
    disconnectBT();
#else
    // Keep BT connected on exit — avoids BT stack degradation from
    // repeated disconnect/reconnect cycles. On re-entry, isConnected()
    // fast path skips straight to main menu.
    MPLAYER_LOG("destroyAudioPipeline: keeping BT connected (DISCONNECT_ON_EXIT=0)");
    btConnected = false;
#endif
    MPLAYER_LOG("destroyAudioPipeline: done");
}

// =========================================================================
// NVS Device Storage
// =========================================================================
void MusicPlayerApp::loadSavedDevices() {
    savedDevices.clear();
    Preferences prefs;
    prefs.begin("btdevs", true);

    int count = prefs.getInt("count", 0);
    for (int i = 0; i < count && i < 8; i++) {
        SavedDevice dev;
        dev.name = prefs.getString(("d" + String(i) + "n").c_str(), "");
        prefs.getBytes(("d" + String(i) + "a").c_str(), dev.address, ESP_BD_ADDR_LEN);
        if (dev.name.length() > 0) {
            savedDevices.push_back(dev);
        }
    }

    prefs.end();
}

void MusicPlayerApp::saveDevice(const String& name, esp_bd_addr_t address) {
    // Check if already saved (by address)
    for (int i = 0; i < (int)savedDevices.size(); i++) {
        if (memcmp(savedDevices[i].address, address, ESP_BD_ADDR_LEN) == 0) {
            // Move to front
            SavedDevice dev = savedDevices[i];
            savedDevices.erase(savedDevices.begin() + i);
            savedDevices.insert(savedDevices.begin(), dev);
            writeSavedDevicesToNVS();
            return;
        }
    }

    // Add new device at front
    SavedDevice dev;
    dev.name = name;
    memcpy(dev.address, address, ESP_BD_ADDR_LEN);
    savedDevices.insert(savedDevices.begin(), dev);

    // Trim to max 8
    while (savedDevices.size() > 8) savedDevices.pop_back();

    writeSavedDevicesToNVS();
}

void MusicPlayerApp::forgetAllDevices() {
    savedDevices.clear();
    Preferences prefs;
    prefs.begin("btdevs", false);
    prefs.clear();
    prefs.end();
}

void MusicPlayerApp::writeSavedDevicesToNVS() {
    Preferences prefs;
    prefs.begin("btdevs", false);
    prefs.putInt("count", savedDevices.size());
    for (int i = 0; i < (int)savedDevices.size(); i++) {
        prefs.putString(("d" + String(i) + "n").c_str(), savedDevices[i].name);
        prefs.putBytes(("d" + String(i) + "a").c_str(), savedDevices[i].address, ESP_BD_ADDR_LEN);
    }
    prefs.end();
}

// =========================================================================
// Library Scanning
// =========================================================================
void MusicPlayerApp::startLibraryScan() {
    if (!sdAvailable) {
        setState(STATE_FILE_BROWSER);
        return;
    }

    // Count MP3 files to check cache
    File root = SD.open("/");
    int fileCount = 0;
    if (root) {
        File entry;
        while ((entry = root.openNextFile())) {
            if (!entry.isDirectory()) {
                String name = entry.name();
                String lower = name;
                lower.toLowerCase();
                if (lower.endsWith(".mp3")) fileCount++;
            }
            entry.close();
        }
        root.close();
    }

    // Try loading cache
    if (fileCount > 0 && ID3Scanner::isCacheValid(CACHE_PATH, fileCount)) {
        if (ID3Scanner::loadCache(CACHE_PATH, trackLibrary)) {
            libraryScanned = true;
            setState(STATE_FILE_BROWSER);
            return;
        }
    }

    // Need full scan
    if (fileCount == 0) {
        trackLibrary.clear();
        libraryScanned = true;
        setState(STATE_FILE_BROWSER);
        return;
    }

    libraryScanTotal = fileCount;
    libraryScanCurrent = 0;
    setState(STATE_SCANNING_LIBRARY);

    // Do the scan (blocking for now — shows progress via update/render cycle)
    ID3Scanner::scanAllFiles("/", "mp3", trackLibrary,
        [](int current, int total) {
            if (MusicPlayerApp::instance) {
                MusicPlayerApp::instance->libraryScanCurrent = current;
                MusicPlayerApp::instance->libraryScanTotal = total;
                // Force a display update during scan
                display.clear();
                MusicPlayerApp::instance->renderScanningLibrary();
                display.display();
            }
        });

    // Save cache
    ID3Scanner::saveCache(CACHE_PATH, trackLibrary);
    libraryScanned = true;
    setState(STATE_FILE_BROWSER);
}

// =========================================================================
// Playback
// =========================================================================
void MusicPlayerApp::playTrack(int index) {
    if (!pPlayer || index < 0 || index >= (int)trackLibrary.size()) return;

    currentTrackIndex = index;
    nowPlayingTitle = "";
    nowPlayingArtist = "";
    marqueeOffset = 0;

    // Open file directly by path (bypasses SDIndex which may not be populated)
    const char* path = trackLibrary[index].path.c_str();
    MPLAYER_LOGF("playTrack: %d -> %s", index, path);
    pPlayer->setPath(path);
    pPlayer->setActive(true);
    isPlaying = true;
    setState(STATE_PLAYER);
}

void MusicPlayerApp::stopPlayback() {
    MPLAYER_LOG("stopPlayback: enter");
    isPlaying = false;
    if (pPlayer) {
        // Disable auto-fade before stopping. Auto-fade writes silence (2KB+)
        // to the A2DPStream output, which hangs after multiple BT disconnect/
        // reconnect cycles due to accumulated BT stack state corruption.
        pPlayer->setAutoFade(false);
        pPlayer->stop();
        pPlayer->setAutoFade(true);
    }
    MPLAYER_LOG("stopPlayback: done");
}

void MusicPlayerApp::nextTrack() {
    if (trackLibrary.empty()) return;
    int next;
    if (shuffleEnabled) {
        next = random(0, trackLibrary.size());
        if (next == currentTrackIndex && trackLibrary.size() > 1)
            next = (next + 1) % trackLibrary.size();
    } else {
        next = (currentTrackIndex + 1) % trackLibrary.size();
    }
    playTrack(next);
}

void MusicPlayerApp::prevTrack() {
    if (trackLibrary.empty()) return;
    int prev = currentTrackIndex - 1;
    if (prev < 0) prev = trackLibrary.size() - 1;
    playTrack(prev);
}

void MusicPlayerApp::togglePlayPause() {
    if (!pPlayer) return;

    // If no track selected, auto-pick a random one from the library
    if (currentTrackIndex < 0) {
        if (!trackLibrary.empty()) {
            playTrack(random(0, trackLibrary.size()));
        }
        return;
    }

    isPlaying = !isPlaying;
    pPlayer->setActive(isPlaying);
}

void MusicPlayerApp::updateVolumeFromSlider() {
    if (!pA2dpStream || !pA2dpStream->isConnected()) return;
    // Invert slider direction so "up" = louder
    float vol = (100.0f - sliderPosition_Percentage_Filtered) / 100.0f;
    static float lastVol = -1.0;
    if (abs(vol - lastVol) > 0.03) {
        pA2dpStream->setVolume(vol);
        lastVol = vol;
    }
}

// =========================================================================
// Metadata Callback (static)
// =========================================================================
void MusicPlayerApp::onMetadata(MetaDataType type, const char* str, int len) {
    if (!instance) return;
    String val = String(str).substring(0, len);
    val.trim();
    if (type == Title && val.length()) {
        instance->nowPlayingTitle = val;
        instance->marqueeOffset = 0;
    }
    if (type == Artist && val.length()) {
        instance->nowPlayingArtist = val;
    }
}

// =========================================================================
// State Management
// =========================================================================
void MusicPlayerApp::setState(MusicAppState newState) {
#if MUSIC_PLAYER_DEBUG
    static const char* stateNames[] = {
        "DEVICE_MENU", "BT_SCANNING", "BT_CONNECTING", "CONNECT_FAIL",
        "MAIN_MENU", "BT_SUBMENU", "FILE_BROWSER", "PLAYER", "SCANNING_LIBRARY",
        "BT_SWITCHING"
    };
    MPLAYER_LOGF("setState: %s -> %s",
        stateNames[currentState], stateNames[newState]);
#endif
    currentState = newState;
    resetCursor();
}

void MusicPlayerApp::resetCursor() {
    menuCursorIndex = 0;
    menuScrollOffset = 0;
}

int MusicPlayerApp::getListSize() const {
    switch (currentState) {
        case STATE_DEVICE_MENU:
            return savedDevices.size() + 1; // +1 for "Scan for new..."
        case STATE_BT_SCANNING:
            return btScanner.getResultCount();
        case STATE_MAIN_MENU:
            return MAIN_MENU_COUNT;
        case STATE_BT_SUBMENU:
            return getBtSubMenuCount();
        case STATE_FILE_BROWSER:
            return trackLibrary.size();
        default:
            return 0;
    }
}

void MusicPlayerApp::exitApp() {
    // Don't call end() here — AppManager::switchToApp() calls it via endFunc().
    // Double end() was causing pPlayer->stop() to hang after multiple cycles.
    MenuManager::instance().returnToMenu();
}

// =========================================================================
// Main Menu Items
// =========================================================================
String MusicPlayerApp::getMainMenuItem(int index) const {
    switch (index) {
        case 0: return "Now Playing";
        case 1: return "Browse Songs";
        case 2: return shuffleEnabled ? "Shuffle: On" : "Shuffle: Off";
        case 3: return "Bluetooth";
        case 4: return "Exit";
        default: return "";
    }
}

int MusicPlayerApp::getBtSubMenuCount() const {
    return 3; // Status, Disconnect, Forget All
}

String MusicPlayerApp::getBtSubMenuItem(int index) const {
    switch (index) {
        case 0: return btConnected ? ("Connected: " + connectedDeviceName) : "Not Connected";
        case 1: return "Disconnect";
        case 2: return "Forget All Devices";
        default: return "";
    }
}

// =========================================================================
// Button Callbacks (static)
// =========================================================================
void MusicPlayerApp::onButtonUp(const ButtonEvent& event) {
    if (event.eventType == ButtonEvent_Pressed) instance->handleUp();
}
void MusicPlayerApp::onButtonDown(const ButtonEvent& event) {
    if (event.eventType == ButtonEvent_Pressed) instance->handleDown();
}
void MusicPlayerApp::onButtonLeft(const ButtonEvent& event) {
    if (event.eventType == ButtonEvent_Pressed) instance->handleLeft();
}
void MusicPlayerApp::onButtonRight(const ButtonEvent& event) {
    if (event.eventType == ButtonEvent_Pressed) instance->handleRight();
}
void MusicPlayerApp::onButtonEnter(const ButtonEvent& event) {
    if (event.eventType == ButtonEvent_Released) instance->handleEnter();
}
void MusicPlayerApp::onButtonBack(const ButtonEvent& event) {
    if (event.eventType == ButtonEvent_Released) instance->handleBack();
}

// =========================================================================
// Navigation Logic
// =========================================================================
void MusicPlayerApp::handleUp() {
    if (currentState == STATE_PLAYER || currentState == STATE_BT_CONNECTING ||
        currentState == STATE_BT_SWITCHING || currentState == STATE_SCANNING_LIBRARY) return;

    if (menuCursorIndex > 0) {
        menuCursorIndex--;
        if (menuCursorIndex < menuScrollOffset)
            menuScrollOffset = menuCursorIndex;
    }
}

void MusicPlayerApp::handleDown() {
    if (currentState == STATE_PLAYER || currentState == STATE_BT_CONNECTING ||
        currentState == STATE_BT_SWITCHING || currentState == STATE_SCANNING_LIBRARY) return;

    int listSize = getListSize();
    if (menuCursorIndex < listSize - 1) {
        menuCursorIndex++;
        if (menuCursorIndex >= menuScrollOffset + MAX_VISIBLE_ITEMS)
            menuScrollOffset = menuCursorIndex - MAX_VISIBLE_ITEMS + 1;
    }
}

void MusicPlayerApp::handleLeft() {
    if (currentState == STATE_PLAYER) {
        prevTrack();
    }
}

void MusicPlayerApp::handleRight() {
    if (currentState == STATE_PLAYER) {
        nextTrack();
    }
}

void MusicPlayerApp::handleEnter() {
    switch (currentState) {
        case STATE_DEVICE_MENU: {
            if (menuCursorIndex < (int)savedDevices.size()) {
                // Copy device data before saveDevice() modifies the vector
                // (erase + insert invalidates references)
                SavedDevice dev = savedDevices[menuCursorIndex];
                saveDevice(dev.name, dev.address); // Move to front
                startConnectingByAddress(dev);
            } else {
                // "Scan for new..." selected
                if (!scannerActive) {
                    if (btScanner.begin()) {
                        scannerActive = true;
                        btScanner.startScan(12);
                        setState(STATE_BT_SCANNING);
                    }
                }
            }
            break;
        }

        case STATE_BT_SCANNING: {
            int count = btScanner.getResultCount();
            if (menuCursorIndex < count) {
                BTScanResult result = btScanner.getResult(menuCursorIndex);
                saveDevice(String(result.name), result.address);
                // Build a SavedDevice and use address-based connection
                SavedDevice dev;
                dev.name = String(result.name);
                memcpy(dev.address, result.address, ESP_BD_ADDR_LEN);
                startConnectingByAddress(dev);
            }
            break;
        }

        case STATE_CONNECT_FAIL:
            setState(STATE_DEVICE_MENU);
            break;

        case STATE_MAIN_MENU:
            switch (menuCursorIndex) {
                case 0: // Now Playing
                    setState(STATE_PLAYER);
                    break;
                case 1: // Browse Songs
                    if (!libraryScanned) {
                        startLibraryScan();
                    } else {
                        setState(STATE_FILE_BROWSER);
                    }
                    break;
                case 2: // Shuffle toggle
                    shuffleEnabled = !shuffleEnabled;
                    break;
                case 3: // Bluetooth
                    setState(STATE_BT_SUBMENU);
                    break;
                case 4: // Exit
                    exitApp();
                    break;
            }
            break;

        case STATE_BT_SUBMENU:
            switch (menuCursorIndex) {
                case 0: // Status (info only)
                    break;
                case 1: { // Disconnect — always disconnects (explicit user action)
                    stopPlayback();
                    disconnectBT();
                    setState(STATE_DEVICE_MENU);
                    break;
                }
                case 2: { // Forget All
                    forgetAllDevices();
                    stopPlayback();
                    disconnectBT();
                    setState(STATE_DEVICE_MENU);
                    break;
                }
            }
            break;

        case STATE_FILE_BROWSER:
            if (!trackLibrary.empty()) {
                playTrack(menuCursorIndex);
            }
            break;

        case STATE_PLAYER:
            togglePlayPause();
            break;

        default:
            break;
    }
}

void MusicPlayerApp::handleBack() {
    switch (currentState) {
        case STATE_DEVICE_MENU:
            exitApp();
            break;

        case STATE_BT_SCANNING:
            if (scannerActive) {
                btScanner.end();
                scannerActive = false;
            }
            setState(STATE_DEVICE_MENU);
            break;

        case STATE_CONNECT_FAIL:
            setState(STATE_DEVICE_MENU);
            break;

        case STATE_MAIN_MENU:
            exitApp();
            break;

        case STATE_BT_SUBMENU:
            setState(STATE_MAIN_MENU);
            break;

        case STATE_FILE_BROWSER:
            setState(STATE_MAIN_MENU);
            break;

        case STATE_PLAYER:
            // Back to main menu, playback continues
            setState(STATE_MAIN_MENU);
            break;

        default:
            break;
    }
}

// =========================================================================
// UI Rendering
// =========================================================================
void MusicPlayerApp::drawHeader(const String& title) {
    display.setColor(WHITE);
    display.fillRect(0, 0, 128, 14);
    display.setColor(BLACK);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 1, title);
    display.setColor(WHITE);
}

void MusicPlayerApp::drawList(const std::vector<String>& items, int cursor, int scrollOffset) {
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        int idx = i + scrollOffset;
        if (idx >= (int)items.size()) break;

        int y = LIST_Y_START + (i * ITEM_HEIGHT);
        if (idx == cursor) {
            display.fillRect(0, y, 128, ITEM_HEIGHT);
            display.setColor(BLACK);
            display.drawString(4, y - 1, items[idx]);
            display.setColor(WHITE);
        } else {
            display.drawString(4, y - 1, items[idx]);
        }
    }
}

void MusicPlayerApp::drawScrollBar(int totalItems, int visibleItems, int scrollOffset) {
    if (totalItems <= visibleItems) return;

    int barHeight = 48; // Available height for scrollbar
    int thumbHeight = max(4, (barHeight * visibleItems) / totalItems);
    int thumbY = LIST_Y_START + (barHeight * scrollOffset) / totalItems;

    display.drawRect(126, LIST_Y_START, 2, barHeight);
    display.fillRect(126, thumbY, 2, thumbHeight);
}

void MusicPlayerApp::renderDeviceMenu() {
    drawHeader("Bluetooth");

    std::vector<String> items;
    for (const auto& dev : savedDevices) {
        items.push_back(dev.name);
    }
    items.push_back("Scan for new...");

    drawList(items, menuCursorIndex, menuScrollOffset);
    drawScrollBar(items.size(), MAX_VISIBLE_ITEMS, menuScrollOffset);
}

void MusicPlayerApp::renderBtScanning() {
    drawHeader("Scanning");

    int count = btScanner.getResultCount();

    if (count == 0) {
        // Animated "Searching..."
        int dots = (millis() / 500) % 4;
        String msg = "Searching";
        for (int i = 0; i < dots; i++) msg += ".";

        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 30, msg);
    } else {
        std::vector<String> items;
        for (int i = 0; i < count; i++) {
            BTScanResult r = btScanner.getResult(i);
            // Show signal strength indicator
            String entry = r.name;
            int bars = 0;
            if (r.rssi > -60) bars = 3;
            else if (r.rssi > -75) bars = 2;
            else bars = 1;
            String sig = " (";
            for (int b = 0; b < bars; b++) sig += "*";
            sig += ")";
            entry += sig;
            items.push_back(entry);
        }
        drawList(items, menuCursorIndex, menuScrollOffset);
    }

    // Show scanning indicator at bottom
    if (btScanner.isScanning()) {
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.drawString(124, 54, "scanning...");
    }
}

void MusicPlayerApp::renderConnecting() {
    drawHeader("Bluetooth");

    int dots = (millis() / 400) % 4;
    String msg = "Connecting";
    for (int i = 0; i < dots; i++) msg += ".";

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, msg);
    display.drawString(64, 38, connectingDeviceName);
}

void MusicPlayerApp::renderConnectFail() {
    drawHeader("Bluetooth");

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, "Connection Failed");
    display.drawString(64, 40, "Press any button");
}

void MusicPlayerApp::renderMainMenu() {
    drawHeader("Music Player");

    std::vector<String> items;
    for (int i = 0; i < MAIN_MENU_COUNT; i++) {
        items.push_back(getMainMenuItem(i));
    }
    drawList(items, menuCursorIndex, menuScrollOffset);

    // Connection status in bottom-right
    if (btConnected) {
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.drawString(124, 54, "BT");
    }
}

void MusicPlayerApp::renderBtSubMenu() {
    drawHeader("Bluetooth");

    std::vector<String> items;
    int count = getBtSubMenuCount();
    for (int i = 0; i < count; i++) {
        items.push_back(getBtSubMenuItem(i));
    }
    drawList(items, menuCursorIndex, menuScrollOffset);
}

void MusicPlayerApp::renderFileBrowser() {
    drawHeader("Library");

    if (trackLibrary.empty()) {
        display.setFont(ArialMT_Plain_10);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 30, sdAvailable ? "No MP3 files found" : "No SD card");
        return;
    }

    std::vector<String> items;
    for (const auto& t : trackLibrary) {
        items.push_back(t.display);
    }
    drawList(items, menuCursorIndex, menuScrollOffset);
    drawScrollBar(items.size(), MAX_VISIBLE_ITEMS, menuScrollOffset);
}

void MusicPlayerApp::renderPlayer() {
    drawHeader("Now Playing");

    // BT icon top-left (drawn over white header in black)
    if (btConnected) {
        display.setColor(BLACK);
        display.drawXbm(2, 2, BT_ICON_WIDTH, BT_ICON_HEIGHT, bt_icon_bits);
        display.setColor(WHITE);
    }

    // Battery in header: outline with % text inside, bolt if charging
    display.setColor(BLACK);
    display.setFont(ArialMT_Plain_10);
    int battPct = (int)batteryVoltagePercentage;
    if (battPct > 100) battPct = 100;
    if (battPct < 0) battPct = 0;
    String battStr = String(battPct);
    int textW = display.getStringWidth(battStr);
    int bodyW = textW + 6;              // 3px padding each side
    int bodyH = 10;
    int bodyX = 124 - bodyW;            // right-aligned with 2px for terminal
    int bodyY = 2;
    display.drawRect(bodyX, bodyY, bodyW, bodyH);           // battery body outline
    display.fillRect(bodyX + bodyW, bodyY + 3, 2, 4);       // terminal nub
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(bodyX + bodyW / 2, bodyY - 1, battStr);
    // Lightning bolt when charging
    if (batteryChangeRate > 0.5f) {
        display.drawXbm(bodyX - BOLT_ICON_WIDTH - 1, bodyY + 1, BOLT_ICON_WIDTH, BOLT_ICON_HEIGHT, bolt_icon_bits);
    }
    display.setColor(WHITE);

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);

    // Artist (shifted up ~7px from original)
    if (nowPlayingArtist.length()) {
        display.drawString(64, 15, nowPlayingArtist);
    }

    // Title with marquee
    String titleText = nowPlayingTitle.length() ? nowPlayingTitle :
                       (currentTrackIndex >= 0 && currentTrackIndex < (int)trackLibrary.size()) ?
                       trackLibrary[currentTrackIndex].display : "No track";

    int titleWidth = display.getStringWidth(titleText);
    int maxWidth = 120;
    if (titleWidth > maxWidth) {
        // Marquee scroll
        if (millis() - lastMarqueeUpdate > 300) {
            lastMarqueeUpdate = millis();
            marqueeOffset += 6;
            if (marqueeOffset > titleWidth - maxWidth + 30) {
                marqueeOffset = -20;
            }
        }
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(4 - marqueeOffset, 26, titleText);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
    } else {
        display.drawString(64, 26, titleText);
    }

    // Play/Pause + shuffle
    String status = isPlaying ? "> Playing" : "|| Paused";
    if (shuffleEnabled) status += "  [S]";
    display.drawString(64, 38, status);

    // Volume bar
    int volumePct = (int)(100.0f - sliderPosition_Percentage_Filtered);
    if (volumePct < 0) volumePct = 0;
    if (volumePct > 100) volumePct = 100;
    display.drawProgressBar(14, 50, 100, 6, volumePct);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 47, "V");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 47, String(volumePct));
}

void MusicPlayerApp::renderScanningLibrary() {
    drawHeader("Library");

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, "Scanning library...");

    if (libraryScanTotal > 0) {
        int pct = (libraryScanCurrent * 100) / libraryScanTotal;
        display.drawProgressBar(10, 40, 108, 10, pct);

        String progress = String(libraryScanCurrent) + " / " + String(libraryScanTotal);
        display.drawString(64, 54, progress);
    }
}

void MusicPlayerApp::renderSwitching() {
    drawHeader("Bluetooth");

    int dots = (millis() / 400) % 4;
    String msg = "Switching";
    for (int i = 0; i < dots; i++) msg += ".";

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, msg);
    display.drawString(64, 38, switchTargetDevice.name);
}
