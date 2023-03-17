#ifndef BoxSD_h
#define BoxSD_h

#include "BaseHeader.h"
#include "SD_MMC.h"
#include <WebServer.h>

class BoxSD {
    public:
        void
            begin(),
            loop();

        bool isInitialized();

        void webJsonListDir(WebServer* webServer, char* directory);
    private:
        const int _SD_CMD = 38;
        const int _SD_CLK = 35;
        const int _SD_DAT0 = 36;
        const int _SD_DAT1 = 37;
        const int _SD_DAT2 = 33;
        const int _SD_DAT3 = 34;
        bool _initialized = false;
};

#endif