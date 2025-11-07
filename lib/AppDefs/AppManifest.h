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

// MUST include app in AppManifest_Includes.h if it is not already included
//        APP_NAME                   "Menu Label",          "Menu/Path",      beginFunc,                                     endFunc,                                           runFunc
APP_ENTRY(APP_BOOT_ANIMATION,        "Boot Animation",      "Screensavers",   [](){ bootAnimationApp.begin();                }, [](){ bootAnimationApp.end();                }, [](){ bootAnimationApp.update();                })
APP_ENTRY(APP_MENU,                  "",                    "",               [](){ MenuManager::instance().begin();         }, [](){ MenuManager::instance().end();         }, [](){ MenuManager::instance().update();         })
APP_ENTRY(APP_BOOPER,                "Booper",              "Games",          [](){ booper.begin();                          }, [](){ booper.end();                          }, [](){ booper.update();                          })
APP_ENTRY(APP_FLASHLIGHT,            "Flashlight",          "Tools/LEDs",           flashlightBegin,                                  flashlightEnd,                                  flashlightUpdate                           )
APP_ENTRY(APP_POWER_MANAGER,         "Power Manager",       "Tools",                powerManagerBegin,                                powerManagerEnd,                                powerManagerUpdate                         )
APP_ENTRY(APP_MATRIX_SCREENSAVER,    "Matrix Screensaver",  "Screensavers",   [](){ matrixScreensaver.begin();               }, [](){ matrixScreensaver.end();               }, [](){ matrixScreensaver.update();               })
APP_ENTRY(APP_BREAKOUT_GAME,         "Breakout",            "Games",          [](){ breakoutGame.begin();                    }, [](){ breakoutGame.end();                    }, [](){ breakoutGame.update();                    })
APP_ENTRY(APP_DINO_GAME,             "Dino Run",            "Games",          [](){ dinoGame.begin();                        }, [](){ dinoGame.end();                        }, [](){ dinoGame.update();                        })
APP_ENTRY(APP_CLOCK_DISPLAY,         "Clock",               "Tools",          [](){ clockDisplay.begin();                    }, [](){ clockDisplay.end();                    }, [](){ clockDisplay.update();                    })
APP_ENTRY(APP_SIMON_SAYS_GAME,       "Simon Says",          "Games",          [](){ simonSaysGame.begin();                   }, [](){ simonSaysGame.end();                   }, [](){ simonSaysGame.update();                   })
APP_ENTRY(APP_SPH_FLUID_GAME,        "Particle Sim",        "Games",          [](){ sphFluidGame.begin();                    }, [](){ sphFluidGame.end();                    }, [](){ sphFluidGame.update();                    })
APP_ENTRY(APP_EXAMPLE_FONTFACE,      "Font Face Demo",      "Examples",       [](){ fontFaceApp.begin();                     }, [](){ fontFaceApp.end();                     }, [](){ fontFaceApp.update();                     })
APP_ENTRY(APP_EXAMPLE_TEXTFLOW,      "Text Flow Demo",      "Examples",       [](){ textFlowApp.begin();                     }, [](){ textFlowApp.end();                     }, [](){ textFlowApp.update();                     })
APP_ENTRY(APP_EXAMPLE_TEXTALIGNMENT, "Text Alignment Demo", "Examples",       [](){ textAlignmentApp.begin();                }, [](){ textAlignmentApp.end();                }, [](){ textAlignmentApp.update();                })
APP_ENTRY(APP_EXAMPLE_RECT,          "Rect Demo",           "Examples",       [](){ rectApp.begin();                         }, [](){ rectApp.end();                         }, [](){ rectApp.update();                         })
APP_ENTRY(APP_EXAMPLE_CIRCLE,        "Circle Demo",         "Examples",       [](){ circleApp.begin();                       }, [](){ circleApp.end();                       }, [](){ circleApp.update();                       })
APP_ENTRY(APP_EXAMPLE_IMG1,          "Image Demo 1",        "Examples",       [](){ imageDemo1App.begin();                   }, [](){ imageDemo1App.end();                   }, [](){ imageDemo1App.update();                   })
APP_ENTRY(APP_EXAMPLE_IMG2,          "Image Demo 2",        "Examples",       [](){ imageDemo2App.begin();                   }, [](){ imageDemo2App.end();                   }, [](){ imageDemo2App.update();                   })
APP_ENTRY(APP_EXAMPLE_IMG3,          "Image Demo 3",        "Examples",       [](){ imageDemo3App.begin();                   }, [](){ imageDemo3App.end();                   }, [](){ imageDemo3App.update();                   })
APP_ENTRY(APP_EXAMPLE_IMG4,          "Image Demo 4",        "Examples",       [](){ imageDemo4App.begin();                   }, [](){ imageDemo4App.end();                   }, [](){ imageDemo4App.update();                   })
APP_ENTRY(APP_EXAMPLE_BATTERYBAR,    "Battery Level",       "Tools",          [](){ batteryBarApp.begin();                   }, [](){ batteryBarApp.end();                   }, [](){ batteryBarApp.update();                   })
APP_ENTRY(APP_EXAMPLE_SLIDERBAR,     "Slider Status",       "Tools",          [](){ sliderBarApp.begin();                    }, [](){ sliderBarApp.end();                    }, [](){ sliderBarApp.update();                    })
APP_ENTRY(APP_EXAMPLE_ACCELEROMETER, "Accelerometer Demo",  "Tools/LEDs",     [](){ accelerometerApp.begin();                }, [](){ accelerometerApp.end();                }, [](){ accelerometerApp.update();                })
APP_ENTRY(APP_EXAMPLE_BUTTONCOUNTERS,"Button Counters",     "Examples",       [](){ buttonCountersApp.begin();               }, [](){ buttonCountersApp.end();               }, [](){ buttonCountersApp.update();               })
APP_ENTRY(APP_EXAMPLE_TIMEONCOUNTER, "Time On Counter",     "Examples",       [](){ timeOnCounterApp.begin();                }, [](){ timeOnCounterApp.end();                }, [](){ timeOnCounterApp.update();                })
APP_ENTRY(APP_EXAMPLE_PROGRESSBAR,   "Progress Bar",        "Examples",       [](){ progressBarApp.begin();                  }, [](){ progressBarApp.end();                  }, [](){ progressBarApp.update();                  })
APP_ENTRY(APP_REACTION_TIME,         "Reaction Time",       "Games",          [](){ reactionTimeGame.begin();                }, [](){ reactionTimeGame.end();                }, [](){ reactionTimeGame.update();                })
APP_ENTRY(APP_WIFI_MANAGER,          "WiFi Manager",        "Tools",          [](){ wifiManagerCF_App.begin();               }, [](){ wifiManagerCF_App.end();               }, [](){ wifiManagerCF_App.update();               })
APP_ENTRY(APP_SERIAL_DISPLAY,        "Serial Display",      "Tools",          [](){ serialDisplayApp.begin();                }, [](){ serialDisplayApp.end();                }, [](){ serialDisplayApp.update();                })
APP_ENTRY(APP_STRATAGEM_GAME,        "Stratagem",           "Games",          [](){ stratagemGame.begin();                   }, [](){ stratagemGame.end();                   }, [](){ stratagemGame.update();                   })
APP_ENTRY(APP_GRAVEYARDSCREENSAVER,  "Graveyard",           "Screensavers",   [](){ graveyardScreensaverApp.begin();         }, [](){ graveyardScreensaverApp.end();         }, [](){ graveyardScreensaverApp.update();         })

//
// ... more lines as needed ...