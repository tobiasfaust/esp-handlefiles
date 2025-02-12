#ifndef HANDLEFILES_H
#define HANDLEFILES_H

#include <WiFi.h> 
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class handleFiles {
    public:
        handleFiles(AsyncWebServer *server);

        void        HandleAjaxRequest(JsonDocument& jsonGet, AsyncResponseStream* response);
        void        handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        void        registerLogCallback(void (*logCallback)(const int loglevel, const char* format, ...));

    private:
        void        getDirList(JsonArray* json, String path);
        void (*logCallback)(const int loglevel, const char* format, ...) = nullptr; // Callback function pointer
};

#endif
