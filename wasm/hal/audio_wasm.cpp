// WASM AudioManager implementation — uses Web Audio API for tone generation

#include "AudioManager.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, js_audio_play_tone, (float frequency, float volume), {
    if (!Module._audioCtx) {
        Module._audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    }
    js_audio_stop_tone();

    var ctx = Module._audioCtx;
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = 'square';
    osc.frequency.setValueAtTime(frequency, ctx.currentTime);
    gain.gain.setValueAtTime(volume, ctx.currentTime);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start();
    Module._audioOsc = osc;
    Module._audioGain = gain;
});

EM_JS(void, js_audio_stop_tone, (), {
    if (Module._audioOsc) {
        try { Module._audioOsc.stop(); } catch(e) {}
        Module._audioOsc = null;
        Module._audioGain = null;
    }
});

EM_JS(void, js_audio_set_volume, (float volume), {
    if (Module._audioGain) {
        Module._audioGain.gain.setValueAtTime(volume, Module._audioCtx.currentTime);
    }
});

#else
inline void js_audio_play_tone(float, float) {}
inline void js_audio_stop_tone() {}
inline void js_audio_set_volume(float) {}
#endif

AudioManager::AudioManager()
    : currentFrequency(0)
    , isPlaying(false)
    , stopAtMillis(0)
    , in(generator)
{}

void AudioManager::init() {}

void AudioManager::loop() {
    if (isPlaying && stopAtMillis > 0 && millis() >= stopAtMillis) {
        stopTone();
    }
}

void AudioManager::setVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    js_audio_set_volume(volume);
}

void AudioManager::playTone(float frequency, int durationMs) {
    currentFrequency = frequency;
    isPlaying = true;
    stopAtMillis = (durationMs > 0) ? millis() + durationMs : 0;
    js_audio_play_tone(frequency, 0.3f);
}

void AudioManager::stopTone() {
    isPlaying = false;
    currentFrequency = 0;
    stopAtMillis = 0;
    js_audio_stop_tone();
}

void AudioManager::enableMic(bool) {}

float AudioManager::getMicVolumeDb() const {
    return -60.0f;
}
