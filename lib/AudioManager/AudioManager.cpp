#include "AudioManager.h"
#include "globals.h"           // For globals like sliderPosition_Percentage, button_* constants
#include "SSD1306Wire.h"       // For display access

extern SSD1306Wire display;
extern const uint8_t suiGenerisRg_20[];  // Font used in drawAudioPlayer

bool audioPlayerRunning = false;
bool playingBeep = false;
unsigned long beepStart = 0;

// Instantiate audio objects
I2SStream i2s;
SineWaveGenerator<int16_t> sine;
GeneratedSoundStream<int16_t> in(sine);
VolumeStream volume(in);
StreamCopy copier(i2s, volume);
AudioActions action;

void initAudio() {
    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.i2s_format      = I2S_LSB_FORMAT;
    cfg.pin_ws          = 27; 
    cfg.pin_bck         = 26; 
    cfg.pin_data        = 14; 
    cfg.channels        = 2;
    cfg.bits_per_sample = 16;
    
    i2s.begin(cfg);
    in.begin(cfg);
    
    auto vcfg = volume.defaultConfig();
    vcfg.copyFrom(cfg);
    volume.begin(vcfg);
    volume.setVolume(0.0);
    
    setupActions(); 
}

void setupActions() {
    auto act_low = AudioActions::ActiveLow;
    static float note[] = { N_C3, N_D3, N_E3, N_F3, N_G3, N_A3 };
    action.add(button_TopLeft,    actionKeyOn, actionKeyOff, act_low, &note[0]);
    action.add(button_TopRight,   actionKeyOn, actionKeyOff, act_low, &note[1]);
    action.add(button_MiddleLeft, actionKeyOn, actionKeyOff, act_low, &note[2]);
    action.add(button_MiddleRight,actionKeyOn, actionKeyOff, act_low, &note[3]);
    action.add(button_BottomLeft, actionKeyOn, actionKeyOff, act_low, &note[4]);
    action.add(button_BottomRight,actionKeyOn, actionKeyOff, act_low, &note[5]);
}

void actionKeyOn(bool active, int pin, void* ptr) {
    if (audioPlayerRunning) {
        float freq = *((float*)ptr);
        sine.setFrequency(freq);
        in.begin();
    }
}

void actionKeyOff(bool active, int pin, void* ptr) {
    in.end();
}

void beepOnBounce() {
    if (!playingBeep) {
        sine.setFrequency(440.0f);  // A4 note as default
        in.begin();
        playingBeep = true;
        beepStart = millis();
    }
}

void drawAudioPlayer() {
    float volumeSlider = float(sliderPosition_Percentage_Filtered) / 100.0f;
    volume.setVolume(volumeSlider);

    if (!audioPlayerRunning) {
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setFont(suiGenerisRg_20);
        display.drawString(64, 10, "Booper");
        display.display();
        audioPlayerRunning = true;
    }
    Serial.print("Volume: " + String(volume.volume()));
}

void loopAudio() {
    copier.copy();
}
