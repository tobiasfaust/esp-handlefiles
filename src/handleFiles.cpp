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
 * @brief Register a LittleFS Filesystem instance
 * @param fs: Pointer to the LittleFS instance to register
 * @param basePath: Base path for the filesystem (default is "/")
 */
void handleFiles::registerLittleFS(fs::LittleFSFS* fs , String basePath) {
  if (fs == nullptr) {
    log(1, "handleFiles::registerLittleFS: fs pointer is null");
    return;
  }

  // Check if the filesystem is already registered
  for (const auto& instance : littleFSVector) {
    if (instance.fs == fs) {
      log(2, "handleFiles::registerLittleFS: Filesystem already registered");
      return;
    }
  }

  // Add the new filesystem instance to the vector
  LittleFSInstance newInstance = {fs, basePath};
  littleFSVector.push_back(newInstance);
  log(3, "handleFiles::registerLittleFS: Filesystem registered with base path: %s", basePath.c_str());
} 

/*************************************
 * @brief Get the LittleFS pointer for a given path
 * @param path: The path to check
 * @return Pointer to the LittleFS instance or nullptr if not found
 */
fs::LittleFSFS* handleFiles::getFSPtr(const char* path) {
  if (littleFSVector.empty()) {
    return &LittleFS;
  }

  // Find the LittleFS instance that matches the given path
  for (const auto& instance : littleFSVector) {
    if (String(path).startsWith(instance.basePath)) {
      log(4, "handleFiles::getFSPtr: Found matching LittleFS instance for path '%s': %s", path, instance.basePath.c_str());
      // Return the pointer to the matching LittleFS instance
      return instance.fs;
    }
  }
  // If no matching instance is found, return the default LittleFS instance
  log(2, "handleFiles::getFSPtr: No matching LittleFS instance found for path: %s", path);
  return &LittleFS;
}

/*************************************
 * @brief Get the true filename for a given LittleFS instance and filename
 * @param fs: Pointer to the LittleFS instance
 * @param filename: The filename to check
 * @return The true filename without the base path
 */
String handleFiles::getFsFilePath(fs::LittleFSFS* fs, String filename) {
  for (const auto& instance : littleFSVector) {
    if (instance.fs == fs && instance.basePath != "/") {
      if (filename.startsWith(instance.basePath)) {
        return filename.substring(instance.basePath.length());
      }
    }
  }
  return filename;
}

/**************************************
 * @brief Register a callback function for logging
 * @param logCallback: Callback function pointer
 */
void handleFiles::registerLogCallback(std::function<void(int, const char*)> logCallback) {
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
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    logCallback(loglevel, buffer);
    va_end(args);
  }
}

//###############################################################
// returns the complete folder structure
//###############################################################
void handleFiles::getDirList(JsonArray json, LittleFSInstance* myfs, String path) {
  JsonDocument doc;
  JsonObject jsonRoot = doc.to<JsonObject>();

  String p1 = myfs->basePath + path;
  if (p1.startsWith("//")) { p1 = p1.substring(1); }
  if (p1.endsWith("/")) { p1 = p1.substring(0, p1.length() - 1); }
  jsonRoot["path"] = p1;
  JsonArray content = jsonRoot["content"].to<JsonArray>();

  File FSroot = myfs->fs->open(path, "r");
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
        this->getDirList(json, myfs, p); // recursive call
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

  if (subaction == "listDir") {
    JsonArray content = json["JS"]["listdir"].to<JsonArray>();
    
    if (littleFSVector.empty()) {
      // If no LittleFS instances are registered, list the root directory of the default LittleFS
      LittleFSInstance defaultInstance = {&LittleFS, "/"};
      this->getDirList(content, &defaultInstance, "/");
    } else {
      //start with root for all basepath of registered LittleFS instances
      JsonObject fsroot = content.add<JsonObject>();
      fsroot["path"] = "/";
      JsonArray rootcontent = fsroot["content"].to<JsonArray>();

      for (const auto& instance : littleFSVector) {
        JsonObject jsonObj = rootcontent.add<JsonObject>();
        String fname(instance.basePath.substring(1));
        jsonObj["name"] = fname;
        jsonObj["isDir"] = 1; // mark as directory
      }

      for (const auto& instance : littleFSVector) {
        log(5, "handleFiles::HandleRequest: Listing directory for LittleFS instance with base path: %s", instance.basePath.c_str());
        this->getDirList(content, const_cast<LittleFSInstance*>(&instance), "/");
      }
    }

  } else if (subaction == "deleteFile") {
    String filename("");
    this->log(3, "Request to delete file %s", filename.c_str());

    if (json["cmd"]["filename"])  {filename  = json["cmd"]["filename"].as<String>();}
    
    fs::LittleFSFS* fs = this->getFSPtr(filename.c_str());
    filename = this->getFsFilePath(fs, filename);
    
    if (fs->remove(filename)) { 
      json["response"]["status"] = 1;
      json["response"]["text"] = "deletion successful";
    } else {
      json["response"]["status"] = 0;
      json["response"]["text"] = "deletion failed";
    }
    
    this->log(3, json.as<String>().c_str());
  
  
  } else if (subaction == "deleteFolder") {
    String foldername("");
    this->log(3, "Request to delete folder %s", foldername.c_str());

    if (json["cmd"]["foldername"])  {foldername  = json["cmd"]["foldername"].as<String>();}
    
    fs::LittleFSFS* fs = this->getFSPtr(foldername.c_str());
    foldername = this->getFsFilePath(fs, foldername);
    
    if (this->deleteFolderRecursive(fs, foldername)) { 
      json["response"]["status"] = 1;
      json["response"]["text"] = "deletion successful";
    } else {
      json["response"]["status"] = 0;
      json["response"]["text"] = "deletion failed";
    }
    
    this->log(3, json.as<String>().c_str());
  
  } else if (subaction == "addFolder") {
    String foldername("");
    this->log(3, "Request to add folder %s", foldername.c_str());

    if (json["cmd"]["foldername"])  {foldername  = json["cmd"]["foldername"].as<String>();}
    
    fs::LittleFSFS* fs = this->getFSPtr(foldername.c_str());
    foldername = this->getFsFilePath(fs, foldername);
    
    if (fs->exists(foldername)) {
      json["response"]["status"] = 0;
      json["response"]["text"] = "folder already exists";
      this->log(2, "Folder already exists: %s", foldername.c_str());
      return;
    }

    if (fs->mkdir(foldername)) { 
      json["response"]["status"] = 1;
      json["response"]["text"] = "folder created successfully";
      this->log(3, "Folder created successfully: %s", foldername.c_str());
    } else {
      json["response"]["status"] = 0;
      json["response"]["text"] = "folder creation failed";
      this->log(2, "Failed to create folder: %s", foldername.c_str());
    }

  } else {
    json["response"]["status"] = 0;
    json["response"]["text"] = "unknown subaction";
  }


}

/**
 * @brief Recursively deletes all files and folders under the given folder, then deletes the folder itself.
 * @param fs Pointer to the LittleFS instance
 * @param folder The folder path to delete
 * @return true if successful, false otherwise
 */
bool handleFiles::deleteFolderRecursive(fs::LittleFSFS* fs, String folder) {
  if (!fs->exists(folder)) {
    this->log(2, "deleteFolderRecursive: Folder does not exist: %s", folder.c_str());
    return false;
  }

  File dir = fs->open(folder, "r");
  if (!dir || !dir.isDirectory()) {
    this->log(2, "deleteFolderRecursive: Not a directory: %s", folder.c_str());
    return false;
  }

  File file = dir.openNextFile();
  while (file) {
    String path = String(folder) + "/" + String(file.name());
    bool isDir = file.isDirectory();
    file.close();
    this->log(5, "deleteFolderRecursive: Processing file: %s", path.c_str());
    if (isDir) {
      this->log(5, "deleteFolderRecursive: Found directory, do a recurive call: %s", path.c_str());
      // Recursive call for subdirectory
      if (!deleteFolderRecursive(fs, path)) {
        dir.close();
        return false;
      }
    } else {
      // Delete file
      this->log(5, "deleteFolderRecursive: Deleting file: %s", path.c_str());
      if (!fs->remove(path)) {
        this->log(2, "deleteFolderRecursive: Failed to delete file: %s", path.c_str());
        dir.close();
        return false;
      }
    }
    file = dir.openNextFile();
  }
  dir.close();

  // Delete the folder itself
  this->log(5, "deleteFolderRecursive: Deleting folder: %s", folder.c_str());
  if (!fs->rmdir(folder)) {
    this->log(2, "deleteFolderRecursive: Failed to delete folder: %s", folder.c_str());
    return false;
  }
  this->log(3, "deleteFolderRecursive: Successfully deleted folder: %s", folder.c_str());
  return true;
}

//###############################################################
// store a file at Filesystem
//###############################################################
void handleFiles::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  
  this->log(5, "Client: %s %s", request->client()->remoteIP().toString().c_str(), request->url().c_str());
  fs::LittleFSFS* fs = this->getFSPtr(filename.c_str());
  filename = this->getFsFilePath(fs, filename);

  if (!index) {
    // open the file on first call and store the file handle in the request object
    String path = "";
    for (unsigned int i = 0; i < filename.length(); i++) {
      if (filename[i] == '/') {
        if (!fs->exists(path)) { 
          fs->mkdir(path);
        }
      }
      path += filename[i];
    }

    request->_tempFile = fs->open(filename, "w");
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