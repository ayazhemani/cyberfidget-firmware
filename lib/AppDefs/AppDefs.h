#ifndef APP_DEFS_H
#define APP_DEFS_H

#include <functional>

/**
 * @brief Each app implements these three functions:
 *   begin()  -> registers callbacks, initializes
 *   end()    -> unregisters callbacks, tear-down
 *   run()    -> or draw(), called periodically (like your 20ms cycle)
 *
 * The function pointers below let us store them all in a table.
 */
using AppBeginFunc = std::function<void()>;
using AppEndFunc   = std::function<void()>;
using AppRunFunc   = std::function<void()>;


// 1) The enumerated list of all apps, 
//    including the Menu if you like:
enum AppIndex {
    APP_MENU = 0,
    APP_BOOPER,
    APP_DINO_GAME,
    APP_MATRIX_SCREENSAVER,
    APP_REACTION,
    APP_SIMON_SAYS,
    APP_BREAKOUT,
    APP_CLOCK_DISPLAY,
    APP_WIFI_CONFIG,
    APP_POWER_MANAGER,
    APP_COUNT // must be last
};

/**
 * @brief A struct describing a single app. 
 */
struct AppDefinition {
    const char*    name;      // optional: app name if you want
    AppBeginFunc   beginFunc;
    AppEndFunc     endFunc;
    AppRunFunc     runFunc;   // or "drawFunc"
};


/**
 * @brief The actual registry array. 
 * We’ll define it in AppDefs.cpp
 */
extern AppDefinition appRegistry[APP_COUNT];

#endif
