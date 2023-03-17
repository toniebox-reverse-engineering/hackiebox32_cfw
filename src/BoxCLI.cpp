#include "BoxCLI.h"

#include "Hackiebox.h"
#include "BoxEvents.h"
#include <ESP8266SAM.h>

void BoxCLI::begin() {
    setInterval(50);

    cmdI2C = cli.addCmd("i2c");
    cmdI2C.setDescription(" Access I2C");
    cmdI2C.addFlagArg("re/ad");
    cmdI2C.addFlagArg("wr/ite");
    cmdI2C.addArg("a/ddress");
    cmdI2C.addArg("r/egister");
    cmdI2C.addArg("v/alue", "");
    cmdI2C.addArg("l/ength", "1");
    cmdI2C.addArg("o/utput", "B");

    cmdSpiRFID = cli.addCmd("spi-rfid");
    cmdSpiRFID.setDescription(" Access RFID SPI");
    cmdSpiRFID.addFlagArg("re/ad");
    cmdSpiRFID.addFlagArg("wr/ite");
    cmdSpiRFID.addFlagArg("c/md,co/mmand");
    cmdSpiRFID.addArg("r/egister", "0");
    cmdSpiRFID.addArg("v/alue", "0");

    cmdBeep = cli.addCmd("beep");
    cmdBeep.setDescription(" Beep with build-in DAC synthesizer");
    cmdBeep.addArg("m/idi-id", "60");
    cmdBeep.addArg("l/ength", "200");

    cmdRFID = cli.addCmd("rfid");
    cmdRFID.setDescription(" Access RFID");
    cmdRFID.addFlagArg("u/id");
    cmdRFID.addFlagArg("r/ead");
    cmdRFID.addFlagArg("m/emory");
    cmdRFID.addFlagArg("d/ump");
    cmdRFID.addFlagArg("o/verwrite");
    
    cmdLoad = cli.addCmd("load");
    cmdLoad.setDescription(" Shows the load of all threads");
    cmdLoad.addArg("n/ame", "");
    cmdLoad.addArg("p/ointer", "0");
    cmdLoad.addFlagArg("r/eset");

    cmdHelp = cli.addSingleArgumentCommand("help");
    cmdHelp.setDescription(" Show this screen");

    cmdI2S = cli.addCmd("i2s");
    cmdI2S.setDescription(" I2S debug information");
    cmdI2S.addFlagArg("l/og");
    cmdI2S.addArg("t/est", 0);
    cmdI2S.addArg("f/requency", "440");

    cmdSay = cli.addCmd("say");
    cmdSay.setDescription(" Generate speech with SAM");
    cmdSay.addPosArg("t/ext");
    cmdSay.addArg("v/oice", "0");
    cmdSay.addArg("s/peed", "0");
    cmdSay.addArg("p/itch", "0");
    cmdSay.addArg("t/hroat", "0");
    cmdSay.addArg("m/outh", "0");
    cmdSay.addFlagArg("sing");
    cmdSay.addFlagArg("p/hoentic");

    cmdAudio = cli.addCmd("audio");
    cmdAudio.setDescription(" Play/Pause audio files");
    cmdAudio.addFlagArg("p/lay,pause");
    cmdAudio.addArg("f/ile/name", "");
    cmdAudio.addArg("g/en-limit", "0");
    cmdAudio.addArg("b/uffer", "-1");
    cmdAudio.addArg("m/axSampleRate", "0");
}

void BoxCLI::loop() {
    parse();
}
void BoxCLI::parse() {
    if (cli.available()) {
        lastCmd = cli.getCmd();

        if (lastCmd == cmdHelp) {
            Log.println("Help:");
            Log.println();
            if (lastCmd.getArg(0).isSet()) {
                String arg = lastCmd.getArg(0).getValue();
                Command cmd = cli.getCmd(arg);
                if (cmd.getName() == arg) {
                    Log.println(cmd.toString().c_str());
                    return;
                }
            }
            
            Log.println(cli.toString().c_str());
        } else if (lastCmd == cmdI2C) {
            execI2C();
        } else if (lastCmd == cmdSpiRFID) {
            execSpiRFID();
        } else if (lastCmd == cmdBeep) {
            execBeep();
        } else if (lastCmd == cmdRFID) {
            execRFID();
        } else if (lastCmd == cmdLoad) {
            execLoad();
        } else if (lastCmd == cmdI2S) {
            execI2S();
        } else if (lastCmd == cmdSay) {
            execSay();
        } else if (lastCmd == cmdAudio) {
            execAudio();
        }
    }

    if (cli.errored()) {
        CommandError cmdError = cli.getError();

        Log.print("ERROR: ");
        Log.println(cmdError.toString().c_str());

        if (cmdError.hasCommand()) {
            Log.print("Did you mean \"");
            Log.print(cmdError.getCommand().toString().c_str());
            Log.println("\"?");
        }
    }
}

void BoxCLI::execI2C() {
    Command c = lastCmd;
    //int argNum = c.countArgs();

    unsigned long int tmpNum;
    uint8_t addr, regi;

    bool read = c.getArg("read").isSet();
    bool write = c.getArg("write").isSet();
    if (read == write) {
        Log.error("Either read or write has to be set");
        return;
    }

    String saddress = c.getArg("address").getValue();
    String sregister = c.getArg("register").getValue();

    tmpNum = parseNumber(saddress);
    if (tmpNum > 127) {
        Log.error("address must be <128");
        return;
    }
    addr = (uint8_t)tmpNum;

    tmpNum = parseNumber(sregister);
    if (tmpNum > 255) {
        Log.error("register must be <256");
        return;
    }
    regi = (uint8_t)tmpNum;

    //Log.printfln("I2C command read=%T, write=%T, addr=%X, regi=%X", read, write, addr, regi);
    String slength = c.getArg("length").getValue();
    String svalue = c.getArg("value").getValue();
    if (read) {
        String soutput = c.getArg("output").getValue();
        if (!(soutput == "x" || soutput == "X" || soutput == "i" || soutput == "b" || soutput == "B")) {
            Log.error("Allowed output values are: x, X, i, b, B");
            return;
        }

        if (slength.length() == 0) {
            Log.error("length must be specified");
            return;
        }
        tmpNum = parseNumber(slength);
        unsigned long len = tmpNum;
        
        Log.printfln("Read %i bytes", len);
        for (uint16_t i=0; i<len; i++) {
            String format = String(" %") + soutput;
            uint8_t result = Box.boxI2C.readByte(addr, regi+i);
            Log.printf(format.c_str(), result);
        }
    } else if (write) {
        char* value = (char*)svalue.c_str();
        char* newVal;
        while (strlen(value)>0) {
            while(isspace((unsigned char)*value)) value++;
            tmpNum = parseNumber(value, &newVal);
            if (tmpNum > 255) {
                Log.error("value must be <256");
                return;
            } else if (value == newVal) {
                Log.error("Couldn't parse part \"%s\" of \"%s\"", value, svalue.c_str());
                return;
            }
            uint8_t data = (uint8_t)tmpNum;
            bool result = Box.boxI2C.send(addr, regi++, data);
            if (!result) {
                Log.error(" %X not written, abort", data);
                break;
            }
            Log.info(" %X successful written", data);
            value = newVal;
        }
    }
}

void BoxCLI::execSpiRFID() {
    Command c = lastCmd;
    unsigned long tmpNum;

    String sregister = c.getArg("register").getValue();
    tmpNum = parseNumber(sregister);
    if (tmpNum > 255) {
        Log.error("register must be <256");
        return;
    }
    uint8_t regi = (uint8_t)tmpNum;

    String svalue = c.getArg("value").getValue();
    tmpNum = parseNumber(svalue);
    if (tmpNum > 255) {
        Log.error("value/command must be <256");
        return;
    }
    uint8_t value = (uint8_t)tmpNum;

    bool read = c.getArg("read").isSet();
    bool write = c.getArg("write").isSet();
    bool cmd = c.getArg("command").isSet();

    //TODO Exclusive check

    if (read) {
        uint8_t result = Box.boxRFID.readRegister(regi);
        Log.info("Read from register=%X value=%X", regi, result);
    } else if (write) {
        Box.boxRFID.writeRegister(regi, value);
        Log.info("Write to register=%X value=%X", regi, value);
    } else if (cmd) {
        Box.boxRFID.sendCommand(value);
        Log.info("Send command %X", value);
    }
}

void BoxCLI::execBeep() {
    Command c = lastCmd;
    unsigned long tmpNum;

    String sid = c.getArg("midi-id").getValue();
    tmpNum = parseNumber(sid);
    if (tmpNum > 127) {
        Log.error("midi-id must be <128");
        return;
    }
    uint8_t id = (uint8_t)tmpNum;

    String slength = c.getArg("length").getValue();
    tmpNum = parseNumber(slength);
    if (tmpNum > 65535) {
        Log.error("length must be <65.536");
        return;
    }
    uint16_t length = (uint16_t)tmpNum;

    Box.boxDAC.beepMidi(id, length);
}

void BoxCLI::execRFID() {
    Command c = lastCmd;

    if (c.getArg("read").isSet()) {
        if (Box.boxRFID.tagActive) {
            Box.boxRFID.tagActive = false;
            Events.handleTagEvent(BoxRFID::TAG_EVENT::TAG_REMOVED);
        }
        Box.boxRFID.loop();
        if (!Box.boxRFID.tagActive) {
            Log.error("No tag in place");
        }
    } else if (c.getArg("uid").isSet()) {
        if (!Box.boxRFID.tagActive) {
            Log.error("No tag in place, last known is shown");
        }
        Box.boxRFID.logUID();
    } else if (c.getArg("memory").isSet()) {
        Box.boxRFID.loop();
        if (!Box.boxRFID.tagActive) {
            Log.error("No tag in place");
        }
        Box.boxRFID.logTagMemory();
    } else if (c.getArg("dump").isSet()) {
        Box.boxRFID.loop();
        if (!Box.boxRFID.tagActive) {
            Log.error("No tag in place");
        }
        Box.boxRFID.dumpTagMemory(c.getArg("overwrite").isSet());
    } else {
        Log.error("Nothing to do...");
    }
}

void BoxCLI::execLoad() {
    Command c = lastCmd;

    String name = c.getArg("name").getValue();
    String pointerStr = c.getArg("pointer").getValue();
    bool reset = c.getArg("reset").isSet();
    unsigned long pointer = parseNumber(pointerStr);

    if (name != "") {
        Log.info("Statistics for Threads starting with \"%s\"", name.c_str());
        Log.println("---");
        for (uint8_t i = 0; i < Box.threadController.size(); i++) {
            EnhancedThread* thread = (EnhancedThread*)Box.threadController.get(i);
            if (strncasecmp(name.c_str(), thread->ThreadName, name.length()) == 0) {
                thread->logStats();
                if (reset) thread->resetStats();
                Log.println();
            }
            Box.delayTask(1);
        }
    } else if (pointer > 0) {
        Log.info("Statistics for Threads with pointer=%i", pointer);
        Log.println("---");
        for (uint8_t i = 0; i < Box.threadController.size(); i++) {
            EnhancedThread* thread = (EnhancedThread*)Box.threadController.get(i);
            if (pointer == thread->ThreadID) {
                thread->logStats();
                if (reset) thread->resetStats();
                Log.println();
            }
        }
        Box.delayTask(1);
    } else {
        Log.info("Statistics for all %i Threads", Box.threadController.size(false)); //TODO ThreadController
        Log.println("---");
        for (uint8_t i = 0; i < Box.threadController.size(); i++) {
            EnhancedThread* thread = (EnhancedThread*)Box.threadController.get(i);
            thread->logStats();
            if (reset) thread->resetStats();
            Log.println();
            Box.delayTask(1);
        }
    }

    if (reset) Log.info("All stats of selected threads reset.");
}

void BoxCLI::execI2S() {
    /*
    Command c = lastCmd;
    unsigned long freq = parseNumber(c.getArg("frequency").getValue());
    unsigned long testTime = parseNumber(c.getArg("test").getValue());
    if (c.getArg("log").isSet()) {
        Box.boxDAC.audioBuffer.logState();
        Box.boxDAC.logDmaIrqChanges();
    }
    if (testTime > 0 && freq > 0) {
        Log.info("Run frequency test for %ims with %iHz", testTime, freq);
        BoxTimer timer;
        timer.setTimer(testTime);
        while (timer.isRunning()) {
            Box.boxDAC.generateFrequency(freq, timer.getTimeTillEnd());
            timer.tick();
        }
        Box.boxDAC.audioOutput->flush();

        if (c.getArg("log").isSet()) {
            Box.boxDAC.audioBuffer.logState();
            Box.boxDAC.logDmaIrqChanges();
        }
    }
    */
}

void BoxCLI::execSay() {
    /*
    Command c = lastCmd;
    String text = c.getArg("text").getValue();
    ESP8266SAM::SAMVoice voice = (ESP8266SAM::SAMVoice)parseNumber(c.getArg("voice").getValue());
    uint8_t speed = parseNumber(c.getArg("speed").getValue());
    uint8_t pitch = parseNumber(c.getArg("pitch").getValue());
    uint8_t throat = parseNumber(c.getArg("throat").getValue());
    uint8_t mouth = parseNumber(c.getArg("mouth").getValue());
    bool sing = c.getArg("sing").isSet();
    bool phoentic = c.getArg("phoentic").isSet();

    if (text != "") {
        Box.boxDAC.samSay(
            text.c_str(),
            voice,
            speed,
            pitch,
            throat,
            mouth,
            sing,
            phoentic
        );
    }
    */
}

void BoxCLI::execAudio() {
    /*
    Command c = lastCmd;

    String filenameStr = c.getArg("file").getValue();
    uint16_t genLimit = (uint16_t)parseNumber(c.getArg("gen-limit").getValue());

    uint32_t resampleRate = (uint16_t)parseNumber(c.getArg("maxSampleRate").getValue());
    String bufferStr = c.getArg("buffer").getValue();

    int32_t buffer = -1;
    if (bufferStr != "-1")
        buffer = parseNumber(bufferStr);
    if (bufferStr != "0" && buffer == 0)
        buffer = -1;

    if (c.getArg("file").isSet() && filenameStr != "") {
        Box.boxDAC.playFile(filenameStr.c_str());
    } else if (c.getArg("play").isSet()) { 
        Box.boxDAC.audioPlaying = !Box.boxDAC.audioPlaying;
    } else {
        if (genLimit > 0)
            Box.boxDAC.audioTimeoutMs = genLimit;
        Log.info("Generator time limit is set to %ims", Box.boxDAC.audioTimeoutMs);

        if (resampleRate > 0) {
            Box.boxDAC.audioOutputResample->SetMaxRate(resampleRate);
            Log.info("Max Samplerate is set to %ihz", Box.boxDAC.audioOutputResample->GetMaxRate());
        }
        if (buffer == 0) {
            Log.info("Additional buffering off");
            Box.boxDAC.audioOutput = Box.boxDAC.audioOutputResample;
        } else if (buffer > 0) {
            if (buffer <= freeHeapMemory() / 2) {
                Box.boxDAC.audioOutput = Box.boxDAC.audioOutputBuffer;
                free(Box.boxDAC.audioOutputBuffer);
                Box.boxDAC.audioOutputBuffer = new AudioOutputBuffer(buffer, Box.boxDAC.audioOutputResample);
                Box.boxDAC.audioOutput = Box.boxDAC.audioOutputBuffer;
                Log.info("Additional buffer set to %ib", buffer);
            } else {
                Log.error("Buffer should not use more than half of the HEAP (%ib)", freeHeapMemory());
            }
        }
    }
    */
}

unsigned long BoxCLI::parseNumber(String numberString) {
    const char* num = numberString.c_str();
    return parseNumber((char*)num);
}
unsigned long BoxCLI::parseNumber(char* number) {
    return parseNumber(number, NULL);
}
unsigned long BoxCLI::parseNumber(char* number, char** rest) {
    if (strncmp(number, "0x", 2) == 0) {
        return strtoul(number, rest, 16);
    } else if (strncmp(number, "0b", 2) == 0) {
        return strtoul(number+2, rest, 2);
    }
    return strtoul(number, rest, 10);
}