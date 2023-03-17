#include "BoxPower.h"

#include "Hackiebox.h"
#include <driverlib/prcm.h>

void BoxPower::initPins() {
    pinMode(_powerSD, OUTPUT); //SD Power pin
    pinMode(_powerDACPin, OUTPUT); //Audio, Accelerometer, RFID, LED Blue / Red?
    pinMode(_resetDACPin, OUTPUT);
}

void BoxPower::begin() {
    _sleepMinutes = Config.get()->battery.sleepMinutes;
    _lastFeed = millis();
    setInterval(5000);

    Log.info("Init BoxPower, sleepMinutes=%i", _sleepMinutes);
}

void BoxPower::loop() {
    uint32_t millis_tmp = millis();
    uint32_t deltaMinutes = (millis_tmp - _lastFeed) / (1000 * 60);
    if (_sleepMinutes > 0 && deltaMinutes >= _sleepMinutes) {
        Log.verbose("millis_tmp=%l, _lastFeed=%l, deltaMinutes=%l", millis_tmp, _lastFeed, deltaMinutes);
        Events.handlePowerEvent(PowerEvent::BOX_IDLE);
    }
}

void BoxPower::feedSleepTimer() {
    feedSleepTimerSilent();
    Log.verbose("Sleep timer rst, _lastFeed=%l, Mem: free(str/ptr/cny/end) Stack: %ib(%X/%X/%X/%X), Heap: %ib(%X/%X/%X/%X)",
        _lastFeed,
        freeStackMemory(), (uint32_t)stackStart()-0x20004000, (uint32_t)stackPointer()-0x20004000, (uint32_t)getFirstStackCanary()-0x20004000, (uint32_t)stackEnd()-0x20004000,
        freeHeapMemory(), (uint32_t)heapStart()-0x20004000, (uint32_t)heapPointer()-0x20004000, (uint32_t)getFirstHeapCanary()-0x20004000, (uint32_t)heapEnd()-0x20004000
    );
    uint32_t stackCanaries = countStackCanaries();
    uint32_t heapCanaries = countHeapCanaries();
    if (stackCanaries < 10 || heapCanaries < 10) {
        Log.error("!!! Canaries low !!! Stack=%l, Heap=%l", stackCanaries, heapCanaries);
    }
}
void BoxPower::feedSleepTimerSilent() {
    _lastFeed = millis();
    Box.watchdog_feed();
}

void BoxPower::_preparePowerDown() {
    Log.info("Prepare power down...");
    //TODO
    //smartconfig down
    Box.logStreamMulti.flush();
    Box.boxPlayer.stop();
    setSdPower(false);
    setOtherPower(false);
    Box.boxLEDs.setAllBool(false);
    Serial.end();
    Box.watchdog_stop();
}
void BoxPower::reset() {
    Events.handlePowerEvent(PowerEvent::PRE_RESET);

    _preparePowerDown();
    PRCMMCUReset(true);
}
void BoxPower::hibernate() {
    Events.handlePowerEvent(PowerEvent::PRE_HIBERNATE);

    _preparePowerDown();
    //enable ear wakeup interrupt
    PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2 | PRCM_HIB_GPIO4);
    //TODO
    //Utils_SpiFlashDeepPowerDown();
    PRCMHibernateEnter();
}

void BoxPower::setSdPower(bool power) {
    digitalWrite(_powerSD, !power);
}
void BoxPower::setOtherPower(bool power) {
    digitalWrite(_powerDACPin, power);

    if (power) {
        //RESET Chips TODO, invert?
        digitalWrite(_resetDACPin, LOW);
        Box.delayTask(1);
        digitalWrite(62, HIGH);
        Box.delayTask(10);
    }
}

bool BoxPower::getSdPower() {
    return !digitalRead(_powerSD);
}
bool BoxPower::getOtherPower() {
    return digitalRead(_powerDACPin);
}