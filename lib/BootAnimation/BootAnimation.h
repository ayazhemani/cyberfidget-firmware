#ifndef BOOTANIMATION_H
#define BOOTANIMATION_H

#include "ButtonManager.h"
#include "MenuManager.h"

void drawBootAnimation();

class BootAnimationApp {
    public:
        BootAnimationApp(ButtonManager& btnMgr);
        void begin();
        void end();
        void update();
        void timeout();
    
        static void onButtonBackPressed(const ButtonEvent& event);
        static BootAnimationApp* instance;
    
    private:
        ButtonManager& buttonManager;
    };

extern BootAnimationApp bootAnimationApp;

#endif // BOOTANIMATION_H