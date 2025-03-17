#include "AppDefs.h"
#include "MenuManager.h" // If you keep your menu code in a single file
#include "Booper.h"
#include "DinoGame.h"
#include "ClockDisplay.h"
#include "Flashlight.h"

// etc.

AppDefinition appRegistry[APP_COUNT] = {
    /* [APP_MENU] */
    {
       "Menu",
       [](){ MenuManager::instance().begin(); }, // beginFunc
       [](){ MenuManager::instance().end(); },   // endFunc
       [](){ MenuManager::instance().update(); } // runFunc
    },
    /* [APP_BOOPER] */
    {
       "Booper",
       [](){ booper.begin(); },         // or [](){ appNameGlobalReference.begin(); }
       [](){ booper.end();   },         // or [](){ appNameGlobalReference.end(); }
       [](){ booper.update();   },      // or appNameGlobalReference, etc.
    },
    /* [APP_DINO] */
    {
       "Dino",
       [](){ dinoGame.begin(); },
       [](){ dinoGame.end();   },
       [](){ dinoGame.update();   },
    },
    /* [APP_CLOCK_DISPLAY] */
    {
      "Clock",
      [](){ clockDisplay.begin(); },
      [](){ clockDisplay.end();   },
      [](){ clockDisplay.update();   },
      },
   /* [APP_FLASHLIGHT] */
    {
      "Flashlight",
      [](){ flashlight.begin(); },
      [](){ flashlight.end();   },
      [](){ flashlight.update();   },
      },
    // ...
};


/*
TEMPLATE GUIDE
---

1) If you have in Booper.h:

class Booper {
public:
   void begin();
   void update();
   void end();
   ...
};

// some global
extern Booper booper;  // declared in Booper.h

---
2) Then in Booper.cpp:

#include "Booper.h"
#include "HAL.h"
Booper booper(HAL::buttonManager(), HAL::audioManager());

Booper::Booper(ButtonManager&, AudioManager&) { ... }
void Booper::begin() { ... }
void Booper::update() { ... }
void Booper::end() { ... }

---
3) Finally in AppDefs.cpp:

#include "Booper.h"  // so we see extern Booper booper
#include "AppDefs.h"

AppDefinition appRegistry[APP_COUNT] = {
    // ...
    [APP_BOOPER] = {
        "Booper",
        [](){ booper.begin(); },
        [](){ booper.end();   },
        [](){ booper.update();}
    },
    // ...
};

-------

Extra Hints

either do lambdas in AppDefs.cpp:

[APP_BOOPER] = {
   "Booper",
   [](){ booper.begin(); },
   [](){ booper.end(); },
   [](){ booper.update(); }
};

Or define free functions:

// In BooperApp.h
void booperBegin();
void booperEnd();
void booperUpdate();

// In BooperApp.cpp
#include "Booper.h"
Booper booper(...);

void booperBegin()  { booper.begin(); }
void booperEnd()    { booper.end(); }
void booperUpdate() { booper.update(); }

// Then in AppDefs.cpp
[APP_BOOPER] = {
   "Booper",
   booperBegin,
   booperEnd,
   booperUpdate
};

*/