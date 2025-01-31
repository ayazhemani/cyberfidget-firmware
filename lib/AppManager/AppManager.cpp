// AppManager.cpp
#include "AppManager.h"
#include "ReactionTimeGame.h"
#include "MatrixScreensaver.h" // Assume this exists
#include "globals.h"

AppManager::AppManager(ButtonManager& btnManager)
    : buttonManager(btnManager), currentApp(None) {}

/**
 * @brief Activate a specific app, ensuring only one app is active at a time
 * 
 * @param app The app to activate
 */
void AppManager::activateApp(ActiveApp app) {
    if (currentApp == app) {
        // App is already active
        return;
    }

    // Deactivate the current app
    deactivateCurrentApp();

    // Activate the new app
    switch (app) {
        case ReactionTimeGameApp:
            // Assuming you have a global ReactionTimeGame instance
            buttonManager.registerCallback(reactionGame.getButtonIndex(), ReactionTimeGame::reactionButtonPressedCallback);
            Serial.println("Reaction Time Game Activated.");
            break;
        
        // case MatrixScreensaverApp:
        //     // Assuming you have a global MatrixScreensaver instance
        //     buttonManager.registerCallback(matrixScreensaver.getButtonIndex(), MatrixScreensaver::screensaverButtonPressedCallback);
        //     Serial.println("Matrix Screensaver Activated.");
        //     break;

        // Handle other apps similarly
        default:
            break;
    }

    currentApp = app;
}

/**
 * @brief Deactivate the currently active app
 */
void AppManager::deactivateCurrentApp() {
    switch (currentApp) {
        case ReactionTimeGameApp:
            buttonManager.unregisterCallback(reactionGame.getButtonIndex());
            Serial.println("Reaction Time Game Deactivated.");
            break;
        
        // case MatrixScreensaverApp:
        //     buttonManager.unregisterCallback(matrixScreensaver.getButtonIndex());
        //     Serial.println("Matrix Screensaver Deactivated.");
        //     break;

        // Handle other apps similarly
        default:
            break;
    }

    currentApp = None;
}
