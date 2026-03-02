#ifndef MUSIC_PLAYER_APP_H
#define MUSIC_PLAYER_APP_H

#include <Arduino.h>
#include <vector>
#include <string>

#include "HAL.h"
#include "AppDefs.h"
#include "ButtonManager.h"
#include "MenuManager.h"

// --- Audio Tools Includes (Diet Mode) ---
#include "AudioTools/CoreAudio/AudioLogger.h"
#include "AudioTools/Disk/AudioSource.h"
#include "AudioTools/CoreAudio/AudioPlayer.h"
#include "AudioTools/AudioLibs/A2DPStream.h"
#include "AudioTools/Disk/AudioSourceIdxSD.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

enum MusicAppState {
    STATE_BT_SCAN,
    STATE_BT_CONNECTING,
    STATE_FILE_BROWSER,
    STATE_PLAYER
};

struct BTDeviceEntry {
    String name;
    esp_bd_addr_t address;
};

class MusicPlayerApp {
public:
    MusicPlayerApp(ButtonManager& btnMgr);

    void begin();
    void update();
    void end();

    static void onButtonUpPressed(const ButtonEvent& event);
    static void onButtonDownPressed(const ButtonEvent& event);
    static void onButtonSelectPressed(const ButtonEvent& event);
    static void onButtonBackPressed(const ButtonEvent& event);

private:
    static MusicPlayerApp* instance;
    ButtonManager& buttonManager;

    MusicAppState currentState;
    bool isConnected = false;
    bool audioPipelineReady = false;
    int menuCursorIndex = 0;
    int menuScrollOffset = 0;

    std::vector<BTDeviceEntry> deviceList;

    // Audio pipeline - heap-allocated on demand, not at global construction
    audio_tools::A2DPStream a2dpStream;
    audio_tools::AudioSourceIdxSD* pSourceSD = nullptr;
    audio_tools::MP3DecoderHelix decoder;
    audio_tools::AudioPlayer* pPlayer = nullptr;

    bool isPlaying = false;

    bool initAudioPipeline();
    void scanForDevices();
    void connectToDevice(int index);
    void drawFileBrowser();
    void playFileAtIndex(int index);
    void stopPlayback();
    void updateVolumeFromSlider();

    void drawHeader(String title);
    void drawList(const std::vector<String>& items, int cursor, int offset);
    void renderScanMenu();
    void renderConnecting();
    void renderPlayer();

    void handleNavUp();
    void handleNavDown();
    void handleNavSelect();
    void handleNavBack();
};

extern MusicPlayerApp musicPlayerApp;

#endif // MUSIC_PLAYER_APP_H
