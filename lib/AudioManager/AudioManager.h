#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

// Expose the audio player state and beep-related flags to other modules if needed
extern bool audioPlayerRunning;
extern bool playingBeep;
extern unsigned long beepStart;

// Audio subsystem objects
extern I2SStream i2s;
extern SineWaveGenerator<int16_t> sine;
extern GeneratedSoundStream<int16_t> in;
extern VolumeStream volume;
extern StreamCopy copier;
extern AudioActions action;

// Audio management function prototypes
void initAudio();            // New initialization function
void setupActions();
void actionKeyOn(bool active, int pin, void* ptr);
void actionKeyOff(bool active, int pin, void* ptr);
void drawAudioPlayer();
void loopAudio();
void beepOnBounce();

#endif // AUDIO_MANAGER_H
