#include "BoxConfig.h"

#define CONFIG_SD_PATH "/revvox/hackiebox.config.json"

void BoxConfig::begin() { 
    read();
}

void BoxConfig::read() {
    _initializeConfig();
    File file = SD_MMC.open(CONFIG_SD_PATH, FILE_READ, false);
    if (file) {
        uint8_t buffer[4096];
        size_t read;
        String json = String();
        while (file.position() < file.size()) {
            read = file.read(buffer, sizeof(buffer)); 
            if (read == 0)
                break;
            for (int i=0; i<read; i++) {
                json += buffer[i];
            }
        }
        file.close();
        if (!setFromJson(json)) {
            Log.error("Couldn't read cfg file %s, recreating it", CONFIG_SD_PATH);
            write();
        }
    } else {
        Log.error("Couldn't read cfg file %s, recreating it", CONFIG_SD_PATH);
        write();
    }
}
void BoxConfig::write() { 
    _json = getAsJson();
    File file = SD_MMC.open(CONFIG_SD_PATH, FILE_WRITE, true);
    if (file) {
        file.write((uint8_t*)_json.c_str(), _json.length());
        file.close();
    } else {
        Log.error("Couldn't write cfg file %", CONFIG_SD_PATH);
    }
}

ConfigStruct* BoxConfig::get() { 
    return &_config;
}

String BoxConfig::getAsJson() { 
    StaticJsonDocument<BOXCONFIG_JSON_SIZE> doc;
    doc["version"] = _config.version;
    
    JsonObject batteryDoc = doc.createNestedObject("battery");
    ConfigBattery* batteryCfg = &_config.battery;
    batteryDoc["voltageFactor"] = batteryCfg->voltageFactor;
    batteryDoc["lowAdc"] = batteryCfg->lowAdc;
    batteryDoc["criticalAdc"] = batteryCfg->criticalAdc;
    batteryDoc["sleepMinutes"] = batteryCfg->sleepMinutes;

    JsonObject buttonsDoc = doc.createNestedObject("buttonEars");
    ConfigButtonEars* buttonCfg = &_config.buttonEars;
    buttonsDoc["longPressMs"] = buttonCfg->longPressMs;
    buttonsDoc["veryLongPressMs"] = buttonCfg->veryLongPressMs;

    JsonObject wifiDoc = doc.createNestedObject("wifi");
    ConfigWifi* wifiCfg = &_config.wifi;
    wifiDoc["ssid"] = wifiCfg->ssid;
    wifiDoc["password"] = wifiCfg->password;

    JsonObject logDoc = doc.createNestedObject("log");
    ConfigLog* logCfg = &_config.log;
    logDoc["sdLog"] = logCfg->sdLog;

    JsonObject miscDoc = doc.createNestedObject("misc");
    ConfigMisc* miscCfg = &_config.misc;
    miscDoc["autodump"] = miscCfg->autodump;
    miscDoc["swd"] = miscCfg->swd;
    miscDoc["watchdogSeconds"] = miscCfg->watchdogSeconds;

    _json = "";
    serializeJson(doc, _json);
    return _json;
}
bool BoxConfig::setFromJson(String json) { 
    StaticJsonDocument<BOXCONFIG_JSON_SIZE> doc;

    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Log.error("deserializeJson() returned %s, %s", err.c_str(), json.c_str());
        return false;
    }

    _config.version = doc["version"].as<uint8_t>();

    JsonObject batteryDoc = doc["battery"];
    ConfigBattery* batteryCfg = &_config.battery;
    batteryCfg->voltageFactor = batteryDoc["voltageFactor"].as<uint32_t>();
    batteryCfg->lowAdc = batteryDoc["lowAdc"].as<uint16_t>();
    batteryCfg->criticalAdc = batteryDoc["criticalAdc"].as<uint16_t>();
    batteryCfg->sleepMinutes = batteryDoc["sleepMinutes"].as<uint8_t>();

    JsonObject buttonsDoc = doc["buttonEars"];
    ConfigButtonEars* buttonCfg = &_config.buttonEars;
    buttonCfg->longPressMs = buttonsDoc["longPressMs"].as<uint16_t>();
    buttonCfg->veryLongPressMs = buttonsDoc["veryLongPressMs"].as<uint16_t>();

    JsonObject wifiDoc = doc["wifi"];
    ConfigWifi* wifiCfg = &_config.wifi;
    strncpy(&wifiCfg->ssid[0], wifiDoc["ssid"].as<char*>(), sizeof(wifiCfg->ssid));
    strncpy(&wifiCfg->password[0], wifiDoc["password"].as<char*>(), sizeof(wifiCfg->password));

    JsonObject logDoc = doc["log"];
    ConfigLog* logCfg = &_config.log;
    logCfg->sdLog = logDoc["sdLog"].as<bool>();

    JsonObject miscDoc = doc["misc"];
    ConfigMisc* miscCfg = &_config.misc;
    miscCfg->autodump = miscDoc["autodump"].as<bool>();
    miscCfg->swd = miscDoc["swd"].as<bool>();
    miscCfg->watchdogSeconds = miscDoc["watchdogSeconds"].as<uint8_t>();

    // Convert old config version to latest one.
    if (_config.version != CONFIG_ACTIVE_VERSION) {
        switch (_config.version) {
        case 2:
            batteryCfg->criticalAdc = batteryDoc["minimalAdc"].as<uint16_t>();
            batteryCfg->lowAdc = batteryCfg->criticalAdc + 100;
            _config.version = 3;
        case 3:
            miscCfg->autodump = false;
            _config.version = 4;
        case 4:
            miscCfg->swd = false;
            _config.version = 5;
        case 5:
            batteryCfg->lowAdc = 9658;
            batteryCfg->criticalAdc = 8869;
            batteryCfg->voltageFactor = 27850;
            _config.version = 6;
        case 6:
            miscCfg->watchdogSeconds = 10;
            if (batteryCfg->voltageFactor == 67690) //Fix for wrong value in previous CFW
                batteryCfg->voltageFactor = 27850;
            _config.version = 7;
            write();
            break;
        default:
            _initializeConfig();
            write();
            break;
        } 
    }

    return true;
}

void BoxConfig::_initializeConfig() { 
    _config.version = CONFIG_ACTIVE_VERSION;

    //(4936,0258+0x59)/(10000/0x663d) = adc
    //(10000/0x663d)×13152−0x59 = v-OFW
    ConfigBattery* battery = &_config.battery;
    battery->voltageFactor = 27850;
    battery->lowAdc = 9658; //OFW 0xE11 (9657,837)
    battery->criticalAdc = 8869; //OFW 0xCE3/0xCE4 (8867,4124/8870,0297)
    battery->sleepMinutes = 15;

    ConfigButtonEars* buttons = &_config.buttonEars;
    buttons->longPressMs = 1000;
    buttons->veryLongPressMs = 10000;

    ConfigWifi* wifi = &_config.wifi;
    wifi->dhcp = true;
    #ifdef WIFI_SSID
        strcpy(wifi->ssid, WIFI_SSID);
    #endif
    #ifdef WIFI_PASS
        strcpy(wifi->password, WIFI_PASS); 
    #endif

    ConfigLog* log = &_config.log;
    log->sdLog = false;

    ConfigMisc* misc = &_config.misc;
    misc->autodump = false;
    misc->swd = false;
    misc->watchdogSeconds = 10;
}