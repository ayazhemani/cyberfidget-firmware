#include "AppDefs.h"
#include "MenuManager.h"  // if you want to insert items into the menu
#include <vector>
#include <string>
#include <cstring>

// 1) Build the array with the lines from "AppManifest.h"
#define APP_ENTRY(ID, LABEL, CATPATH, BEGINF, ENDF, RUNF) \
    { LABEL, CATPATH, BEGINF, ENDF, RUNF },

AppDefinition appDefs[APP_COUNT] = {
    #include "AppManifest.h"  // each line expands into { LABEL, PATH, begin..., end..., run... }
};

#undef APP_ENTRY

// 2) (Optional) A function to parse each categoryPath
static void addAppToMenu(const char* label, const char* path, int appIndex)
{
    // For example, pass it to your MenuManager or something. We'll do a minimal example:
    // We might store in some global data structure “menuRoot”
    // Then parse path by splitting on '/'
    // E.g. "Tools/WiFi" => [ "Tools", "WiFi" ]
    // Then go create subcategories if needed, etc.

    // Pseudocode for path splitting:
    // Tools => Subcategory "WiFi" => add item "label" => appIndex
    // We'll keep it simple:
    // MenuManager::instance().addItem(path, label, (AppIndex)appIndex);

    // We'll do a simple placeholder:
    MenuManager::instance().registerApp(path, label, (AppIndex)appIndex);
}

void buildNestedMenu()
{
    for (int i = 0; i < (int)APP_COUNT; i++) {
        addAppToMenu(appDefs[i].name, appDefs[i].categoryPath, i);
    }
}
