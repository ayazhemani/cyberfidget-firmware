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
    cfg.buffer_count = 12;    // default is usually smaller
    cfg.buffer_size  = 256;  // bytes per buffer

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
    inCfg.buffer_count = 6;
    inCfg.buffer_size  = 512;

    i2sIn.begin(inCfg);
    Serial.printf("Mic I2S: sr=%d ch=%d bits=%d\n",
              inCfg.sample_rate, inCfg.channels, inCfg.bits_per_sample);

    micMeter.begin(AudioInfo(inCfg.sample_rate, 1, 16));
    i2sIn.setTimeout(0);   // readBytes returns immediately if no data
    micMeter.setTimeout(0);

    // Start low-priority mic pump on the other core
    if (micTaskHandle == nullptr) {
        xTaskCreatePinnedToCore(
            &AudioManager::micTaskThunk,  // task entry
            "micPump",
            4096,                         // stack
            this,                         // arg
            1,                            // low prio
            &micTaskHandle,
            0                             // pin to core 0
        );
    }
}

void AudioManager::loop() {
    // --- Tone path ---
    if (isPlaying) {
    copier.copy(); // this one already pulls fast/non-blocking
    if (stopAtMillis > 0 && millis() >= stopAtMillis) {
        stopTone();
        stopAtMillis = 0;
        }
    }
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
    float lin = getMicVolumeLinear();      // 0..1
    if (lin < 1e-6f) lin = 1e-6f;          // avoid log(0)
    return 20.0f * log10f(lin);            // dBFS (negative up to 0)
}

void AudioManager::micTaskThunk(void *arg) {
  reinterpret_cast<AudioManager*>(arg)->micTaskLoop();
}

void AudioManager::micTaskLoop() {
  static uint8_t buf[512];     // small DMA-friendly chunk
  const TickType_t pollDelay = pdMS_TO_TICKS(1);
  uint32_t lastLevelMs = millis();

  for (;;) {
    int avail = i2sIn.available();
    if (avail > 0) {
      size_t toRead = (size_t)avail;
      if (toRead > sizeof(buf)) toRead = sizeof(buf);

      // readBytes is non-blocking because you set timeout(0)
      int n = i2sIn.readBytes(buf, toRead);
      if (n > 0) {
        micMeter.write(buf, (size_t)n);
      }
    } else {
      // no data ready; yield a tick
      vTaskDelay(pollDelay);
    }

    // publish a new level ~every 20ms
    uint32_t now = millis();
    if (now - lastLevelMs >= 20) {
      float raw = micMeter.volume();            // ~0..32767
      // optional: micMeter.clear();            // if you want “peak since last publish”
      micVolumeAtomic = (raw <= 0.0f) ? 0.0f : (raw / 32768.0f); // 0..1
      lastLevelMs = now;
    }
  }
}