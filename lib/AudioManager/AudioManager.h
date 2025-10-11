// lib/AudioManager/AudioManager.h
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include "AudioTools.h"
using namespace audio_tools;

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class AudioManager {
public:
    AudioManager();

    void init();
    void loop(); // Call this regularly to process audio

    // Tone control
    void setVolume(float volume);                       // 0.0..1.0
    void playTone(float frequency, int durationMs = 0); // 0 = indefinite
    void stopTone();

    // Mic level (linear 0..1 and dBFS-ish)
    float getMicVolumeLinear() const { return micVolumeAtomic; }
    float getMicVolumeDb() const;      // dBFS (<= 0), derived from linear 0..1
    float getMicVolumeDbFS() const {   // alias, if you prefer explicit name
        return getMicVolumeDb();
    }

private:
    // --- Tone state ---
    float currentFrequency;
    bool  isPlaying;
    unsigned long stopAtMillis;

    // --- Tone chain (TX) ---
    I2SStream i2s;                           // TX to MAX98357A (keep your pins)
    SineWaveGenerator<int16_t> generator;
    GeneratedSoundStream<int16_t> in;
    VolumeStream volume;
    StreamCopy copier;                       // volume -> i2s

    // --- Mic chain (RX) ---
    I2SStream            i2sIn;              // RX from ICS-43434
    VolumeMeter          micMeter;           // measures amplitude
    StreamCopy           micCopy;            // convIn -> micMeter

    // Latest measured mic amplitude (0..1)
    float micVolume = 0.0f;

    // Mic task
    static void micTaskThunk(void *arg);
    void micTaskLoop();
    TaskHandle_t micTaskHandle = nullptr;

    // cross-core safe handoff
    volatile float micVolumeAtomic = 0.0f;
};

#endif // AUDIO_MANAGER_H