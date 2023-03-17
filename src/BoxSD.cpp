#include "BoxSD.h"

void BoxSD::begin() { 
    _initialized = false;
    Log.info("Init SD card");
    SD_MMC.setPins(_SD_CLK, _SD_CMD, _SD_DAT0, _SD_DAT1, _SD_DAT2, _SD_DAT3);
    int result = SD_MMC.begin();
    if (!result) {
        Log.error("SD not mounted. Code %i", result);
        return;
    }

    Log.info(" Capacity: %iMB", SD_MMC.cardSize()/1024/1024);
    Log.info(" Free: %iMB", (SD_MMC.totalBytes()-SD_MMC.usedBytes())/1024/1024);
    _initialized = true;
}

void BoxSD::loop() { 

}

bool BoxSD::isInitialized() {
    return _initialized;
}

void BoxSD::webJsonListDir(WebServer* webServer, char* directory) {
    File dir = SD_MMC.open(directory);
    if (dir && dir.isDirectory()) {;
        webServer->sendContent("{\"files\":[");
        bool firstRound = true;
        File file = dir.openNextFile();
        while (file) {
            StaticJsonDocument<361> fileJson; //Maximum 256 chars filename length //https://arduinojson.org/v6/assistant/

            fileJson["name"] = file.name(); 
            fileJson["size"] = file.size();
            //fileJson["time"] = file. TODO!; 
            //fileJson["date"] = file. TODO!;
            fileJson["dir"] = file.isDirectory();

            size_t len = measureJson(fileJson)+1;
            char json[len];
            serializeJson(fileJson, json, len); //TODO directly stream to save mem
            if (!firstRound)
                webServer->sendContent(","); 
            webServer->sendContent(json); 
            firstRound = false;
            dir.openNextFile();
        }
        dir.close();
        webServer->sendContent("]}");
    } else {
        StaticJsonDocument<299> doc; //Maximum 256 chars path length //https://arduinojson.org/v6/assistant/
        doc["error"] = "Dir not found";
        Log.error("Dir %s not found", directory);

        size_t len = measureJson(doc)+1;
        char json[len];
        serializeJson(doc, json, len); //TODO directly stream to save mem
        webServer->sendContent(json); 
    }
}