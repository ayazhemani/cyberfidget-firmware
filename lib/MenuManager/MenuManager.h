#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <string>

// Forward declarations or includes for UIElement/Animation:
#include "UIElement.h"
#include "Animation.h"
#include "HAL.h"          // So we can reference buttonManager, displayProxy, etc.
#include "AppManager.h"    // We need to launch apps from here
#include "ButtonManager.h" // For button callbacks

/**
 * @brief A struct to represent a single menu item.
 *
 * If isCategory == true, then "children" are sub-items to display
 * when this category is selected.
 *
 * If isCategory == false, then this item is actually an "app." 
 * Selecting it will launch the associated appIndex from your existing
 * AppManager (the enumerated “AppIndex”).
 */
struct MenuItem
{
    std::string label;
    bool isCategory;
    AppIndex appIndex;              // Used only if isCategory == false
    std::vector<MenuItem> children; // Sub-items if this is a category
};

/**
 * @brief The MenuManager class is responsible for:
 *  - Storing a hierarchy of categories / sub-items
 *  - Tracking which category or sub-menu is active
 *  - Handling button presses for up/down/left/right/back/select
 *  - Smoothly animating the highlight UIElement
 *  - Launching an App when a user selects a menu item that is not a category
 *  - Providing a "returnToMenu()" callback for apps that want to hand back control
 */
class MenuManager
{
public:
    /**
     * @brief Singleton accessor (optional).
     * We often want a single global MenuManager.
     */
    static MenuManager &instance()
    {
        static MenuManager _instance;
        return _instance;
    }

    /**
     * @brief Initialize the menu system. 
     * - Register button callbacks for navigation.
     * - Build the top-level items (Screensavers, Games, Tools) plus their children.
     */
    void begin();

    /**
     * @brief Call this repeatedly inside the main loop (e.g., from AppManager::loop())
     * so that animations can update and the menu can redraw if needed.
     */
    void update();

    /**
     * @brief Close the menu system
     *  - Unregister button callbacks for navigation.
     */
    void end();

    /**
     * @brief Called by an app to signal "I am done; go back to the menu."
     * This re-registers the menu's button callbacks and shows the last-known highlight.
     *  - Can also call a specific app to switch to, including the menu:
     *  - AppManager::instance().switchToApp(APP_MENU);
     */
    void returnToMenu();

    /**
     * @brief Utility to know if the menu is currently the "active" UI,
     * as opposed to an app being active.
     */
    bool isMenuActive() const { return menuActive; }

private:
    // Private constructor: we use the singleton pattern above
    MenuManager();

    // The "root" vector of top-level categories: Screensavers, Games, Tools
    std::vector<MenuItem> rootMenuItems;

    // Current "path" in the menu. For instance:
    //   If we're at root and have 3 categories, we have rootMenuItems for each category
    //   If we select "Games," we move deeper to the children inside "Games."
    // We keep track of the "current vector of items" and "current selected index" at each level.
    std::vector<MenuItem> *currentItemList = nullptr;
    int currentIndex = 0;      // which item is highlighted in the currentItemList

    // If we navigate into a sub-menu, we push state here so we can go back
    struct MenuNavState {
        std::vector<MenuItem> *itemList;
        int index;
    };
    std::vector<MenuNavState> navigationStack;

    // Our highlight UIElement for the current selected item
    // We'll animate its position whenever the user moves up/down.
    UIElement highlightElement;

    // Keep track of where each item is drawn on screen.
    // This example: each item is drawn in rows, so we store item Y positions.
    // That way we can animate the highlight from row to row.
    std::vector<int> itemYPositions; 

    // Are we in the menu (true) or inside an app (false)?
    bool menuActive = true;

    // We will have one callback function for each button (some might share).
    static void onButtonLeftPressed(const ButtonEvent& event);
    static void onButtonRightPressed(const ButtonEvent& event);
    static void onButtonUpPressed(const ButtonEvent& event);
    static void onButtonDownPressed(const ButtonEvent& event);
    static void onButtonBackPressed(const ButtonEvent& event);
    static void onButtonSelectPressed(const ButtonEvent& event);

    /**
     * @brief Register or unregister all six callbacks. 
     * We do this so that the app itself can take over the same hardware buttons.
     */
    void registerMenuCallbacks();
    void unregisterMenuCallbacks();

    /**
     * @brief Re-draw the menu: items and highlight.
     * This is called from update().
     */
    void drawMenu();

    /**
     * @brief Animate highlight from its current location to the newly selected item row.
     */
    void animateHighlight(int oldIndex);

    /**
     * @brief Helper to go deeper into a category or launch an app if it’s not a category.
     */
    void selectCurrentItem();

    /**
     * @brief Helper to go back up one level in the navigation stack.
     */
    void goBack();

    /**
     * @brief Helper to move the highlight up/down within the current item list.
     */
    void moveHighlightUp();
    void moveHighlightDown();

    /**
     * @brief Helper to adjust scrollOffset if the highlight goes off-screen
     */
    void updateScrollForCurrentIndex(int oldIndex);
};

#endif
