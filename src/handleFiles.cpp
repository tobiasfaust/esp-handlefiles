#include "handleFiles.h"

handleFiles::handleFiles(AsyncWebServer *server) {

  server->on("/doUpload", HTTP_POST, [](AsyncWebServerRequest *request) {},
                                    std::bind(&handleFiles::handleUpload, this, std::placeholders::_1, 
                                        std::placeholders::_2,
                                        std::placeholders::_3,
                                        std::placeholders::_4,
                                        std::placeholders::_5,
                                        std::placeholders::_6));

}  

/**************************************
 * @brief Register a callback function for logging
 * @param logCallback: Callback function pointer
 */
void handleFiles::registerLogCallback(std::function<void(int, const char*, va_list)> logCallback) {
  this->logCallback = logCallback;
}

/***************************************
 * @brief Wrapper function for logging to pass the log messages to the callback function
 * @param loglevel: Log level
 * @param format: Format string
 * @param ...: Variable arguments
 */
void handleFiles::log(int loglevel, const char* format, ...) {
  if (logCallback) {
    va_list args;
    va_start(args, format);
    logCallback(loglevel, format, args);
    va_end(args);
  }
}

//###############################################################
// returns the complete folder structure
//###############################################################
void handleFiles::getDirList(JsonArray json, String path) {
  JsonDocument doc;
  JsonObject jsonRoot = doc.to<JsonObject>();

  jsonRoot["path"] = path;
  JsonArray content = jsonRoot["content"].to<JsonArray>();

  File FSroot = LittleFS.open(path);
  File file = FSroot.openNextFile();

  while (file) {
    JsonDocument doc1;
    JsonObject jsonObj = doc1.to<JsonObject>();
    String fname(file.name());
    jsonObj["name"] = fname;

    if(file.isDirectory()){    
        jsonObj["isDir"] = 1;
        String p = path + "/" + fname;
        if (p.startsWith("//")) { p = p.substring(1); }
        this->getDirList(json, p); // recursive call
    } else {
      jsonObj["isDir"] = 0;
    }

    content.add(jsonObj);
    file.close();
    file = FSroot.openNextFile();
  }
  FSroot.close();
  json.add(jsonRoot);
}

//###############################################################
// returns the requested data from Webserver.cpp
//###############################################################
void handleFiles::HandleRequest(JsonDocument& json) {
  String subaction = "";
  if (json["cmd"]["subaction"])  {subaction  = json["cmd"]["subaction"].as<String>();}

  this->log(3, "handle Request in handleFiles.cpp: %s", subaction.c_str());

  if (subaction == "listDir") {
    JsonArray content = json["JS"]["listdir"].to<JsonArray>();
    
    this->getDirList(content, "/");
    this->log(5, json["content"].as<String>().c_str());

  } else if (subaction == "deleteFile") {
    String filename("");

    this->log(3, "Request to delete file %s", filename.c_str());

    if (json["cmd"]["filename"])  {filename  = json["cmd"]["filename"].as<String>();}
    
    if (LittleFS.remove(filename)) { 
      json["response"]["status"] = 1;
      json["response"]["text"] = "deletion successful";
    } else {
      json["response"]["status"] = 0;
      json["response"]["text"] = "deletion failed";
    }
    
    this->log(3, json.as<String>().c_str());
  }
}

//###############################################################
// store a file at Filesystem
//###############################################################
void handleFiles::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  
  this->log(5, "Client: %s %s", request->client()->remoteIP().toString().c_str(), request->url().c_str());;

  if (!index) {
    // open the file on first call and store the file handle in the request object
    request->_tempFile = LittleFS.open(filename, "w");
    this->log(5, "Upload Start: %s", filename.c_str());
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    this->log(5, "Writing file: %s ,index=%d len=%d bytes, FreeMem: %d", filename.c_str(), index, len, ESP.getFreeHeap());
  }

  if (final) {
    // close the file handle as the upload is now done
    request->_tempFile.close();
    this->log(3, "Upload Complete: %s ,size: %d Bytes", filename.c_str(), (index + len));

    AsyncResponseStream *response = request->beginResponseStream("text/json");
    response->addHeader("Server","ESP Async Web Server");

    JsonDocument jsonReturn;
    String ret;

    jsonReturn["status"] = 1;
    jsonReturn["text"] = "OK";

    serializeJson(jsonReturn, ret);
    response->print(ret);
    request->send(response);

    if (logCallback) {
      String jsonString;
      serializeJson(jsonReturn, jsonString);
      this->log(5, jsonString.c_str());
    }
  }
}