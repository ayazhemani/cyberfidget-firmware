// lib/AudioManager/AudioManager.cpp
// Mode-aware AudioManager: idle by default.
// MusicPlayerApp and future apps manage their own audio pipelines.
#include "AudioManager.h"

AudioManager::AudioManager() {}

void AudioManager::init() {
    // Idle mode: no I2S ports initialized.
    // MusicPlayerApp will set up A2DP + MP3 pipeline in its own begin().
    // Future apps (VoiceRecorder, TTS) can claim I2S ports as needed.
}

void AudioManager::loop() {
    // No-op in idle mode. Apps handle their own audio processing.
}
