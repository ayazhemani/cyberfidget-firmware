#include "StratagemGame.h"
#include "globals.h"      // For your button_*Index definitions
#include "HAL.h"          // For display, button, audio, etc.

// -----------------------------------------------------
// Declare the global instance
// -----------------------------------------------------
StratagemGame stratagemGame(HAL::buttonManager(), HAL::audioManager());

// -----------------------------------------------------
// Static instance pointer definition
// -----------------------------------------------------
StratagemGame* StratagemGame::instance = nullptr;

// -----------------------------------------------------
// We’ll embed the entire HD2-sequences JSON data as a static array.
// Each item has a name, a sequence string, and an image placeholder.
// If you have too many items for memory, consider storing them in PROGMEM (Flash).
// -----------------------------------------------------
static const Stratagem s_stratagemData[] = {
    //  Example subset from HD2-Sequences.json
    //  If you want to include them all, just replicate each object here.
    //  For demonstration, here are a few:
    { "Reinforce",             "UDRLU",   "reinforce.svg" },
    { "Resupply",              "DDUR",    "resupply.svg" },
    { "SOS Beacon",            "UDRU",    "sos_beacon.svg" },
    { "SEAF Artillery",        "RUUD",    "seaf_artillery.svg" },
    { "Hellbomb",              "DULDURDU","hellbomb.svg" },
    { "Prospecting Drill",     "DDLRDD",  "prospecting_drill.svg" },
    { "Orbital Gatling",       "RDLUU",   "orbital_gatling_barrage.svg" },
    { "Eagle Airstrike",       "URDR",    "eagle_airstrike.svg" },
    { "Jump Pack",             "DUUDU",   "jump_pack.svg" },
    { "Stalwart",              "DLDUUL",  "stalwart.svg" },
    // ... continue adding the rest from HD2-Sequences.json as needed ...
};

// The number of items in our array:
const int StratagemGame::NUM_STRATAGEMS = sizeof(s_stratagemData) / sizeof(s_stratagemData[0]);

// -----------------------------------------------------
// Constructor
// -----------------------------------------------------
StratagemGame::StratagemGame(ButtonManager &buttonMgr, AudioManager &audioMgr)
    : display(HAL::displayProxy())
    , buttonManager(buttonMgr)
    , audioManager(audioMgr)
{
    instance = this;
    currentState          = WAIT_START;
    score                 = 0;
    timeRemaining         = TOTAL_TIME;
    lastUpdateTime        = 0;
    hitlagStartTime       = 0;
    currentSequenceIndex  = 0;
    currentStratagem      = { "None", "", "placeholder.png" };
}

// -----------------------------------------------------
// begin()
// -----------------------------------------------------
void StratagemGame::begin() 
{
    // Register callbacks for the four directions 
    // (assuming your actual hardware indices are in your “globals.h” or “HAL.h”).
    // For example, let's say:
    buttonManager.registerCallback(button_TopLeftIndex,    onButtonEvent);
    buttonManager.registerCallback(button_TopRightIndex,   onButtonEvent);
    buttonManager.registerCallback(button_MiddleLeftIndex, onButtonEvent);
    buttonManager.registerCallback(button_MiddleRightIndex,onButtonEvent);

    // Register “Back” button
    buttonManager.registerCallback(button_BottomLeftIndex, onButtonBackPressed);

    // Reset game state
    resetGame();
}

// -----------------------------------------------------
// end()
// -----------------------------------------------------
void StratagemGame::end()
{
    // Unregister callbacks
    buttonManager.unregisterCallback(button_TopLeftIndex);
    buttonManager.unregisterCallback(button_TopRightIndex);
    buttonManager.unregisterCallback(button_MiddleLeftIndex);
    buttonManager.unregisterCallback(button_MiddleRightIndex);
    buttonManager.unregisterCallback(button_BottomLeftIndex);

    // Stop any playing audio
    audioManager.stopTone();
    //audioManager.stopFile();
}

// -----------------------------------------------------
// update() - main loop
// -----------------------------------------------------
void StratagemGame::update()
{
    unsigned long now = millis();

    switch (currentState) {
    case WAIT_START:
        // Draw idle screen
        drawScreen();
        break;

    case RUNNING: {
        // Calculate time delta
        if (lastUpdateTime == 0) {
            // First time we enter RUNNING
            lastUpdateTime = now;
        }
        unsigned long delta = now - lastUpdateTime;
        lastUpdateTime = now;

        // Decrease time (but clamp if underflow)
        if (delta < timeRemaining) {
            timeRemaining -= delta;
        } else {
            timeRemaining = 0;
        }

        // If time runs out -> game over
        if (timeRemaining == 0) {
            gameOver();
            return;
        }

        // Draw game screen
        drawScreen();
        break;
    }

    case HITLAG: {
        // Just wait in this state for HITLAG_TIMEOUT ms
        handleHitlag();
        break;
    }

    case GAME_OVER:
        // Show final screen (score)
        drawGameOver();
        // We might remain here until user taps the Back button
        // or we could automatically reset after some time. 
        break;
    }
}

// -----------------------------------------------------
// resetGame()
// -----------------------------------------------------
void StratagemGame::resetGame()
{
    currentState         = WAIT_START;
    score                = 0;
    timeRemaining        = TOTAL_TIME;
    lastUpdateTime       = 0;
    hitlagStartTime      = 0;
    currentSequenceIndex = 0;
}

// -----------------------------------------------------
// Called when we need a new random stratagem
// -----------------------------------------------------
void StratagemGame::pickRandomStratagem()
{
    if (NUM_STRATAGEMS == 0) {
        // fallback if no data
        currentStratagem = { "NO DATA", "", "placeholder.png" };
        return;
    }
    int index = random(0, NUM_STRATAGEMS);
    currentStratagem = s_stratagemData[index];
    currentSequenceIndex = 0;
}

// -----------------------------------------------------
// onButtonEvent - handle one of the direction buttons
// -----------------------------------------------------
void StratagemGame::onButtonEvent(const ButtonEvent& event)
{
    if (!instance) return;
    if (event.eventType != ButtonEvent_Pressed) return; // Only act on press

    // If we are WAIT_START, pressing any direction starts the game
    if (instance->currentState == WAIT_START) {
        instance->currentState = RUNNING;
        instance->pickRandomStratagem();
        instance->lastUpdateTime = millis(); 
        return;
    }

    // If we are in GAME_OVER, do nothing for directions
    // (or you could choose to allow a button press to reset)
    if (instance->currentState == GAME_OVER) {
        return;
    }

    // If we are in HITLAG, ignore new inputs
    if (instance->currentState == HITLAG) {
        return;
    }

    // So we must be in RUNNING state:
    int btn = event.buttonIndex;
    char direction = '\0';

    // Map hardware button index to 'U','D','L','R'
    // This is your own mapping; adapt if needed:
    if      (btn == button_TopLeftIndex)      direction = 'L'; 
    else if (btn == button_TopRightIndex)     direction = 'R';
    else if (btn == button_MiddleLeftIndex)   direction = 'U';
    else if (btn == button_MiddleRightIndex)  direction = 'D';
    else return;

    // Check input
    instance->checkInput(direction);
}

// -----------------------------------------------------
// onButtonBackPressed - handle the “Back”/exit button
// -----------------------------------------------------
void StratagemGame::onButtonBackPressed(const ButtonEvent &event)
{
    if (!instance) return;
    if (event.eventType == ButtonEvent_Released) {
        // We want to end and return to menu
        instance->end();
        MenuManager::instance().returnToMenu();
    }
}

// -----------------------------------------------------
// checkInput() - compare pressed direction vs next in sequence
// -----------------------------------------------------
void StratagemGame::checkInput(char direction)
{
    // Safety
    if (currentStratagem.sequence == nullptr || strlen(currentStratagem.sequence) == 0) {
        return;
    }

    // Next needed char
    char needed = currentStratagem.sequence[currentSequenceIndex];

    if (direction == needed) {
        // Correct
        currentSequenceIndex++;
        // Play success sound for direction
        playDirectionSound(direction);

        // Did we finish the entire sequence?
        if (currentSequenceIndex >= (int)strlen(currentStratagem.sequence)) {
            // Add to score
            score++;

            // Add time bonus
            timeRemaining += TIME_BONUS;

            // Move to HITLAG
            currentState = HITLAG;
            hitlagStartTime = millis();

            // Stop tracking delta for a moment
            lastUpdateTime = 0;
        }
    } else {
        // Wrong -> reset partial progress to 0
        currentSequenceIndex = 0;
        // Possibly play an error beep
        audioManager.playTone(200.0f); // short beep for “error”
    }
}

// -----------------------------------------------------
// handleHitlag() - after completing a stratagem sequence
// we pause for a short time, then pick a new stratagem
// and return to RUNNING
// -----------------------------------------------------
void StratagemGame::handleHitlag()
{
    unsigned long now = millis();
    if (now - hitlagStartTime >= HITLAG_TIMEOUT) {
        // Move on
        currentState = RUNNING;
        pickRandomStratagem();
        lastUpdateTime = now; 
    }
}

// -----------------------------------------------------
// gameOver() 
// -----------------------------------------------------
void StratagemGame::gameOver()
{
    currentState = GAME_OVER;
    audioManager.playTone(110.0f, 500); // e.g. low beep
    // or if you have an MP3 for game over, do: audioManager.playFile("/sounds/game_over.mp3");
}

// -----------------------------------------------------
// drawScreen() - updates the display
// -----------------------------------------------------
void StratagemGame::drawScreen()
{
    display.clear();

    if (currentState == WAIT_START) {
        display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        display.drawString(64, 32, "Press any arrow\n to START");
        display.display();
        return;
    }

    if (currentState == RUNNING || currentState == HITLAG) {
        // Show the current stratagem name
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(0, 0, String("Stratagem: ") + currentStratagem.name);

        // Show the partial progress (like “UD_R_” for example)
        // We'll show the entire sequence, highlighting how many steps are done
        String seqDisplay;
        int len = strlen(currentStratagem.sequence);
        for (int i = 0; i < len; i++) {
            if (i < currentSequenceIndex) {
                // Mark completed step with parentheses or something
                seqDisplay += "(";
                seqDisplay += currentStratagem.sequence[i];
                seqDisplay += ") ";
            } else {
                seqDisplay += currentStratagem.sequence[i];
                seqDisplay += " ";
            }
        }
        display.drawString(0, 12, String("Seq: ") + seqDisplay);

        // Show time & score
        display.drawString(0, 24, String("Time: ") + (timeRemaining/1000.0f,1));
        display.drawString(0, 36, String("Score: ") + score);

        // If you want an optional image placeholder, you can do so here,
        // or store small 8x8 / 16x16 bitmaps in PROGMEM.

        display.display();
        return;
    }
}

// -----------------------------------------------------
// drawGameOver() - final screen
// -----------------------------------------------------
void StratagemGame::drawGameOver()
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);

    display.drawString(64, 20, "GAME OVER!");
    display.drawString(64, 40, String("Score: ") + score);

    // Optionally instruct user to press BACK to exit
    display.drawString(64, 56, "Press Back");

    display.display();
}

// -----------------------------------------------------
// playDirectionSound() 
// For each direction, you can either beep or play an MP3
// if your system supports file-based playback.
// -----------------------------------------------------
void StratagemGame::playDirectionSound(char direction)
{
    // Simple approach: beep with different frequencies
    switch (direction) {
        case 'U': audioManager.playTone(523.25f, 150); break; // C5
        case 'D': audioManager.playTone(329.63f, 150); break; // E4
        case 'L': audioManager.playTone(392.00f, 150); break; // G4
        case 'R': audioManager.playTone(261.63f, 150); break; // C4
        default: break;
    }
}
