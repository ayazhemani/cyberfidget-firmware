cmake_minimum_required(VERSION 3.13)
project(cyberfidget_wasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Include paths ----
# Shims MUST come first so they override real ESP32/Arduino headers
include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/shims
    ${CMAKE_CURRENT_SOURCE_DIR}/shims/freertos
)

# Firmware library headers (the real DisplayProxy, ButtonManager, etc.)
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
# Core WASM runtime and HAL
set(WASM_SOURCES
    hal/wasm_runtime.cpp
    hal/wasm_fonts.cpp
    hal/audio_wasm.cpp
    hal/HAL_WASM.cpp
    main_wasm.cpp
)

# Firmware libraries that compile cleanly with shims.
# Start minimal, expand as more apps are brought in.
set(FW_SOURCES
    ${LIB_DIR}/DisplayProxy/DisplayProxy.cpp
    ${LIB_DIR}/Globals/Globals.cpp
    ${LIB_DIR}/ButtonManager/ButtonManager.cpp
)

add_executable(cyberfidget ${WASM_SOURCES} ${FW_SOURCES})

# ---- Emscripten-specific settings ----
if(EMSCRIPTEN)
    set_target_properties(cyberfidget PROPERTIES
        SUFFIX ".js"
    )

    target_link_options(cyberfidget PRIVATE
        # Export C functions callable from JS
        "SHELL:-s EXPORTED_FUNCTIONS=['_main','_wasm_button_press','_wasm_button_release','_wasm_set_slider','_wasm_get_framebuffer','_wasm_get_framebuffer_size']"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAPU8']"

        # Memory settings
        "SHELL:-s INITIAL_MEMORY=4194304"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"

        # Use requestAnimationFrame for the main loop
        "SHELL:-s ASYNCIFY"

        # Module output
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s EXPORT_NAME='CyberFidgetModule'"

        # Optimization
        "SHELL:-O2"
    )
endif()
