// lib/AudioManager/AudioManager.cpp

#include "AudioManager.h"
#include "globals.h" // For accessing global variables like slider positions

AudioManager::AudioManager()
    : currentFrequency(440.0f), // Default frequency
      isPlaying(false),
      stopAtMillis(0),          // Initialize stop time
      in(generator),
      volume(in),
      copier(i2s, volume)
{
    // Constructor body can be empty or initialize variables as needed
}

void AudioManager::init() {
    //AudioLogger::instance().begin(Serial, AudioLogger::Info);

    // Configure I2S for audio output
    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.i2s_format = I2S_LSB_FORMAT; // Use standard I2S format
    cfg.pin_ws = 27;   // LRC pin (Word Select)
    cfg.pin_bck = 26;  // BCLK pin
    cfg.pin_data = 14; // DIN pin
    cfg.channels = 2;
    cfg.bits_per_sample = 16;

    i2s.begin(cfg);

    // Initialize the sine wave generator
    generator.setFrequency(currentFrequency);

    // Initialize the volume control
    auto vcfg = volume.defaultConfig();
    vcfg.copyFrom(cfg); // Copy configuration from I2S
    volume.begin(vcfg);
    volume.setVolume(0.7f); // Default volume at 100%

    isPlaying = false;
}

void AudioManager::loop() {
    if (isPlaying) {
        // Copy audio data from generator through volume to I2S
        copier.copy();

        // Check if we need to stop the tone
        if (stopAtMillis > 0 && millis() >= stopAtMillis) {
            stopTone();
            stopAtMillis = 0; // Reset stop time
        }
    }
}

void AudioManager::setVolume(float volumeLevel) {
    // Ensure volume is within 0.0 to 1.0
    float vol = constrain(volumeLevel, 0.0f, 1.0f);
    volume.setVolume(vol);
}

void AudioManager::playTone(float frequency, int durationMs) {
    currentFrequency = frequency;
    generator.setFrequency(currentFrequency);

    if (!isPlaying) {
        generator.begin();
        isPlaying = true;
    }

    if (durationMs > 0) {
        stopAtMillis = millis() + durationMs; // Set time to stop the tone
    } else {
        stopAtMillis = 0; // Play indefinitely
    }
}

void AudioManager::stopTone() {
    if (isPlaying) {
        generator.end();
        isPlaying = false;
    }
}