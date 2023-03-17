#ifndef BoxPower_h
#define BoxPower_h

#include "BaseHeader.h"

#include <EnhancedThread.h>

class BoxPower : public EnhancedThread {
    public:
        enum class PowerEvent {
            PRE_HIBERNATE,
            PRE_RESET,
            BOX_IDLE,
        };
        void
            initPins(),
            begin(),
            loop();
        
        void
            feedSleepTimer(),
            feedSleepTimerSilent(),
            reset(),
            hibernate();

        void
            setSdPower(bool power),
            setOtherPower(bool power);

        bool
            getSdPower(),
            getOtherPower();

    private:
        uint8_t _sleepMinutes;
        uint32_t _lastFeed;
        const int _powerDACPin = 45;
        const int _resetDACPin = 26;
        const int _powerSD = 47;
        
        void _preparePowerDown();
};

#endif