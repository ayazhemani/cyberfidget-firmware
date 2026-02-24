cmake_minimum_required(VERSION 3.13)
project(cyberfidget_wasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- App selection ----
# Pass -DWASM_APP=DinoGame (or Flashlight, Demo) to select which app to build.
# Default: Demo (built-in Hello OLED test screen)
if(NOT DEFINED WASM_APP)
    set(WASM_APP "Demo")
endif()

message(STATUS "Building WASM app: ${WASM_APP}")

# ---- Include paths ----
# Shims MUST come first so they override real ESP32/Arduino headers
include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/shims
    ${CMAKE_CURRENT_SOURCE_DIR}/shims/freertos
)

# Firmware library headers
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
include_directories(
    ${LIB_DIR}/HAL
    ${LIB_DIR}/DisplayProxy
    ${LIB_DIR}/ButtonManager
    ${LIB_DIR}/AudioManager
    ${LIB_DIR}/RGBController
    ${LIB_DIR}/SliderPosition
    ${LIB_DIR}/BatteryManager
    ${LIB_DIR}/AppManager
    ${LIB_DIR}/AppDefs
    ${LIB_DIR}/MenuManager
    ${LIB_DIR}/Globals
    ${LIB_DIR}/Fonts
    ${LIB_DIR}/PowerManager
    ${LIB_DIR}/WiFiManagerCF
    ${LIB_DIR}/ClockDisplay
    ${LIB_DIR}/Flashlight
    ${LIB_DIR}/Booper
    ${LIB_DIR}/BootAnimation
    ${LIB_DIR}/BreakoutGame
    ${LIB_DIR}/DinoGame
    ${LIB_DIR}/SimonSaysGame
    ${LIB_DIR}/SPHFluidGame
    ${LIB_DIR}/StratagemGame
    ${LIB_DIR}/ExampleScreens
    ${LIB_DIR}/ReactionTimeGame
    ${LIB_DIR}/SerialDisplay
    ${LIB_DIR}/MatrixScreensaver
    ${LIB_DIR}/MatrixScreensaverDither
    ${LIB_DIR}/GraveyardScreensaver
    ${LIB_DIR}/Ghosts
    ${LIB_DIR}/Eye
    ${LIB_DIR}/Images
    ${LIB_DIR}/CyberFidgie
    ${LIB_DIR}/Splooty
)

# ---- Source files ----
# Core WASM runtime and HAL (always included)
set(WASM_SOURCES
    hal/wasm_runtime.cpp
    hal/wasm_fonts.cpp
    hal/audio_wasm.cpp
    hal/HAL_WASM.cpp
    main_wasm.cpp
)

# Firmware libraries needed by all apps
set(FW_SOURCES
    ${LIB_DIR}/DisplayProxy/DisplayProxy.cpp
    ${LIB_DIR}/Globals/globals.cpp
    ${LIB_DIR}/ButtonManager/ButtonManager.cpp
    ${LIB_DIR}/RGBController/RGBController.cpp
)

# ---- Per-app sources and defines ----
if(WASM_APP STREQUAL "DinoGame")
    add_compile_definitions(WASM_APP_DINOGAME)
    list(APPEND FW_SOURCES
        ${LIB_DIR}/DinoGame/DinoGame.cpp
    )
elseif(WASM_APP STREQUAL "Flashlight")
    add_compile_definitions(WASM_APP_FLASHLIGHT)
    list(APPEND FW_SOURCES
        ${LIB_DIR}/Flashlight/Flashlight.cpp
    )
elseif(WASM_APP STREQUAL "Breakout")
    add_compile_definitions(WASM_APP_BREAKOUT)
    list(APPEND FW_SOURCES
        ${LIB_DIR}/BreakoutGame/BreakoutGame.cpp
    )
elseif(WASM_APP STREQUAL "MatrixScreensaver")
    add_compile_definitions(WASM_APP_MATRIXSCREENSAVER)
    list(APPEND FW_SOURCES
        ${LIB_DIR}/MatrixScreensaver/MatrixScreensaver.cpp
    )
elseif(WASM_APP STREQUAL "Custom")
    if(NOT DEFINED CUSTOM_APP_NAME)
        message(FATAL_ERROR "CUSTOM_APP_NAME is required for Custom builds (e.g. -DCUSTOM_APP_NAME=MyApp)")
    endif()
    if(NOT DEFINED CUSTOM_APP_INSTANCE)
        # Convention: lowercase first letter of class name
        string(SUBSTRING ${CUSTOM_APP_NAME} 0 1 _FIRST)
        string(TOLOWER ${_FIRST} _FIRST_LOWER)
        string(SUBSTRING ${CUSTOM_APP_NAME} 1 -1 _REST)
        set(CUSTOM_APP_INSTANCE "${_FIRST_LOWER}${_REST}")
    endif()

    message(STATUS "Custom app: class=${CUSTOM_APP_NAME} instance=${CUSTOM_APP_INSTANCE}")
    add_compile_definitions(WASM_APP_CUSTOM)
    add_compile_definitions(WASM_CUSTOM_INSTANCE=${CUSTOM_APP_INSTANCE})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app)
    list(APPEND FW_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/app/${CUSTOM_APP_NAME}.cpp)
else()
    message(STATUS "Building default demo (no app selected)")
endif()

add_executable(cyberfidget ${WASM_SOURCES} ${FW_SOURCES})

# ---- Emscripten-specific settings ----
if(EMSCRIPTEN)
    set_target_properties(cyberfidget PROPERTIES
        SUFFIX ".js"
    )

    target_link_options(cyberfidget PRIVATE
        "SHELL:-s EXPORTED_FUNCTIONS=['_main','_wasm_button_press','_wasm_button_release','_wasm_set_slider','_wasm_set_accel','_wasm_get_framebuffer','_wasm_get_framebuffer_size','_wasm_stop']"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

        "SHELL:-s INITIAL_MEMORY=4194304"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"

        "SHELL:-s ASYNCIFY"

        "SHELL:-s MODULARIZE=1"
        "SHELL:-s EXPORT_NAME='CyberFidgetModule'"

        "SHELL:-O2"
    )
endif()
