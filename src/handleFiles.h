#ifndef HANDLEFILES_H
#define HANDLEFILES_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <WiFi.h> 
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class handleFiles {
    public:
        handleFiles(AsyncWebServer *server);

        void        HandleRequest(JsonDocument& json);
        void        handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        void        registerLogCallback(std::function<void(int, const char*, va_list)> logCallback);

    private:
        void        getDirList(JsonArray json, String path);
        void        log(int loglevel, const char* format, ...);
        
        std::function<void(int, const char*, va_list)> logCallback; // Callback function pointer
};

#endif
