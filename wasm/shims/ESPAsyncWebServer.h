#ifndef WASM_ESPASYNCWEBSERVER_H
#define WASM_ESPASYNCWEBSERVER_H

#include <cstdint>

class AsyncWebServerRequest;
class AsyncWebServer {
public:
    AsyncWebServer(uint16_t = 80) {}
    void begin() {}
    void end() {}
    void on(const char*, int, void(*)(AsyncWebServerRequest*)) {}
};

#endif
