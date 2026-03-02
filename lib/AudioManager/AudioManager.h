// lib/AudioManager/AudioManager.h
// Mode-aware AudioManager: starts idle, apps manage their own audio pipelines.
// Keeps I2S pin constants and mic/speaker port info accessible for future use.
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>

// I2S Pin Constants (available for any app that needs them)
namespace AudioPins {
    // TX: MAX98357A speaker amplifier (I2S Port 0)
    constexpr int TX_WS   = 27;  // LRCLK
    constexpr int TX_BCK  = 26;  // BCLK
    constexpr int TX_DOUT = 14;  // DATA OUT

    // RX: ICS-43434 microphone (I2S Port 1)
    constexpr int RX_WS   = 25;  // LRCLK
    constexpr int RX_BCK  = 32;  // BCLK
    constexpr int RX_DIN  = 33;  // DATA IN
}

class AudioManager {
public:
    AudioManager();

    // Starts in idle mode - no I2S ports initialized.
    // Apps that need audio (MusicPlayerApp, future VoiceRecorderApp, etc.)
    // manage their own pipelines via AudioTools directly.
    void init();
    void loop();
};

#endif // AUDIO_MANAGER_H
