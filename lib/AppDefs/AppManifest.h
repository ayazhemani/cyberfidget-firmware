/**
 * CyberFidget Music Player - App Manifest
 * Stripped to essential apps only for IRAM budget with BT A2DP audio stack.
 *
 * Format:
 *   APP_ENTRY(EnumName, "Menu Label", "CategoryPath", beginFunc, endFunc, runFunc)
 */

// MUST include app in AppManifest_Includes.h if it is not already included
//        APP_NAME                   "Menu Label",          "Menu/Path",      beginFunc,                                     endFunc,                                           runFunc
APP_ENTRY(APP_BOOT_ANIMATION,        "Boot Animation",      "Screensavers",   [](){ bootAnimationApp.begin();                }, [](){ bootAnimationApp.end();                }, [](){ bootAnimationApp.update();                })
APP_ENTRY(APP_MENU,                  "",                    "",               [](){ MenuManager::instance().begin();         }, [](){ MenuManager::instance().end();         }, [](){ MenuManager::instance().update();         })
APP_ENTRY(APP_POWER_MANAGER,         "Power Manager",       "Tools",                powerManagerBegin,                                powerManagerEnd,                                powerManagerUpdate                         )
APP_ENTRY(APP_SPACESHIP,             "Spaceship",           "Games",          [](){ spaceshipApp.begin();                    }, [](){ spaceshipApp.end();                    }, [](){ spaceshipApp.update();                    })
APP_ENTRY(APP_MUSIC_PLAYER,          "Music Player",        "Media",          [](){ musicPlayerApp.begin();                  }, [](){ musicPlayerApp.end();                  }, [](){ musicPlayerApp.update();                  })
