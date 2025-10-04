// lib/AudioManager/AudioManager.h
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include "AudioTools.h"
using namespace audio_tools;

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
    float getMicVolumeLinear() const { return micVolume; }
    float getMicVolumeDb() const; // 20*log10(linear), clamped

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
};

#endif // AUDIO_MANAGER_H