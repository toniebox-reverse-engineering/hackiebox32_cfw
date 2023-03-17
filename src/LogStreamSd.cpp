#include "LogStreamSd.h"

#include "WrapperWebServer.h"
#include "Hackiebox.h"

#include <Logging.h>

size_t LogStreamSd::write(uint8_t character) {
    return write(&character, 1);
}
size_t LogStreamSd::write(const uint8_t *buffer, size_t size) {
    if (!Box.boxSD.isInitialized())
        return 0;
    if (!Config.get()->log.sdLog)
        return 0;/*
    if (!_isOpen) {
        Log.enableAdditionalLogger(false);
        Log.info("LogStreamSd::!_isOpen");
        Log.enableAdditionalLogger(true);
        _isOpen = _file.open("/revvox/logging.log", FA_OPEN_APPEND | FA_WRITE);
        if (!_isOpen) 
            return 0;
    }*/
    _file = SD_MMC.open("/revvox/logging.log", FILE_APPEND, true);
    if (!_file) 
        return 0;
    uint32_t result = _file.write(buffer, size);
    _file.close();
    return result;
}

size_t LogStreamSd::println() {
    size_t result = print("\r\n");
    /*
    if (_isOpen) {
        _file.close();
        _isOpen = false;
    }*/
    return result;
}