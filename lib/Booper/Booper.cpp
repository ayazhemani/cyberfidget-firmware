#include "Booper.h"
#include "globals.h" // For slider position and button indices
#include "HAL.h"  // Use our proxy header
#include "MenuManager.h" // For returning back to menu

Booper* Booper::instance = nullptr;

Booper::Booper(ButtonManager& btnMgr, AudioManager& audioMgr) :
    buttonManager(btnMgr),
    audioManager(audioMgr),
    display(HAL::displayProxy()),  // Use the DisplayProxy from HAL
    volume(0.5f),
    octave(0),
    toneStopTime(0)
{
    instance = this;
}

void Booper::begin() {
    ESP_LOGI(TAG_MAIN, "[Booper] begin() => registering booper callbacks...");
    registerButtonCallbacks();
}

void Booper::end() {
    ESP_LOGI(TAG_MAIN, "[Booper] end() => unregistering booper callbacks...");
    unregisterButtonCallbacks();
    // Stop any playing tones
    audioManager.stopTone();
}

void Booper::update() {
    // Update volume from slider
    updateVolumeFromSlider();

    // No need to manage toneStopTime if tone plays only while button is pressed
    // Remove toneStopTime handling if not used
}

void Booper::draw() {
    display.clear();

    // Display current volume and octave
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 10, "Booper");
    display.drawString(64, 22, "Volume: " + String((int)(volume * 100)) + "%");
    display.drawString(64, 34, "Octave: " + String(octave));

    display.display();
}

void Booper::registerButtonCallbacks() {
    // Register callbacks for the buttons
    buttonManager.registerCallback(button_TopLeftIndex,     buttonPressedCallback);
    buttonManager.registerCallback(button_TopRightIndex,    buttonPressedCallback);
    buttonManager.registerCallback(button_MiddleLeftIndex,  buttonPressedCallback);
    buttonManager.registerCallback(button_MiddleRightIndex, buttonPressedCallback);

    // Use TopLeft and TopRight buttons to adjust octave
    // buttonManager.registerCallback(button_TopLeftIndex, buttonPressedCallback);
    // buttonManager.registerCallback(button_TopRightIndex, buttonPressedCallback);

    // Exit App
    buttonManager.registerCallback(button_BottomLeftIndex, onButtonBackPressed);
    buttonManager.registerCallback(button_BottomRightIndex,onButtonSelectPressed);
}

void Booper::unregisterButtonCallbacks() {
    // Unregister callbacks
    buttonManager.unregisterCallback(button_TopLeftIndex);
    buttonManager.unregisterCallback(button_TopRightIndex);
    buttonManager.unregisterCallback(button_MiddleLeftIndex);
    buttonManager.unregisterCallback(button_MiddleRightIndex);
    buttonManager.unregisterCallback(button_BottomLeftIndex);
    buttonManager.unregisterCallback(button_BottomRightIndex);
}

void Booper::buttonPressedCallback(const ButtonEvent& event) {
    if (instance) {
        instance->handleButtonEvent(event);
    }
}

void Booper::handleButtonEvent(const ButtonEvent& event) {
    // Handle button events
    if (event.eventType == ButtonEvent_Pressed) {
        // if (event.buttonIndex == button_TopLeftIndex) {
        //     // Decrease octave
        //     adjustOctave(-1);
        // } else if (event.buttonIndex == button_TopRightIndex) {
        //     // Increase octave
        //     adjustOctave(1);
        // } else {
        //     // Play tone for other buttons
        //     float freq = getFrequencyForButton(event.buttonIndex);
        //     audioManager.playTone(freq);
        //     // toneStopTime = 0; // Not needed if we stop tone on button release
        // }

        // Play tone for other buttons
        float freq = getFrequencyForButton(event.buttonIndex);
        audioManager.playTone(freq);
        // toneStopTime = 0; // Not needed if we stop tone on button release
    } else if (event.eventType == ButtonEvent_Released) {
        // Stop tone when button is released
        if (event.buttonIndex == button_TopLeftIndex ||
            event.buttonIndex == button_TopRightIndex ||
            event.buttonIndex == button_MiddleLeftIndex ||
            event.buttonIndex == button_MiddleRightIndex) {
            audioManager.stopTone();
        }
    }
}

float Booper::getFrequencyForButton(int buttonIndex) {
    // Base frequencies for buttons
    float baseFrequencies[] = { 261.63f, 293.66f, 329.63f, 349.23f }; // C4, D4, E4, F4

    int buttonOrder[] = {
        button_TopLeftIndex,
        button_TopRightIndex,
        button_MiddleLeftIndex,
        button_MiddleRightIndex
    };

    // Find index of the button
    int idx = -1;
    for (int i = 0; i < 4; i++) {
        if (buttonIndex == buttonOrder[i]) {
            idx = i;
            break;
        }
    }

    if (idx >= 0) {
        // Adjust frequency based on octave
        float freq = baseFrequencies[idx] * pow(2, octave);
        return freq;
    } else {
        return 440.0f; // Default frequency
    }
}



void Booper::adjustOctave(int delta) {
    octave += delta;
    // Constrain octave between -2 and +2, for example
    octave = constrain(octave, -2, 2);
}

void Booper::updateVolumeFromSlider() {
    // Assume sliderPosition_Percentage_Filtered is a global variable [0, 100]
    volume = sliderPosition_Percentage_Inverted_Filtered / 100.0f;
    audioManager.setVolume(volume);
}



// Static callback handlers:
// void Booper::onButtonLeftPressed(const ButtonEvent& event)
// {
//     // Example: do nothing or implement "move to previous root category"? 
//     // Since it's a nested menu, left/right might not do anything if you only have
//     // vertical list navigation. Feel free to define your own logic.
// }
// void Booper::onButtonRightPressed(const ButtonEvent& event)
// {
//     // Press
//     if (event.eventType == ButtonEvent_Pressed){
//         // Same as above
//     }    
// }
// void Booper::onButtonUpPressed(const ButtonEvent& event)
// {
//     // Press
//     if (event.eventType == ButtonEvent_Pressed){
//         instance().moveHighlightUp();
//     }
// }
// void Booper::onButtonDownPressed(const ButtonEvent& event)
// {
//     // Press
//     if (event.eventType == ButtonEvent_Pressed){
//         instance().moveHighlightDown();
//     }
// }
void Booper::onButtonBackPressed(const ButtonEvent& event)
{    // Press
    if (event.eventType == ButtonEvent_Released){
        ESP_LOGI(TAG_MAIN, "[Booper] onButtonBackPressed => calling end() + returning to menu...");
        instance->end();
        MenuManager::instance().returnToMenu();
    } 
}
void Booper::onButtonSelectPressed(const ButtonEvent& event)
{    
    // Press
    if (event.eventType == ButtonEvent_Pressed){
        //instance().selectCurrentItem();
    }
}
