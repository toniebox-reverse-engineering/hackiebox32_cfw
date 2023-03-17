#include "WrapperWebServer.h"
#include "Hackiebox.h"


void WrapperWebServer::begin() {  

  _server = new WebServer(80);
  _server->enableCORS(true); //DEV ONLY!

  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  _server->onNotFound([&](){ WrapperWebServer::handleUnknown(); });
  _server->on("/", HTTP_GET, [&](){ WrapperWebServer::handleRoot(); });
  _server->on("/local-echo.js", HTTP_GET, [&](){ WrapperWebServer::handleFile("/revvox/web/local-echo.js", "text/javascript"); });
  _server->on("/api/sse/sub", HTTP_GET, [&](){ WrapperWebServer::handleSseSub(); });
  _server->on("/api/ajax", HTTP_GET, [&](){ WrapperWebServer::handleAjax(); });
  _server->on("/api/upload/file", HTTP_POST, [&](){ }, [&](){ WrapperWebServer::handleUploadFile(); });
  _server->on("/api/upload/flash-file", HTTP_POST, [&](){ }, [&](){ WrapperWebServer::handleUploadFlashFile(); });
  _server->begin();

  setInterval(1);
}

void WrapperWebServer::loop() {  
  _server->handleClient();
  
  _sseTimer.tick();
  if (!_sseTimer.isRunning()) {
    sseKeepAlive();
    _sseTimer.setTimer(30*1000);
  }
}

void WrapperWebServer::handleUnknown(void) {
  /*SSE START*/
  String s_uri = _server->uri();
  const char *uri = s_uri.c_str();
  const char *sseEvents = "/api/sse/";
  if (strncmp(uri, sseEvents, strlen(sseEvents)) != 0) {
    Log.verbose("strncmp: %i on uri=%s", strncmp(uri, sseEvents, strlen(sseEvents)), uri);
    return handleNotFound();
  }

  uri += strlen(sseEvents);
  unsigned int channel = atoi(uri);
  if (channel < SSE_MAX_CHANNELS) {
    return sseHandler(channel);
  } else {
    _server->send(200, "text/plain", "MAX_CLIENTS");
  }
  /*SSE END*/

  //handleNotFound();
}

void WrapperWebServer::handleNotFound(void) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += _server->uri();
  message += "\nMethod: ";
  message += (_server->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += _server->args();
  message += "\n";
  for (uint8_t i=0; i<_server->args(); i++){
    message += " " + _server->argName(i) + ": " + _server->arg(i) + "\n";
  }
  _server->send(404, "text/plain", message);
}

void WrapperWebServer::handleRoot(void) {
  Box.boxPower.feedSleepTimer();
  //_server->send(200, "text/html", "ROOT");
  String hackiebox = String("/revvox/web/hackiebox.html");
  commandGetFile(&hackiebox, 0, 0, false);
}
void WrapperWebServer::handleFile(const char* path, const char* type) {
  String spath = String(path);
  commandGetFile(&spath, 0, 0, false);
}
void WrapperWebServer::handleSseSub(void) {
  Box.boxPower.feedSleepTimer();

  if (subscriptionCount >= SSE_MAX_CHANNELS - 1) 
    return handleNotFound();  // We ran out of channels
  

  uint8_t channel;
  IPAddress clientIP = _server->client().remoteIP();   // get IP address of client
  String SSEurl = "";
  //String SSEurl = "http://";
  //SSEurl += WiFi.localIP().toString();
  //SSEurl += ":";
  //SSEurl += port;
  //size_t offset = SSEurl.length();
  SSEurl += "/api/sse/";

  ++subscriptionCount;
  for (channel = 0; channel < SSE_MAX_CHANNELS; channel++) // Find first free slot
    if (!subscription[channel].clientIP) {
      break;
    }
  Log.verbose("Allocated channel %i, on uri %s", channel, SSEurl.c_str());
  //server.on(SSEurl.substring(offset), std::bind(SSEHandler, &(subscription[channel])));
  Log.verbose("subscription for client IP %s: event bus location: %s", clientIP.toString().c_str(), SSEurl.c_str());
  subscription[channel] = {clientIP, _server->client()};
  SSEurl += channel;
  _server->send(200, "text/plain", SSEurl.c_str());
}
void WrapperWebServer::handleAjax(void) {
  Box.boxPower.feedSleepTimer();
  String cmd = _server->arg("cmd");
  
  sampleMemory(0);

  if (cmd) {
    if (cmd.equals("get-config")) {
      _server->send(200, "text/json", Config.getAsJson());
      return;
    } else if (cmd.equals("get-dir")) {
      String path = _server->arg("dir");
      if (!path)
        path = String();
      _server->setContentLength(CONTENT_LENGTH_UNKNOWN);
      _server->send(200, "text/json", "");
      Box.boxSD.webJsonListDir(_server, (char*)path.c_str());
      _server->sendContent("");
      _server->client().stop();

      return;
    } else if (cmd.equals("get-file")) {
      String filename = _server->arg("filepath");
      long read_start = _server->arg("start").toInt();
      long read_length = _server->arg("length").toInt();
      if (commandGetFile(&filename, read_start, read_length, true))
        return;
    } else if (cmd.equals("get-flash-file")) {
      String filename =_server->arg("filepath");
      long read_start = _server->arg("start").toInt();
      long read_length = _server->arg("length").toInt();
      if (commandGetFlashFile(&filename, read_start, read_length))
        return;
    } else if (cmd.equals("copy-file")) {
      /*
      String source_str = _server->arg("source");
      String target_str = _server->arg("target");
      char* source = (char*)source_str.c_str();
      char* target = (char*)target_str.c_str();
      //TODO
      */
    } else if (cmd.equals("move-file")) {
      File checkFile;
      String source_str = _server->arg("source");
      String target_str = _server->arg("target");
      char* source = (char*)source_str.c_str();
      char* target = (char*)target_str.c_str();
      
      if (SD_MMC.exists(source) && !SD_MMC.exists(target)) {
        checkFile = SD_MMC.open(source);
        if (checkFile && !checkFile.isDirectory() && SD_MMC.rename(source, target)) {
          sendJsonSuccess();
          checkFile.close(); //TODO
          return;
        }
      }
    } else if (cmd.equals("delete-file")) {
      File checkFile;
      String filepath_str = _server->arg("filepath");
      char* filepath = (char*)filepath_str.c_str();

      Log.info("Deleting %s", filepath);
      checkFile = SD_MMC.open(filepath);
      if (checkFile && !checkFile.isDirectory() && SD_MMC.exists(filepath)) {
        checkFile.close(); //TODO
        if (SD_MMC.remove(filepath)) {
          sendJsonSuccess();
          return;
        }
      }
    } else if (cmd.equals("delete-flash-file")) {
      /* TODO
      String filepath_str = _server->arg("filepath");
      char* filepath = (char*)filepath_str.c_str();

      Log.info("Deleting Flash %s", filepath);
      if (SerFlash.del(filepath) == SL_FS_OK) {
          sendJsonSuccess();
          return;
      }
      Log.error("Couldn't del %s %s", filepath, SerFlash.lastErrorString());
      */
    } else if (cmd.equals("create-dir")) {
      String dir_str = _server->arg("dir");
      char* dir = (char*)dir_str.c_str();
      if (!SD_MMC.exists(dir)) {
        if (SD_MMC.mkdir(dir)) {
          sendJsonSuccess();
          return;
        }
      }

    } else if (cmd.equals("move-dir")) {
      File checkFile;
      String source_str = _server->arg("source");
      String target_str = _server->arg("target");
      char* source = (char*)source_str.c_str();
      char* target = (char*)target_str.c_str();
      checkFile = SD_MMC.open(source);
      if (checkFile && checkFile.isDirectory() && !SD_MMC.exists(target)) {
        if (SD_MMC.rename(source, target)) {
          sendJsonSuccess();
          checkFile.close(); //TODO
          return;
        }
      }
    } else if (cmd.equals("copy-dir")) {
      /*
      String source_str = _server->arg("source");
      String target_str = _server->arg("target");
      char* source = (char*)source_str.c_str();
      char* target = (char*)target_str.c_str();
      //TODO
      */
    } else if (cmd.equals("delete-dir")) {
      File checkFile;
      String dir_str = _server->arg("dir");
      char* dir = (char*)dir_str.c_str();
      checkFile = SD_MMC.open(dir);
      if (checkFile && checkFile.isDirectory() && SD_MMC.exists(dir)) {
        if (SD_MMC.rmdir(dir)) {
          sendJsonSuccess();
          return;
        }
      }
    } else if (cmd.equals("box-power")) { 
      String sub = _server->arg("sub");
      if (sub.equals("reset")) {
        sendJsonSuccess();
        Box.boxPower.reset();
      } else if (sub.equals("hibernate")) {
        sendJsonSuccess();
        Box.boxPower.hibernate();
      }
    } else if (cmd.equals("box-battery")) {
      String sub = _server->arg("sub");
      if (sub.equals("start-test")) {
        Box.boxBattery.startBatteryTest();
        sendJsonSuccess();
        return;
      } else if (sub.equals("test-active")) {
        //TODO
        return;
      } else if (sub.equals("stats")) {
        StaticJsonDocument<194> jsonStats; //Size from https://arduinojson.org/v6/assistant/
        BoxBattery::BatteryStats stats = Box.boxBattery.getBatteryStats();

        jsonStats["charging"] = stats.charging;
        jsonStats["low"] = stats.low;
        jsonStats["critical"] = stats.critical;
        jsonStats["adcRaw"] = stats.adcRaw;
        jsonStats["voltage"] = stats.voltage;
        jsonStats["testActive"] = stats.testActive;
        jsonStats["testActiveMinutes"] = stats.testActiveMinutes;

        size_t len = measureJson(jsonStats)+1;
        char json[len];
        serializeJson(jsonStats, json, len);
        sampleMemory(5);
        _server->send(200, "text/json", json);
        return;
      }
    } else if (cmd.equals("box-rfid")) {
      String sub = _server->arg("sub");
      if (sub.equals("uid")) {
        StaticJsonDocument<218> rfidStats; //Size from https://arduinojson.org/v6/assistant/
        uint8_t uid[24];
        Box.boxRFID.getUID(uid);

        rfidStats["active"] = Box.boxRFID.tagActive;
        rfidStats["uid"] = uid;
        JsonArray uidRaw = rfidStats.createNestedArray("uidRaw");
        for (uint8_t i = 0; i < 8; i++) {
          uidRaw.add(Box.boxRFID.tagUid[i]);
        }

        size_t len = measureJson(rfidStats)+1;
        char json[len];
        serializeJson(rfidStats, json, len);
        sampleMemory(6);
        _server->send(200, "text/json", json);
        return;
      } else if (sub.equals("memory")) {
        StaticJsonDocument<512> memoryContent;  //Size from https://arduinojson.org/v6/assistant/
        uint8_t data[32];
        uint8_t bytesRead;
        bytesRead = Box.boxRFID.readBlocks(data, 32);
        if (bytesRead == 32) {
          for (uint8_t i=0; i<bytesRead; i++) {
            memoryContent[i] = data[i];
          }
          size_t len = measureJson(memoryContent)+1;
          char json[len];
          serializeJson(memoryContent, json, len);
          sampleMemory(6);
          _server->send(200, "text/json", json);
          return;
        }
      }
    } else if (cmd.equals("cli")) {
        _server->setContentLength(CONTENT_LENGTH_UNKNOWN); // the payload can go on forever
        _server->sendContent("HTTP/1.1 200 OK\nContent-Type: text\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n");

        String cli = _server->arg("cli");
        String echo = _server->arg("echo");
        cli += "\n";
        WiFiClient client = _server->client();
        Box.logStreamMulti.setSlot((Stream*)&client, 2);
        if (echo == "true")
          Log.print(("hackiebox$ " + cli).c_str());
        Box.boxCLI.cli.parse(cli);
        while(Box.boxCLI.cli.available() || Box.boxCLI.cli.errored())
          Box.boxCLI.loop();
        Box.logStreamMulti.setSlot(NULL, 2);
        sampleMemory(7);
        client.flush();
        client.stop();
        return;
    }
  }
  handleNotFound();
}

void WrapperWebServer::sseHandler(uint8_t channel) {
  Box.boxPower.feedSleepTimer();

  WiFiClient client = _server->client();
  SSESubscription &sseSub = subscription[channel];
  if (sseSub.clientIP != client.remoteIP()) { // IP addresses don't match, reject this client
    Log.info("sseHandler - unregistered client with IP %s tries to listen", _server->client().remoteIP().toString().c_str());
    return handleNotFound();
  }

  sseSub.client = client; // capture SSE _server client connection
  _server->setContentLength(CONTENT_LENGTH_UNKNOWN); // the payload can go on forever
  _server->sendContent("HTTP/1.1 200 OK\nContent-Type: text/event-stream;\nConnection: keep-alive\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n");
  //sseSub.client.isSse = true;

  Log.verbose("sseHandler - registered client with IP %s is listening", IPAddress(sseSub.clientIP).toString().c_str());
  run();
}

void WrapperWebServer::sendJsonSuccess() {
  _server->send(200, "text/json", "{ \"success\": true }");
}

bool WrapperWebServer::commandGetFile(String* path, uint32_t read_start, uint32_t read_length, bool download) {
  File file;
  file = SD_MMC.open((char*)path->c_str());
  if (file) {
    if (read_length == 0 || file.size() < read_length) 
      read_length = file.size();

    String filename = *path;
    int16_t index = filename.lastIndexOf("/");
    if (index != -1)
      filename.remove(0, index+1);

    _server->setContentLength(read_length);
    if (download) {
      _server->sendHeader("Content-Disposition", (String("attachment; filename=\"") + filename + String("\"")).c_str());
      _server->send(200, "application/octet-stream", "");
    } else {
      _server->send(200, "text/html", "");
    }

    uint8_t buffer[512]; //higher buffer size may scramble the stream
    size_t read;
    size_t write;

    file.seek(read_start);
    while (file.position() < read_length) {
      //_server->client().write(file.readChar());
      read = file.read(buffer, sizeof(buffer)); //TODO: may read to much if size is limited
      if (read == 0)
        break; //error or empty

      write = _server->client().write(buffer, read);
      if (write == 0) 
        break; //error*/
    }

    sampleMemory(1);

    file.close();
    return true;
  } else {
    Log.error("Couldn't open %s", path->c_str());
  }
  return false;
}
bool WrapperWebServer::commandGetFlashFile(String* path, uint32_t read_start, uint32_t read_length) {
  /*
  if (SerFlash.open((char*)path->c_str(), FS_MODE_OPEN_READ) == SL_FS_OK) {
    if (read_length == 0 || SerFlash.size() < read_length) 
      read_length = SerFlash.size();

    String filename = *path;
    int16_t index = filename.lastIndexOf("/");
    if (index != -1)
      filename.remove(0, index+1);

    _server->setContentLength(read_length);
    _server->sendHeader("Content-DiFatFssposition", (String("attachment; filename=\"") + filename + String("\"")).c_str());
    _server->send(200, "application/octet-stream", "");

    uint8_t buffer[512];
    size_t read = -1;
    SerFlash.seek(read_start);
    while (SerFlash.available()) {
      read = SerFlash.readBytes(buffer, sizeof(buffer)); //TODO: may read to much if size is limited
      if (read == 0)
        break; //error or empty
      if (_server->client().write(buffer, read) == 0) 
        break; //error
    }

    SerFlash.close();
    return true;
  } else {
    Log.error("Couldn't open %s %s", path->c_str(), SerFlash.lastErrorString());
    SerFlash.close();
  }
  */
  return false;
}

void WrapperWebServer::handleUploadFile() {
  Box.delayTask(0);
  HTTPUpload& upload = _server->upload();
  if (upload.status == UPLOAD_FILE_START) {
    Box.boxPower.feedSleepTimer();
    String filepath = _server->arg("filepath");
    char* filename = (char*)filepath.c_str();
    bool overwrite = false;
    if (!_server->arg("overwrite").equals(""))
      overwrite = true;
    long write_start = _server->arg("start").toInt();

    Log.info("handleUploadFile Name: %s, overwrite: %T, start=%l", filename, overwrite, write_start);

    if (overwrite)
      SD_MMC.remove(filename);
    _uploadFile = SD_MMC.open(filename, FILE_WRITE, true);

    if (_uploadFileOpen) {
      _uploadFile.seek(write_start);
      return;
    }
    Log.error("File couldn't be opened.");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Box.boxPower.feedSleepTimerSilent();
    //Log.verbose("handleUploadFile Data: %i", upload.currentSize);
    if (_uploadFileOpen) {
      _uploadFile.write(upload.buf, upload.currentSize);
      return;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Box.boxPower.feedSleepTimer();
    if (_uploadFileOpen) {
      _uploadFile.close();
      Log.info("handleUploadFile Size: %ikB", upload.totalSize / 1024);
      sendJsonSuccess();
      return;
    }
  }
  handleNotFound();
}

void WrapperWebServer::handleUploadFlashFile() {
  /*
  Box.boxPower.feedSleepTimer();
  Box.delayTask(0);
  HTTPUpload& upload = _server->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filepath = _server->arg("filepath");
    uint64_t flashsize = _server->arg("filesize").toInt();
    const char* filename = (const char*)filepath.c_str();
    bool overwrite = false;
    if (!_server->arg("overwrite").equals(""))
      overwrite = true;
    long write_start = _server->arg("start").toInt();

    Log.info("handleUploadFlashFile Name: %s, overwrite: %T, start=%l", filename, overwrite, write_start);

    if (!overwrite || SerFlash.del(filepath) == SL_FS_OK) {
      Log.info("Filesize %lB", flashsize);
      if (flashsize % 512 != 0) {
        flashsize = ((flashsize / 512) + 1) * 512;
      }
      Log.info("Flashsize %lB", flashsize);
      if (SerFlash.open(filename, FS_MODE_OPEN_CREATE(flashsize, 0x0)) == SL_FS_OK) {
        _uploadFlashFileOpen = true;
        if (overwrite && write_start>0)
          SerFlash.seek(write_start);
        return;
      }
      Log.error("Couldn't open %s %s", filename, SerFlash.lastErrorString());
      SerFlash.close();
    } else {
      Log.error("Couldn't delete %s %s", filename, SerFlash.lastErrorString());
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Log.verbose("handleUploadFlashFile Data: %i", upload.currentSize);
    
    if (_uploadFlashFileOpen) {
      SerFlash.write(upload.buf, upload.currentSize);
      return;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (_uploadFlashFileOpen) {
      SerFlash.close();
      _uploadFlashFileOpen = false;
      Log.info("handleUploadFlashFile Size: %iB", upload.totalSize);
      sendJsonSuccess();
      return;
    }
  }
  */
  handleNotFound();
}

void WrapperWebServer::sendEvent(const char* eventname, const char* content, bool escapeData) {
  bool clientConnected = false;
  for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++) {
    if (!(subscription[i].clientIP)) 
      continue;
    
    Box.logStreamMulti.flush();
    sampleMemory(4);
    if (subscription[i].client.connected()) {
      clientConnected = true;
      subscription[i].client.print("data: { \"type\":\"");
      subscription[i].client.print(eventname);
      if (escapeData) {
        subscription[i].client.print("\", \"data\":\"");
        subscription[i].client.print(content);
        subscription[i].client.print("\" }");
      } else {
        subscription[i].client.print("\", \"data\":");
        subscription[i].client.print(content);
        subscription[i].client.print("}");
      }
      subscription[i].client.println("\n"); // Extra newline required by SSE standard
    } else {
      Log.info("Client not listening on channel %i, remove subscription", i);
      //subscription[i].keepAliveTimer.detach();
      subscription[i].client.flush();
      subscription[i].client.stop();
      subscription[i].clientIP = INADDR_NONE;
      subscriptionCount--;
    }
  }
  if (clientConnected) 
    Box.boxPower.feedSleepTimerSilent();
}
void WrapperWebServer::sseKeepAlive() {
  sendEvent("keep-alive", "");
}

