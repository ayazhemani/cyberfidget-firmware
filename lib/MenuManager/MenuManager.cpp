#include "MenuManager.h"
#include "HAL.h"         // So we can reach buttonManager, display, etc.
#include "AppManager.h"   // So we can set appActive, appPreviously, etc.
#include "globals.h"      // If you have global for 'millis_NOW', etc.
#include "AppDefs.h"      // For AppIndex enum

#include <sstream>        // For path parsing
#include <algorithm>  // for std::min/max if needed
#include <string>

// Aliases to hardware
static auto &display       = HAL::displayProxy();
static auto &buttonManager = HAL::buttonManager();

// Screen dimension (SSD1306 128×64 example)
static const int SCREEN_WIDTH  = 128; 
static const int SCREEN_HEIGHT = 64;

// Row spacing for text in the menu:
static const int MENU_ITEM_HEIGHT = 16;
static const int MENU_ITEM_Y_OFFSET = 2;

// We’ll offset the highlight bar a bit from the left edge:
static const int HIGHLIGHT_X_OFFSET = 4;
static const int HIGHLIGHT_Y_OFFSET = 4;
static const int HIGHLIGHT_WIDTH    = SCREEN_WIDTH - (HIGHLIGHT_X_OFFSET * 2); // just wide enough to look nice

// SCROLLING FIELDS
static int scrollOffset = 0;   // how many pixels the entire list is shifted up
static int oldScrollOffset = 0; // used if we want to animate from old→new
static const int VISIBLE_COUNT = 4; // how many items can fit on screen at once

// Helper to parse path like "Tools/WiFi" => ["Tools","WiFi"]
static std::vector<std::string> parseCategoryPath(const std::string &path)
{
    std::vector<std::string> result;
    if (path.empty()) {
        return result; // no subcategories
    }
    std::stringstream ss(path);
    std::string segment;
    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) {
            result.push_back(segment);
        }
    }
    return result;
}

/**
 * @brief Locates or creates a subcategory with name catName 
 * in currentLevel, returning a reference to that subcategory's children vector.
 */
std::vector<MenuItem>& MenuManager::findOrCreateCategory(std::vector<MenuItem> &currentLevel,
                                                         const std::string &catName)
{
    // see if catName already exists
    for (auto &mi : currentLevel) {
        if (mi.isCategory && mi.label == catName) {
            return mi.children; 
        }
    }
    // not found => create new category
    // must pass 3 args to constructor
    MenuItem newCat(catName, true, APP_COUNT); 
    // or APP_MENU or a "dummy" index for categories
    currentLevel.push_back(newCat);

    return currentLevel.back().children;
}

// This method is called from buildNestedMenu() to add a single app’s entry
static void addAppToMenu(const char* label, const char* path, int appIndex)
{
    MenuManager::instance().registerApp(path, label, (AppIndex)appIndex);
}


//-------------------------------------------------------------------
// Singleton
MenuManager &MenuManager::instance()
{
    static MenuManager single;
    return single;
}

//-------------------------------------------------------------------
// Singleton constructor: build root items, set up highlight element, etc.
MenuManager::MenuManager()
{
    // Our "current" location is the root and index=0
    scrollOffset = 0; // top
    currentItemList = &rootMenuItems;
    currentIndex    = 0;

    // Prepare highlight UIElement for row #0
    highlightElement.setX(HIGHLIGHT_X_OFFSET);
    highlightElement.setY(0); // We'll set correct row in begin()
    highlightElement.setWidth(HIGHLIGHT_WIDTH);
    highlightElement.setHeight(MENU_ITEM_HEIGHT);

    // Clear itemYPositions (we fill them in drawMenu())
    itemYPositions.clear();
    
}

void MenuManager::begin()
{
    // Register the menu callbacks:
    registerMenuCallbacks();

    // Clear any existing root items if you want a fresh build
    rootMenuItems.clear();
    navigationStack.clear();
    currentIndex = 0;
    scrollOffset=0;
    highlightElement.setY(0);

    // parse each line's path, build the categories
    buildNestedMenu(); 
    // buildNestedMenu calls something like:
    // for i in [0..APP_COUNT-1]:
    //    addAppToMenu(appDefs[i].name, appDefs[i].categoryPath, i);

    menuActive = true;
}

void MenuManager::end()
{
    menuActive = false;
    unregisterMenuCallbacks();
}


// Called by the main loop to let the menu update animations, etc.
void MenuManager::update()
{
    if (!menuActive) return; // If an app is running, do nothing

    // Update (animate) highlight, etc. 
    animateAll();    // from your Animation.cpp
    updateTmp();     // if needed from your code

    // Redraw the menu each frame (or only if something changed).
    drawMenu();
}

// Called by an app to hand control back to the menu
void MenuManager::returnToMenu()
{
    AppManager::instance().switchToApp(APP_MENU);
}

// This is the function that actually registers an app in the data structure
// by parsing the path. e.g. "Tools/WiFi" => sub-cat "Tools", sub-cat "WiFi", then leaf.
void MenuManager::registerApp(const std::string &path,
                                const std::string &label,
                                AppIndex index)
{
    // parse
    auto categories = parseCategoryPath(path);
    // start at root
    std::vector<MenuItem> *level = &rootMenuItems;

    for (auto &catName : categories) {
        level = &findOrCreateCategory(*level, catName);
    }

    // now add a leaf item
    // we must pass 3 arguments: (label, isCategory=false, index)
    MenuItem leaf(label, false, index);
    level->push_back(leaf);
}   

// Private method: move highlight up
void MenuManager::moveHighlightUp()
{
    if (currentIndex > 0)
    {
        int oldIndex = currentIndex;
        currentIndex--;
        updateScrollForCurrentIndex(oldIndex);
    }
}

// Private method: move highlight down
void MenuManager::moveHighlightDown()
{
    if (currentIndex < (int)currentItemList->size() - 1)
    {
        int oldIndex = currentIndex;
        currentIndex++;
        updateScrollForCurrentIndex(oldIndex);
    }
}

/**
 * @brief This function checks whether the highlight has moved beyond
 *        the top or bottom of the visible region, and if so, it animates
 *        the entire list via scrollOffset. Otherwise, it just animates
 *        the highlight if the item is still on screen.
 */
void MenuManager::updateScrollForCurrentIndex(int oldIndex)
{
    // We'll define how many items fit “fully” on screen:
    // e.g. VISIBLE_COUNT=3 for 3 items (each 20 px tall => 60 px total).
    // The highlight’s “row index” on screen is currentIndex - scrollOffsetInItems
    // but we’re doing pixel-based “scrollOffset”. Let’s compute the item’s Y:
    int itemY = currentIndex * MENU_ITEM_HEIGHT - scrollOffset;
    int topY  = 0;
    int bottomY = SCREEN_HEIGHT - MENU_ITEM_HEIGHT;

    if (itemY < topY) {
        // We need to scroll upward so that the highlight is at or near top
        // e.g. new scrollOffset = currentIndex * MENU_ITEM_HEIGHT
        int newScroll = currentIndex * MENU_ITEM_HEIGHT;
        // We'll animate scrollOffset from oldScrollOffset to newScroll
        oldScrollOffset = scrollOffset;
        insertAnimation(
            // animate the int pointer
            new Animation(
                &scrollOffset,
                LINEAR,
                newScroll,   // endVal
                200          // ms
            )
        );
    }
    else if (itemY > bottomY) {
        // We need to scroll downward so highlight is near the bottom row
        // e.g. new scrollOffset = currentIndex * MENU_ITEM_HEIGHT - bottomY
        int newScroll = currentIndex * MENU_ITEM_HEIGHT - bottomY;
        oldScrollOffset = scrollOffset;
        insertAnimation(
            new Animation(
                &scrollOffset,
                LINEAR,
                newScroll,
                200
            )
        );
    }
    else {
        // The item is fully within the visible area. 
        // Animate the highlight’s Y only.
        animateHighlight(oldIndex);
    }
}

void MenuManager::animateHighlight(int oldIndex)
{
    // If we’re not shifting the entire list, we move highlight from old Y→new Y.
    int oldY = (oldIndex * MENU_ITEM_HEIGHT) - scrollOffset;
    int newY = (currentIndex * MENU_ITEM_HEIGHT) - scrollOffset;

    // 1) Force the highlight element’s “starting” position to oldY,
    //    so the animation engine sees that as the current geometry
    //    (i.e. the highlight is at oldY *right now*).
    highlightElement.setY(oldY);

    // 2) Decide what the *final* geometry is going to be:
    //    - If you only want to move vertically, you can keep the same X, width, height
    //    - newY is from itemYPositions for the *destination row*
    int newX      = highlightElement.getX();      // i.e. same X
    int newWidth  = highlightElement.getWidth();  // same width
    int newHeight = highlightElement.getHeight(); // same height

    // 3) Insert an animation that moves from the highlight’s *current*
    //    geometry (X, oldY, width, height) to the *new* geometry in 300ms:
    insertAnimation(
        new Animation(
            &highlightElement,
            LINEAR,     // or LINEAR, PARALLELOGRAM, etc.
            newX,       // endX
            newY,       // endY
            newWidth,   // endWidth
            newHeight,  // endHeight
            200         // totalTime in ms
        )
    );
}

// Called on "Select" button
void MenuManager::selectCurrentItem()
{
    if (currentItemList->empty()) return;
    const MenuItem &mi = (*currentItemList)[currentIndex];

    if (mi.isCategory)
    {
        // 1) We're drilling into a new sub-menu
        // push current location on stack
        MenuNavState s{ currentItemList, currentIndex };
        navigationStack.push_back(s);

        // 2) Switch to the sub-items 
        currentItemList = const_cast<std::vector<MenuItem>*>(&mi.children);
        currentIndex    = 0; // start highlighting the first item in that sub-menu

        // Move highlight instantly (or animate from old row to row 0 if you prefer)
        highlightElement.setY(0);
    }
    else
    {
        // leaf => an app
        menuActive = false;
        unregisterMenuCallbacks();

        AppManager::instance().switchToApp(mi.appIndex);
    }
}

// Called on "Back" button
void MenuManager::goBack()
{
    if (navigationStack.empty()) {
        // Already at root. You can choose what "Back" does at root:
        // e.g. do nothing, or possibly go to some default state, or 
        // put the device to sleep. For now we do nothing.
        return;
    }
    MenuNavState s = navigationStack.back();
    navigationStack.pop_back();

    currentItemList = s.itemList;
    currentIndex    = s.index;

    // Make sure highlight is physically placed on the correct item
    int newY = (currentIndex * MENU_ITEM_HEIGHT) - scrollOffset;
    highlightElement.setY(newY);
    
    // If you want to re-check whether scrollOffset is correct for that item:
    // e.g. updateScrollForCurrentIndex(...) passing an oldIndex just to ensure
    // the item is in view. But often you'll just snap to no offset or the last offset you had.

    // Possibly keep the same scrollOffset or reset it:
    // scrollOffset = 0; // if you want to always revert to top
}

/**
 * @brief Redraw all items, factoring in scrollOffset for each.
 *        Also draw highlight if we want a separate highlight rect.
 */
void MenuManager::drawMenu()
{
    // Start by clearing the screen or filling background:
    display.clear();

    for (int i=0; i < (int)currentItemList->size(); i++)
    {
        // The baseline Y for item i:
        int yPos = i * MENU_ITEM_HEIGHT - scrollOffset;
        
        // If it’s in screen range, we can draw it
        if (yPos + MENU_ITEM_HEIGHT >= 0 && yPos < SCREEN_HEIGHT) {
            // e.g. draw text
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.setFont(ArialMT_Plain_10);
            display.drawString(10, yPos + MENU_ITEM_Y_OFFSET, (*currentItemList)[i].label.c_str());
        }
    }

    // Optionally draw a highlight rectangle behind the current item 
    // (but we’re animating highlightElement, so let's just fill it):
    //   highlightElement.getX(), highlightElement.getY(), 
    //   highlightElement.getWidth(), highlightElement.getHeight()
    display.drawRect(
        highlightElement.getX(), 
        highlightElement.getY(),
        highlightElement.getWidth(),
        highlightElement.getHeight()
    );

    // Now commit any buffered drawing:
    display.display();
}

// Register our button callbacks:
void MenuManager::registerMenuCallbacks()
{
    // For each hardware button, call the static method we set up:
    buttonManager.registerCallback(button_TopLeftIndex,    onButtonLeftPressed);
    buttonManager.registerCallback(button_TopRightIndex,   onButtonRightPressed);
    buttonManager.registerCallback(button_MiddleLeftIndex, onButtonUpPressed);
    buttonManager.registerCallback(button_MiddleRightIndex,onButtonDownPressed);
    buttonManager.registerCallback(button_BottomLeftIndex, onButtonBackPressed);
    buttonManager.registerCallback(button_BottomRightIndex,onButtonSelectPressed);
}

// Unregister them:
void MenuManager::unregisterMenuCallbacks()
{
    buttonManager.unregisterCallback(button_TopLeftIndex);
    buttonManager.unregisterCallback(button_TopRightIndex);
    buttonManager.unregisterCallback(button_MiddleLeftIndex);
    buttonManager.unregisterCallback(button_MiddleRightIndex);
    buttonManager.unregisterCallback(button_BottomLeftIndex);
    buttonManager.unregisterCallback(button_BottomRightIndex);
}

// Static callback handlers:
void MenuManager::onButtonLeftPressed(const ButtonEvent& event)
{
    // Example: do nothing or implement "move to previous root category"? 
    // Since it's a nested menu, left/right might not do anything if you only have
    // vertical list navigation. Feel free to define your own logic.
}
void MenuManager::onButtonRightPressed(const ButtonEvent& event)
{
    // Press
    if (event.eventType == ButtonEvent_Pressed){
        // Same as above
    }    
}
void MenuManager::onButtonUpPressed(const ButtonEvent& event)
{
    // Press
    if (event.eventType == ButtonEvent_Pressed){
        instance().moveHighlightUp();
    }
}
void MenuManager::onButtonDownPressed(const ButtonEvent& event)
{
    // Press
    if (event.eventType == ButtonEvent_Pressed){
        instance().moveHighlightDown();
    }
}
void MenuManager::onButtonBackPressed(const ButtonEvent& event)
{    // Press
    if (event.eventType == ButtonEvent_Pressed){
        instance().goBack();
    } 
}
void MenuManager::onButtonSelectPressed(const ButtonEvent& event)
{    
    // Press
    if (event.eventType == ButtonEvent_Released){  //Released intentionally to avoid carrying into event
        instance().selectCurrentItem();
    }
}
