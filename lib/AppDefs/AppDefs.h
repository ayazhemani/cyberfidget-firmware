// AppDefs.h
#ifndef APP_DEFS_H
#define APP_DEFS_H

#include <functional>
#include <vector>
#include <string>

/**
 * @brief The function pointer types for an app's lifecycle
 */
using AppBeginFunc = std::function<void()>;
using AppEndFunc   = std::function<void()>;
using AppRunFunc   = std::function<void()>;

struct AppDefinition {
    const char*  name;       // "Matrix Screensaver"
    const char*  categoryPath; // e.g. "Screensavers", or "Tools/WiFi"
    AppBeginFunc beginFunc;
    AppEndFunc   endFunc;
    AppRunFunc   runFunc;
};

/**
 * We'll generate the enum from the lines in AppManifest.h
 */
enum AppIndex {
    #define APP_ENTRY(ID, LABEL, CATPATH, BEGINF, ENDF, RUNF) ID,
    #include "AppManifest.h"
    #undef APP_ENTRY

    APP_COUNT  // final sentinel
};

/**
 * We'll also declare a global array appDefs, 
 * each entry has (name, categoryPath, beginFunc, endFunc, runFunc)
 */
extern AppDefinition appDefs[APP_COUNT];

/**
 * We'll define a function to build the nested menu from these definitions. 
 * We'll show an example of how to parse the categoryPath into subcategories.
 */
void buildNestedMenu();

#endif // APP_DEFS_H
