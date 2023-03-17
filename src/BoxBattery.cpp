#include "BoxBattery.h"
#include "BoxEvents.h"

void BoxBattery::begin() {
    reloadConfig();

    pinMode(8, INPUT); //Charger pin

    _wasLow = false;
    _wasCritical = false;
    _readBatteryAdc();

    batteryTestThread.setInterval(10*60*1000);
    batteryTestThread.enabled = false;

    loop();
    logBatteryStatus();

    setInterval(100);
}
void BoxBattery::loop() {
    _readBatteryAdc();
    _charger.read();
    
    if (_batteryAdcRaw < _batteryAdcLowRaw || isChargerConnected())
        _batteryAdcLowRaw = _batteryAdcRaw;

    if (_charger.wasPressed()) {
        Events.handleBatteryEvent(BatteryEvent::CHR_CONNECT);
    } else if (_charger.wasReleased()) {
        Events.handleBatteryEvent(BatteryEvent::CHR_DISCONNECT);
    }

    if (!isChargerConnected()) {
        if (!_wasCritical && isBatteryCritical()) {
            _wasCritical = true;
            Events.handleBatteryEvent(BatteryEvent::BAT_CRITICAL);
        } else if (!_wasLow && isBatteryLow()) {
            _wasLow = true;
            Events.handleBatteryEvent(BatteryEvent::BAT_LOW);
        }
    } else {
        _wasLow = false;
        _wasCritical = false;
    }
}

void BoxBattery::_readBatteryAdc() {
    uint16_t adcValue = 0; //analogReadAvg(BATTERY_VOLTAGE_PIN, 1);
    _batteryAdcRaw = adcValue;
}

bool BoxBattery::isChargerConnected() {
    if (_charger.isPressed())
        return true;
    return false;
}
uint16_t BoxBattery::getBatteryAdcRaw() {
    return _batteryAdcRaw;
}
uint16_t BoxBattery::getBatteryVoltage() {
    return 1000 * getBatteryAdcRaw() / _batteryVoltageFactor;
}
bool BoxBattery::isBatteryLow() {
    if (getBatteryAdcRaw() < _batteryLowAdc)
        return true;
    return false;
}
bool BoxBattery::isBatteryCritical() {
    if (getBatteryAdcRaw() < _batteryCriticalAdc)
        return true;
    return false;
}

void BoxBattery::logBatteryStatus() {
    int voltageDec = getBatteryVoltage();
    int voltageNum = voltageDec / 100;
    voltageDec = voltageDec - voltageNum * 100;

    Log.info("Battery Stats:");
    Log.info(" Charging: %T", isChargerConnected());
    Log.info(" ADC Raw: %c", getBatteryAdcRaw());
    Log.info(" Estimated Voltage: %d.%s%dV", voltageNum, (voltageDec<10) ? "0": "", voltageDec);
    Log.info(" Battery Low: %T", isBatteryLow());
    Log.info(" Battery Critical: %T", isBatteryCritical());
}

void BoxBattery::reloadConfig() { 
    ConfigStruct* config = Config.get();

    _batteryVoltageFactor = config->battery.voltageFactor;
    _batteryLowAdc = config->battery.lowAdc;
    _batteryCriticalAdc = config->battery.criticalAdc;
}

void BoxBattery::doBatteryTestStep() {
    Log.info("Write battery test data...");

    File file = SD_MMC.open(_batteryTestFilename, FILE_APPEND, true);
    if (file) {
        uint16_t voltageDec = getBatteryVoltage();
        uint8_t voltageNum = voltageDec / 100;
        voltageDec = voltageDec - voltageNum * 100;

        uint16_t timeRunning = (millis()-_batteryTestStartMillis) / (1000*60);
        bool chargerConnected = isChargerConnected();
        uint16_t batteryAdcRaw = getBatteryAdcRaw();
        bool batteryLow = isBatteryLow();
        bool batteryCritical = isBatteryCritical();
        
        char output[5+1 +5+1 +5+1 +3+1+5+5+1 +5+1 +5+1 +1];
        file.printf(output, "%hu;%s;%hu;%hu.%s%hu;%s;%s;",
            timeRunning,
            (chargerConnected ? "true" : "false"),
            batteryAdcRaw,
            voltageNum, (voltageDec<10) ? "0": "", voltageDec,
            (batteryLow ? "true" : "false"),
            (batteryCritical ? "true" : "false")
        );
        Log.info(output);
        file.write((uint8_t*)output, sizeof(output));

        file.write('\r');
        file.write('\n');
        file.close();
    } else {
        Log.error("Couldn't write log %", _batteryTestFilename);
    }
}
void BoxBattery::startBatteryTest() {
    Log.info("Start battery test...");

    batteryTestThread.enabled = true;
    _batteryTestStartMillis = millis();
    File file = SD_MMC.open(_batteryTestFilename, FILE_APPEND, true);
    if (file) {
        char output[26+10+10+1];
        
        file.print("Timestamp;");
        file.print("Charging;");
        file.print("ADC;");
        file.print("Estimated Voltage;");
        file.print("Low;");
        file.print("Critical;");
        file.print("Comments");
        file.print("\r\n");
        file.print("0;;;;;;");
        file.printf(output, "vFactor=%lu;v3-wav", _batteryVoltageFactor);
        file.print(output);
        file.print("\r\n");
        file.close();

        batteryTestThread.run();
        //Box.boxDAC.initBatteryTest();
    } else {
        Log.error("Couldn't init battery log %s", _batteryTestFilename);
        batteryTestThread.enabled = false;
    }
}
void BoxBattery::stopBatteryTest() {
    if (!batteryTestThread.enabled)
        return;
    Log.info("Stop battery test...");
    batteryTestThread.enabled = false;
    doBatteryTestStep();
    File file = SD_MMC.open(_batteryTestFilename, FILE_APPEND, true);
    if (fileno_unlocked) {
        char output[13+5+1];
        uint16_t timeRunning = (millis()-_batteryTestStartMillis) / (1000*60);
        file.printf(output, "%hu;;;;;;stopped", timeRunning);
        file.print(output);
        file.print("\r\n");
        file.close();
    } else {
        Log.error("Couldn't write battery log %s", _batteryTestFilename);
        batteryTestThread.enabled = false;
    }
}
bool BoxBattery::batteryTestActive() {
    return batteryTestThread.enabled;
}

BoxBattery::BatteryStats BoxBattery::getBatteryStats() {
    BoxBattery::BatteryStats stats;
    
    stats.charging = isChargerConnected();
    stats.low = isBatteryLow();
    stats.critical = isBatteryCritical();
    stats.adcRaw = _batteryAdcRaw;
    stats.voltage = getBatteryVoltage();
    stats.testActive = batteryTestActive();
    stats.testActiveMinutes = 0;
    if (stats.testActive)
        stats.testActiveMinutes = (millis()-_batteryTestStartMillis) / (1000*60);

    return stats;
}