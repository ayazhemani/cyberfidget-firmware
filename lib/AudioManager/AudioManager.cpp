// lib/AudioManager/AudioManager.cpp
#include "AudioManager.h"
#include "globals.h"
#include <math.h>

AudioManager::AudioManager()
    : currentFrequency(440.0f),
      isPlaying(false),
      stopAtMillis(0),
      in(generator),
      volume(in),
      copier(i2s, volume),
      micCopy(micMeter, i2sIn)
{
}

void AudioManager::init() {
    // --- TX: MAX98357A path ---
    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.port_no         = 0;
    cfg.i2s_format      = I2S_LSB_FORMAT;   // keep exactly as you had it
    cfg.pin_ws          = 27;               // LRCLK
    cfg.pin_bck         = 26;               // BCLK
    cfg.pin_data        = 14;               // DOUT
    cfg.channels        = 2;
    cfg.bits_per_sample = 16;
    // (Optionally set sample_rate if you want it explicit; not required if working)
    // cfg.sample_rate     = 44100;

    // Keep I2S port default here (usually 0). We’ll put mic on the other port.
    i2s.begin(cfg);

    generator.setFrequency(currentFrequency);

    auto vcfg = volume.defaultConfig();
    vcfg.copyFrom(cfg);
    volume.begin(vcfg);
    volume.setVolume(0.7f);

    isPlaying = false;

    // --- RX: ICS-43434 mic into VolumeMeter ---
    // Put mic on the *other* I2S peripheral to avoid any cross-talk.
    // ESP32 has I2S0/I2S1; if TX is using default (0), use 1 here.
    auto inCfg = i2sIn.defaultConfig(RX_MODE);
    inCfg.port_no         = 1;              // << important: separate port
    inCfg.i2s_format      = I2S_STD_FORMAT; // ICS-43434 standard I2S
    inCfg.sample_rate     = 44100;          // or your preferred rate
    inCfg.bits_per_sample = 16;             // PDM->I2S mics often packed as 24-in-32
    inCfg.channels        = 1;              // mono mic
    inCfg.pin_ws          = 25;             // LRCLK
    inCfg.pin_bck         = 32;             // BCLK
    inCfg.pin_data_rx     = 33;             // DATA IN
    inCfg.pin_data        = -1;             // not used for RX
    inCfg.is_master       = true;
    inCfg.buffer_count = 8;
    inCfg.buffer_size  = 512;

    i2sIn.begin(inCfg);
    Serial.printf("Mic I2S: sr=%d ch=%d bits=%d\n",
              inCfg.sample_rate, inCfg.channels, inCfg.bits_per_sample);

    micMeter.begin(AudioInfo(inCfg.sample_rate, 1, 16));
}

void AudioManager::loop() {
    // --- Tone path ---
    if (isPlaying) {
        copier.copy(); // non-blocking pull

        if (stopAtMillis > 0 && millis() >= stopAtMillis) {
            stopTone();
            stopAtMillis = 0;
        }
    }

    // --- Mic path ---
    // Pull what’s available from i2sIn -> convIn -> micMeter
    // (This is non-blocking; when nothing is available, it moves 0 bytes.)
    //micCopy.copy();
    
    size_t moved = micCopy.copy();

    float raw = micMeter.volume();         // ~0..32767 (peak)
    float lin = raw / 32768.0f;            // normalize to 0..1
    if (lin < 1e-6f) lin = 1e-6f;          // avoid log(0)
    micVolume = lin;

    float dB = 20.0f * log10f(lin);

    // Read current peak (0..1). The '0' channel is fine for mono.
    // VolumeMeter typically holds last peak until cleared; use the no-clear
    // accessor if present in your version, or read + (optionally) clear.
    micVolume = micMeter.volume(); // linear 0..1

    // Serial.printf("mic moved=%u  raw=%.0f  lin=%.3f  dBFS=%.1f\n",
    //           (unsigned)moved, raw, lin, dB);

    // Optional: clear peaks so next frame is fresh
    // micMeter.clear();

    // A tiny yield keeps Wi-Fi/BT/Serial happy
    // delay(1);
}

void AudioManager::setVolume(float volumeLevel) {
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

    stopAtMillis = (durationMs > 0) ? (millis() + durationMs) : 0;
}

void AudioManager::stopTone() {
    if (isPlaying) {
        generator.end();
        isPlaying = false;
        // volume.setVolume(0.0f); // optional instant silence
    }
}

float AudioManager::getMicVolumeDb() const {
    const float v = micVolume <= 1e-6f ? 1e-6f : micVolume;
    float db = 20.0f * log10f(v);
    if (db < -120.0f) db = -120.0f;
    return db;
}