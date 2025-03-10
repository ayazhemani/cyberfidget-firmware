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

    void setVolume(float volume);                      // Set volume (0.0 to 1.0)
    void playTone(float frequency, int durationMs = 0); // Play a tone at specified frequency and duration
    void stopTone();                                   // Stop playing the tone

private:
    float currentFrequency;
    bool isPlaying;
    unsigned long stopAtMillis; // Time when the tone should stop

    // Audio objects
    I2SStream i2s;                           // I2S output stream
    SineWaveGenerator<int16_t> generator;
    GeneratedSoundStream<int16_t> in;
    VolumeStream volume;
    StreamCopy copier;
};

#endif // AUDIO_MANAGER_H