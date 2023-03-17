#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include <ThreadController.h>

#include "BoxAccelerometer.h"
#include "BoxBattery.h"
#include "BoxButtonEars.h"
#include "BoxCLI.h"
//#include "BoxDAC.h"
#include "BoxEvents.h"
#include "BoxI2C.h"
#include "BoxLEDs.h"
#include "BoxPlayer.h"
#include "BoxPower.h"
#include "BoxRFID.h"
#include "BoxSD.h"
#include "BoxTonies.h"

#include "WrapperWiFi.h"
#include "WrapperWebServer.h"

/*
#include "LogStreamMulti.h"
#include "LogStreamSd.h"
#include "LogStreamSse.h
*/

class Hackiebox { 
    public:

        void
            setup(),
            loop();
        
        bool
            watchdog_start(uint8_t timeoutS),
            watchdog_isFed();
        void
            watchdog_stop(),
            watchdog_feed(),
            watchdog_unfeed();

        bool inDelayTask;
        void delayTask(uint16_t millis);
        void delayTaskWork(uint16_t millis);


        ThreadController threadController;
        
        BoxAccelerometer boxAccel;
        BoxBattery boxBattery;
        BoxButtonEars boxEars; 
        BoxCLI boxCLI; 
        //BoxDAC boxDAC;
        BoxI2C boxI2C;
        BoxLEDs boxLEDs;
        BoxPower boxPower;
        BoxPlayer boxPlayer;
        BoxRFID boxRFID;
        BoxSD boxSD;
        BoxTonies boxTonie;
        WrapperWiFi boxWiFi;
        WrapperWebServer webServer;
        /*
        LogStreamMulti logStreamMulti;
        LogStreamSd logStreamSd;
        LogStreamSse logStreamSse;
        */
        bool watchdog_enabled;
    private:/*
        typedef void (*fAPPWDTDevCallbk)();
        void 
            watchdog_handler();*/
        
        bool _watchdog_fed;
};
extern Hackiebox Box;

#endif
