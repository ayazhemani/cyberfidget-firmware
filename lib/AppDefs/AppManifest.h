//#pragma once

// extern void menuBegin();
// extern void menuEnd();
// extern void menuRun();



/**
 * The user (or developer) adds lines here. 
 * The macro is spelled "APP_ENTRY" so we can expand it in multiple ways.
 *
 * Format:
 *   APP_ENTRY(EnumName, "Menu Label", "CategoryPath", beginFunc, endFunc, runFunc)
 *
 * - "CategoryPath" can be something like:
 *       "Screensavers"
 *       "Games"
 *       "Tools/WiFi"
 *       "Music/Player"
 *   etc. 
 *
 * If you want an app at the root, you can specify "" or "Root" or whatever system you prefer.
 */

// Must define APP_ENTRY before including this file
// Example lines:
//APP_ENTRY(APP_MENU,               "Menu",               "",             menuBegin,        menuEnd,        menuRun )
//APP_ENTRY(APP_MATRIX_SCREENSAVER, "Matrix Screensaver", "Screensavers", matrixBegin,      matrixEnd,      matrixUpdate )
//APP_ENTRY(APP_REACTION,           "Reaction Time",      "Games",        reactionBegin,    reactionEnd,    reactionRun )
//APP_ENTRY(APP_BOOPER,             "Booper",             "Games",        booperBegin,      booperEnd,      booperRun )
//APP_ENTRY(APP_WIFI_CONFIG,        "WiFi Config",        "Tools/WiFi",   wifiBegin,        wifiEnd,        wifiRun )
//APP_ENTRY(APP_POWER_MANAGER,      "Power Manager",      "Tools/Power",  powerBegin,       powerEnd,       powerRun )
//APP_ENTRY(APP_FLASHLIGHT,         "Flashlight",         "Tools",        flashlightBegin,  flashlightEnd,  flashlightRun )
//
// ... more lines as needed ...

APP_ENTRY(
    APP_MENU, 
    "", 
    "", 
    [](){ MenuManager::instance().begin(); },
    [](){ MenuManager::instance().end(); },
    [](){ MenuManager::instance().update();}
)

// APP_ENTRY(
//     APP_BOOPER, 
//     "Booper", 
//     "Games", 
//     [](){ Booper::instance().begin(); },
//     [](){ Booper::instance().end(); },
//     [](){ Booper::instance().update();}
// )

APP_ENTRY(
    APP_BOOPER,
    "Booper",
    "Games",
    [](){ booper.begin(); },
    [](){ booper.end(); },
    [](){ booper.update(); }
)

// APP_ENTRY(
//     APP_MENU,
//     "Menu",
//     "",
//     menuBegin,
//     menuEnd,
//     menuRun
// )

APP_ENTRY(
    APP_FLASHLIGHT,
    "Flashlight",
    "Tools/LEDs",
    flashlightBegin,
    flashlightEnd,
    flashlightUpdate
)

APP_ENTRY(
    APP_POWER_MANAGER,
    "Power Manager",
    "Tools",
    powerManagerBegin,
    powerManagerEnd,
    powerManagerUpdate
)

APP_ENTRY(
    APP_MATRIX_SCREENSAVER,
    "Matrix Screensaver",
    "Screensavers",
    [](){ matrixScreensaver.begin(); },
    [](){ matrixScreensaver.end(); },
    [](){ matrixScreensaver.update(); }
)
