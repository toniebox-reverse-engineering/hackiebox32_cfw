#ifndef BoxDAC_h
#define BoxDAC_h

#include "BaseHeader.h"
#include <EnhancedThread.h>

#include "AudioCustomLogger.h"

#include "BoxAudioBufferTriple.h"
#include <AudioOutputI2S.h>
#include "AudioOutputResample.h"
#include <AudioOutputBuffer.h>
#include <ESP8266SAM.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorTonie.h"
#include <AudioGenerator.h>
#include "AudioGeneratorWAV.h"

//#include "libopus/opus.h"

class BoxDAC : public EnhancedThread { 
    public:
        enum class HeadphoneEvent {
           INSERTED,
           REMOVED
        };
        void
            begin(),
            loop(),
            loop(uint16_t timeoutMs);

        void opusTest();

        void generateZeroAudio(uint16_t timeoutMs);

        void generateFrequency(uint32_t frequency, uint16_t timeoutMs);

        void beepTest();
        void beep();
        void beepMidi(uint8_t midiId, uint16_t lengthMs, bool async=false);
        void beepRaw(uint16_t sin, uint16_t cos, uint32_t length);
        void beepRaw(uint16_t sin, uint16_t cos, uint32_t length, uint8_t volume);

        void samSay(const char *text, enum ESP8266SAM::SAMVoice voice = ESP8266SAM::SAMVoice::VOICE_SAM, uint8_t speed = 0, uint8_t pitch = 0, uint8_t throat = 0, uint8_t mouth = 0, bool sing = false, bool phoentic = false);

        void dmaPingPingComplete();

        void initBatteryTest();
        void batteryTestLoop();

        BoxAudioBufferTriple audioBuffer;
        unsigned long dmaIRQcount = 0;
        unsigned long lastDmaIRQcount = 0xFFFF;

        unsigned long dmaBufferFilled = 0;
        unsigned long lastDmaBufferFilled = 0xFFFF;
        unsigned long dmaBufferEmpty = 0;
        unsigned long lastDmaBufferEmpty = 0xFFFF;

        unsigned long priIndexRx = 0;
        unsigned long lastPriIndexRx = 0xFFFF;
        unsigned long altIndexRx = 0;
        unsigned long lastAltIndexRx = 0xFFFF;

        void logDmaIrqChanges();

        const int16_t amplitude = 500; // amplitude of square wave
        int16_t sample = amplitude; // current sample value
        uint32_t count = 0;
        unsigned long i2sElmCount = 0;
        unsigned long i2sStartMicros = 0;

        BoxAudioBufferTriple::BufferStruct* writeBuffer;

        AudioOutputI2S* audioOutputI2S;
        AudioOutputBuffer* audioOutputBuffer;
        AudioOutputResample* audioOutputResample;
        AudioOutput* audioOutput;
        AudioFileSource* audioSource;
        AudioGenerator* audioGenerator;
        bool audioPlaying = false;
        uint16_t audioTimeoutMs = 40;

        bool playFile(const char* path);
        bool _playWAV(const char* path);

        const static uint8_t VOL_MIN = 0x2F; //0xB0+0x7F; //0xB0=-40.0dB /min allowed value 0x81=-63.5dB
        const static uint8_t VOL_MAX = 0x89; //0x0A+0x7F; //0x0A=+04.0dB /max allowed value 0x30=+24.0dB
        const static uint8_t VOL_STEP = 0x06; //3dB
        const static uint8_t VOL_TEST = VOL_MIN + 6*VOL_STEP;
        uint8_t current_volume;

        //const static uint8_t VOL_BEEP_MIN = 0x2A; //0x2A=-40dB /min allowed value 0x3F=-61dB
        //const static uint8_t VOL_BEEP_MAX = 0x00; //0x00=+02dB /max allowed value 0x00=+02dB

        //0x2A
        //0x27
        //0x24
        //0x21
        //0x1E
        //0x1B
        //0x18
        //0x14
        //0x11
        //0x0E
        //0x0B
        //0x08
        //0x05
        //0x02
        //0x00

        bool increaseVolume();
        bool decreaseVolume();

        void setVolume(uint8_t volume);
        uint8_t convertDacVol2BeepVol(uint8_t dacVol);
        void logVolume();
        void logBeepVolume(uint8_t volume);

        void muteSpeaker(bool mute);
        void muteHeadphones(bool mute);

        void
            play(),
            pause(),
            stop();
        bool hasStopped();
    
    private:
        uint8_t _batteryTestFileId;
        AudioGeneratorWAV _genWAV;
        AudioFileSourceSD _srcSD;

        enum class PAGE {
            SERIAL_IO = 0x00,
            DAC_OUT_VOL = 0x01,
            MCLK_DIVIDER = 0x03,
            DAC_FILTER_DRC_COE_1A = 0x08,
            DAC_FILTER_DRC_COE_2A = 0x09,
            DAC_FILTER_DRC_COE_1B = 0x0C,
            DAC_FILTER_DRC_COE_2B = 0x0D,
        };
        enum class ADDR {
            PAGE_CONTROL = 0x00,
        };
        enum class ADDR_P0_SERIAL {
            SOFTWARE_RESET = 0x01,
            CLOCKGEN_MUX = 0x04,
            PLL_P_R_VAL = 0x05,
            PLL_J_VAL = 0x06,
            PLL_D_VAL_MSB = 0x07,
            PLL_D_VAL_LSB = 0x08,
            DAC_NDAC_VAL = 0x0B,
            DAC_MDAC_VAL = 0x0C,
            DAC_DOSR_VAL_MSB = 0x0D,
            DAC_DOSR_VAL_LSB = 0x0E,
            CODEC_IF_CTRL1 = 0x1B,
            DAC_FLAG_REG = 0x26,
            DAC_INTR_FLAGS = 0x2C,
            INTR_FLAGS = 0x2E,
            INT1_CTRL_REG = 0x30,
            GPIO1_INOUT_CTRL = 0x33,
            DAC_PROC_BLOCK_SEL = 0x3C,
            DAC_DATA_PATH_SETUP = 0x3F,
            DAC_VOL_CTRL = 0x40,
            DAC_VOL_L_CTRL = 0x41,
            DAC_VOL_R_CTRL = 0x42,
            HEADSET_DETECT = 0x43,
            BEEP_L_GEN = 0x47,
            BEEP_R_GEN = 0x48,
            BEEP_LEN_MSB = 0x49,
            BEEP_LEN_MID = 0x4A,
            BEEP_LEN_LSB = 0x4B,
            BEEP_SIN_MSB = 0x4C,
            BEEP_SIN_LSB = 0x4D,
            BEEP_COS_MSB = 0x4E,
            BEEP_COS_LSB = 0x4F,
            VOL_MICDET_SAR_ADC = 0x74,
        };
        enum class ADDR_P1_DAC_OUT {
            HP_DRIVERS = 0x1F,
            SPK_AMP = 0x20,
            HP_OUT_POP_REM_SET = 0x21,
            OUT_PGA_RAMP_DOWN_PER_CTRL = 0x22,
            DAC_LR_OUT_MIX_ROUTING = 0x23,
            L_VOL_TO_HPL = 0x24,
            R_VOL_TO_HPR = 0x25,
            L_VOL_TO_SPK = 0x26,
            HPL_DRIVER = 0x28,
            HPR_DRIVER = 0x29,
            SPK_DRIVER = 0x2A,
            HP_DRIVER_CTRL = 0x2C,
            MICBIAS = 0x2E,
        };
        enum class ADDR_P3_MCLK {
            TIMER_CLK_MCLK_DIV = 0x10,
        };

        bool
            send(uint8_t target_register, uint8_t data),
            send(ADDR target_register, PAGE data),
            send(ADDR_P0_SERIAL target_register, uint8_t data),
            send(ADDR_P1_DAC_OUT target_register, uint8_t data),
            send(ADDR_P3_MCLK target_register, uint8_t data);
        
        uint8_t 
            readByte(uint8_t source_register),
            readByte(ADDR source_register),
            readByte(ADDR_P0_SERIAL source_register),
            readByte(ADDR_P1_DAC_OUT source_register),
            readByte(ADDR_P3_MCLK source_register);

        void initDACI2C();
        void checkHeadphoneState();

        uint8_t getSampleRateIndex();
        uint16_t ofwButtonFreqTable[5][4][2] = {
            {   //16000
                {0x278A, 0x79BD}, //+
                {0x30F9, 0x763F}, //++
                {0x18F5, 0x7D87}, //-
                {0x0F0A, 0x7F1A}  //--
            }, {//22050
                {0x1CEA, 0x7CB1}, //+
                {0x23F9, 0x7AD5}, //++
                {0x122A, 0x7EB2}, //-
                {0x0AED, 0x7F87}  //--
            }, {//32000
                {0x1404, 0x7E6D}, //+
                {0x18F7, 0x7D8A}, //++
                {0x0C8A, 0x7F61}, //-
                {0x0788, 0x7FC7}  //--
            }, {//44100
                {0x0E8D, 0x7F2B}, //+
                {0x122C, 0x7EB4}, //++
                {0x091B, 0x7FAC}, //-
                {0x0578, 0x7FE2}  //--
            }, {//48000
                {0x0D60, 0x7F4D}, //+
                {0x10B4, 0x7EE7}, //++
                {0x085E, 0x7FB9}, //-
                {0x0506, 0x7FE6}  //--
            }
        };

        uint32_t frequencyTable[128] = {
            818, 
            866, 
            918, 
            972, 
            1030, 
            1091, 
            1156, 
            1225, 
            1298, 
            1375, 
            1457, 
            1543, 
            1635, 
            1732, 
            1835, 
            1945, 
            2060, 
            2183, 
            2312, 
            2450, 
            2596, 
            2750, 
            2914, 
            3087, 
            3270, 
            3465, 
            3671, 
            3889, 
            4120, 
            4365, 
            4625, 
            4900, 
            5191, 
            5500, 
            5827, 
            6174, 
            6541, 
            6930, 
            7342, 
            7778, 
            8241, 
            8731, 
            9250, 
            9800, 
            10383, 
            11000, 
            11654, 
            12347, 
            13081, 
            13859, 
            14683, 
            15556, 
            16481, 
            17461, 
            18500, 
            19600, 
            20765, 
            22000, 
            23308, 
            24694, 
            26163, 
            27718, 
            29366, 
            31113, 
            32963, 
            34923, 
            36999, 
            39200, 
            41530, 
            44000, 
            46616, 
            49388, 
            52325, 
            55437, 
            58733, 
            62225, 
            65926, 
            69846, 
            73999, 
            78399, 
            83061, 
            88000, 
            93233, 
            98777, 
            104650, 
            110873, 
            117466, 
            124451, 
            131851, 
            139691, 
            147998, 
            156798, 
            166122, 
            176000, 
            186466, 
            197553, 
            209300, 
            221746, 
            234932, 
            248902, 
            263702, 
            279383, 
            295996, 
            313596, 
            332244, 
            352000, 
            372931, 
            395107, 
            418601, 
            443492, 
            469864, 
            497803, 
            527404, 
            558765, 
            591991, 
            627193, 
            664488, 
            704000, 
            745862, 
            790213, 
            837202, 
            886984, 
            939727, 
            995606, 
            1054808, 
            1117530, 
            1183982, 
            1254385
        };


        uint16_t beepTable48000[127][2] = {
            { 0x23, 0x8000 }, 
            { 0x25, 0x8000 }, 
            { 0x27, 0x8000 }, 
            { 0x2A, 0x8000 }, 
            { 0x2C, 0x8000 }, 
            { 0x2F, 0x8000 }, 
            { 0x32, 0x8000 }, 
            { 0x35, 0x8000 }, 
            { 0x38, 0x8000 }, 
            { 0x3B, 0x8000 }, 
            { 0x3E, 0x8000 }, 
            { 0x42, 0x8000 }, 
            { 0x46, 0x8000 }, 
            { 0x4A, 0x8000 }, 
            { 0x4F, 0x8000 }, 
            { 0x53, 0x8000 }, 
            { 0x58, 0x8000 }, 
            { 0x5E, 0x8000 }, 
            { 0x63, 0x8000 }, 
            { 0x69, 0x8000 }, 
            { 0x6F, 0x8000 }, 
            { 0x76, 0x8000 }, 
            { 0x7D, 0x8000 }, 
            { 0x84, 0x8000 }, 
            { 0x8C, 0x8000 }, 
            { 0x95, 0x8000 }, 
            { 0x9D, 0x8000 }, 
            { 0xA7, 0x8000 }, 
            { 0xB1, 0x8000 }, 
            { 0xBB, 0x7FFF }, 
            { 0xC6, 0x7FFF }, 
            { 0xD2, 0x7FFF }, 
            { 0xDF, 0x7FFF }, 
            { 0xEC, 0x7FFF }, 
            { 0xFA, 0x7FFF }, 
            { 0x109, 0x7FFF }, 
            { 0x119, 0x7FFF }, 
            { 0x129, 0x7FFF }, 
            { 0x13B, 0x7FFE }, 
            { 0x14E, 0x7FFE }, 
            { 0x161, 0x7FFE }, 
            { 0x176, 0x7FFE }, 
            { 0x18D, 0x7FFE }, 
            { 0x1A4, 0x7FFD }, 
            { 0x1BD, 0x7FFD }, 
            { 0x1D8, 0x7FFD }, 
            { 0x1F4, 0x7FFC }, 
            { 0x212, 0x7FFC }, 
            { 0x231, 0x7FFB }, 
            { 0x252, 0x7FFB }, 
            { 0x276, 0x7FFA }, 
            { 0x29B, 0x7FF9 }, 
            { 0x2C3, 0x7FF8 }, 
            { 0x2ED, 0x7FF7 }, 
            { 0x319, 0x7FF6 }, 
            { 0x349, 0x7FF5 }, 
            { 0x37B, 0x7FF4 }, 
            { 0x3B0, 0x7FF2 }, 
            { 0x3E8, 0x7FF1 }, 
            { 0x423, 0x7FEF }, 
            { 0x462, 0x7FED }, 
            { 0x4A5, 0x7FEA }, 
            { 0x4EB, 0x7FE8 }, 
            { 0x536, 0x7FE5 }, 
            { 0x585, 0x7FE2 }, 
            { 0x5D9, 0x7FDE }, 
            { 0x632, 0x7FDA }, 
            { 0x691, 0x7FD5 }, 
            { 0x6F4, 0x7FD0 }, 
            { 0x75E, 0x7FCA }, 
            { 0x7CE, 0x7FC3 }, 
            { 0x845, 0x7FBC }, 
            { 0x8C3, 0x7FB3 }, 
            { 0x948, 0x7FAA }, 
            { 0x9D5, 0x7F9F }, 
            { 0xA6A, 0x7F93 }, 
            { 0xB08, 0x7F86 }, 
            { 0xBB0, 0x7F77 }, 
            { 0xC61, 0x7F66 }, 
            { 0xD1D, 0x7F54 }, 
            { 0xDE4, 0x7F3F }, 
            { 0xEB6, 0x7F27 }, 
            { 0xF95, 0x7F0C }, 
            { 0x1081, 0x7EEE }, 
            { 0x117B, 0x7ECD }, 
            { 0x1283, 0x7EA8 }, 
            { 0x139B, 0x7E7D }, 
            { 0x14C3, 0x7E4E }, 
            { 0x15FB, 0x7E19 }, 
            { 0x1746, 0x7DDE }, 
            { 0x18A4, 0x7D9B }, 
            { 0x1A16, 0x7D50 }, 
            { 0x1B9D, 0x7CFC }, 
            { 0x1D3B, 0x7C9E }, 
            { 0x1EEF, 0x7C35 }, 
            { 0x20BC, 0x7BBE }, 
            { 0x22A2, 0x7B3A }, 
            { 0x24A2, 0x7AA5 }, 
            { 0x26BF, 0x79FF }, 
            { 0x28F8, 0x7944 }, 
            { 0x2B50, 0x7873 }, 
            { 0x2DC6, 0x7789 }, 
            { 0x305D, 0x7683 }, 
            { 0x3315, 0x755E }, 
            { 0x35EE, 0x7416 }, 
            { 0x38EA, 0x72A7 }, 
            { 0x3C08, 0x710D }, 
            { 0x3F4A, 0x6F42 }, 
            { 0x42AE, 0x6D43 }, 
            { 0x4634, 0x6B08 }, 
            { 0x49DB, 0x688B }, 
            { 0x4DA1, 0x65C6 }, 
            { 0x5183, 0x62B0 }, 
            { 0x557F, 0x5F43 }, 
            { 0x598E, 0x5B74 }, 
            { 0x5DAC, 0x573B }, 
            { 0x61D2, 0x528E }, 
            { 0x65F5, 0x4D64 }, 
            { 0x6A0B, 0x47B0 }, 
            { 0x6E06, 0x416A }, 
            { 0x71D6, 0x3A87 }, 
            { 0x7568, 0x32FE }, 
            { 0x78A5, 0x2AC5 }, 
            { 0x7B72, 0x21D7 }, 
            { 0x7DB2, 0x182E }, 
            { 0x7F41, 0xDCB }, 
            { 0x7FF9, 0x2AF }
        };

        uint16_t beepTable44100[125][2] = {
            { 0x26, 0x8000 }, 
            { 0x28, 0x8000 }, 
            { 0x2B, 0x8000 }, 
            { 0x2D, 0x8000 }, 
            { 0x30, 0x8000 }, 
            { 0x33, 0x8000 }, 
            { 0x36, 0x8000 }, 
            { 0x39, 0x8000 }, 
            { 0x3D, 0x8000 }, 
            { 0x40, 0x8000 }, 
            { 0x44, 0x8000 }, 
            { 0x48, 0x8000 }, 
            { 0x4C, 0x8000 }, 
            { 0x51, 0x8000 }, 
            { 0x56, 0x8000 }, 
            { 0x5B, 0x8000 }, 
            { 0x60, 0x8000 }, 
            { 0x66, 0x8000 }, 
            { 0x6C, 0x8000 }, 
            { 0x72, 0x8000 }, 
            { 0x79, 0x8000 }, 
            { 0x80, 0x8000 }, 
            { 0x88, 0x8000 }, 
            { 0x90, 0x8000 }, 
            { 0x99, 0x8000 }, 
            { 0xA2, 0x8000 }, 
            { 0xAB, 0x8000 }, 
            { 0xB6, 0x7FFF }, 
            { 0xC0, 0x7FFF }, 
            { 0xCC, 0x7FFF }, 
            { 0xD8, 0x7FFF }, 
            { 0xE5, 0x7FFF }, 
            { 0xF2, 0x7FFF }, 
            { 0x101, 0x7FFF }, 
            { 0x110, 0x7FFF }, 
            { 0x120, 0x7FFF }, 
            { 0x131, 0x7FFF }, 
            { 0x144, 0x7FFE }, 
            { 0x157, 0x7FFE }, 
            { 0x16B, 0x7FFE }, 
            { 0x181, 0x7FFE }, 
            { 0x198, 0x7FFD }, 
            { 0x1B0, 0x7FFD }, 
            { 0x1CA, 0x7FFD }, 
            { 0x1E5, 0x7FFC }, 
            { 0x202, 0x7FFC }, 
            { 0x220, 0x7FFB }, 
            { 0x240, 0x7FFB }, 
            { 0x263, 0x7FFA }, 
            { 0x287, 0x7FFA }, 
            { 0x2AD, 0x7FF9 }, 
            { 0x2D6, 0x7FF8 }, 
            { 0x301, 0x7FF7 }, 
            { 0x32F, 0x7FF6 }, 
            { 0x360, 0x7FF5 }, 
            { 0x393, 0x7FF3 }, 
            { 0x3C9, 0x7FF2 }, 
            { 0x403, 0x7FF0 }, 
            { 0x440, 0x7FEE }, 
            { 0x481, 0x7FEC }, 
            { 0x4C5, 0x7FE9 }, 
            { 0x50E, 0x7FE6 }, 
            { 0x55B, 0x7FE3 }, 
            { 0x5AC, 0x7FE0 }, 
            { 0x602, 0x7FDC }, 
            { 0x65E, 0x7FD7 }, 
            { 0x6BF, 0x7FD2 }, 
            { 0x725, 0x7FCD }, 
            { 0x792, 0x7FC7 }, 
            { 0x805, 0x7FC0 }, 
            { 0x87F, 0x7FB8 }, 
            { 0x900, 0x7FAF }, 
            { 0x989, 0x7FA5 }, 
            { 0xA19, 0x7F9A }, 
            { 0xAB3, 0x7F8D }, 
            { 0xB55, 0x7F7F }, 
            { 0xC01, 0x7F70 }, 
            { 0xCB7, 0x7F5E }, 
            { 0xD78, 0x7F4A }, 
            { 0xE45, 0x7F34 }, 
            { 0xF1D, 0x7F1B }, 
            { 0x1002, 0x7EFF }, 
            { 0x10F4, 0x7EDF }, 
            { 0x11F4, 0x7EBC }, 
            { 0x1304, 0x7E94 }, 
            { 0x1423, 0x7E68 }, 
            { 0x1553, 0x7E36 }, 
            { 0x1694, 0x7DFE }, 
            { 0x17E8, 0x7DC0 }, 
            { 0x194F, 0x7D79 }, 
            { 0x1ACA, 0x7D2A }, 
            { 0x1C5C, 0x7CD2 }, 
            { 0x1E03, 0x7C6E }, 
            { 0x1FC3, 0x7BFF }, 
            { 0x219B, 0x7B82 }, 
            { 0x238E, 0x7AF7 }, 
            { 0x259B, 0x7A5A }, 
            { 0x27C5, 0x79AA }, 
            { 0x2A0C, 0x78E5 }, 
            { 0x2C72, 0x7809 }, 
            { 0x2EF8, 0x7712 }, 
            { 0x319E, 0x75FE }, 
            { 0x3465, 0x74C9 }, 
            { 0x374E, 0x736F }, 
            { 0x3A5A, 0x71ED }, 
            { 0x3D89, 0x703D }, 
            { 0x40DB, 0x6E5A }, 
            { 0x444F, 0x6C40 }, 
            { 0x47E5, 0x69E7 }, 
            { 0x4B9B, 0x6749 }, 
            { 0x4F6E, 0x6460 }, 
            { 0x535D, 0x6122 }, 
            { 0x5762, 0x5D88 }, 
            { 0x5B79, 0x5989 }, 
            { 0x5F9C, 0x551B }, 
            { 0x63C1, 0x5035 }, 
            { 0x67E0, 0x4ACC }, 
            { 0x6BEB, 0x44D5 }, 
            { 0x6FD4, 0x3E47 }, 
            { 0x738A, 0x3717 }, 
            { 0x76F7, 0x2F3D }, 
            { 0x7A03, 0x26B0 }, 
            { 0x7C93, 0x1D6B }, 
            { 0x7E85, 0x136B }, 
            { 0x7FB4, 0x8B1 }
        };

        uint16_t beepTable32000[120][2] = {
            { 0x35, 0x8000 },
            { 0x38, 0x8000 },
            { 0x3B, 0x8000 },
            { 0x3F, 0x8000 },
            { 0x42, 0x8000 },
            { 0x46, 0x8000 },
            { 0x4A, 0x8000 },
            { 0x4F, 0x8000 },
            { 0x54, 0x8000 },
            { 0x58, 0x8000 },
            { 0x5E, 0x8000 },
            { 0x63, 0x8000 },
            { 0x69, 0x8000 },
            { 0x6F, 0x8000 },
            { 0x76, 0x8000 },
            { 0x7D, 0x8000 },
            { 0x85, 0x8000 },
            { 0x8C, 0x8000 },
            { 0x95, 0x8000 },
            { 0x9E, 0x8000 },
            { 0xA7, 0x8000 },
            { 0xB1, 0x8000 },
            { 0xBB, 0x7FFF },
            { 0xC7, 0x7FFF },
            { 0xD2, 0x7FFF },
            { 0xDF, 0x7FFF },
            { 0xEC, 0x7FFF },
            { 0xFA, 0x7FFF },
            { 0x109, 0x7FFF },
            { 0x119, 0x7FFF },
            { 0x12A, 0x7FFF },
            { 0x13B, 0x7FFE },
            { 0x14E, 0x7FFE },
            { 0x162, 0x7FFE },
            { 0x177, 0x7FFE },
            { 0x18D, 0x7FFE },
            { 0x1A5, 0x7FFD },
            { 0x1BE, 0x7FFD },
            { 0x1D8, 0x7FFD },
            { 0x1F4, 0x7FFC },
            { 0x212, 0x7FFC },
            { 0x232, 0x7FFB },
            { 0x253, 0x7FFB },
            { 0x276, 0x7FFA },
            { 0x29C, 0x7FF9 },
            { 0x2C4, 0x7FF8 },
            { 0x2EE, 0x7FF7 },
            { 0x31A, 0x7FF6 },
            { 0x34A, 0x7FF5 },
            { 0x37C, 0x7FF4 },
            { 0x3B1, 0x7FF2 },
            { 0x3E9, 0x7FF1 },
            { 0x424, 0x7FEF },
            { 0x463, 0x7FED },
            { 0x4A6, 0x7FEA },
            { 0x4ED, 0x7FE8 },
            { 0x538, 0x7FE5 },
            { 0x587, 0x7FE1 },
            { 0x5DB, 0x7FDE },
            { 0x634, 0x7FD9 },
            { 0x693, 0x7FD5 },
            { 0x6F7, 0x7FCF },
            { 0x760, 0x7FCA },
            { 0x7D1, 0x7FC3 },
            { 0x847, 0x7FBB },
            { 0x8C5, 0x7FB3 },
            { 0x94A, 0x7FAA },
            { 0x9D8, 0x7F9F },
            { 0xA6D, 0x7F93 },
            { 0xB0B, 0x7F86 },
            { 0xBB3, 0x7F77 },
            { 0xC65, 0x7F66 },
            { 0xD21, 0x7F53 },
            { 0xDE8, 0x7F3E },
            { 0xEBA, 0x7F26 },
            { 0xF9A, 0x7F0C },
            { 0x1086, 0x7EEE },
            { 0x1180, 0x7ECC },
            { 0x1288, 0x7EA7 },
            { 0x13A0, 0x7E7D },
            { 0x14C8, 0x7E4D },
            { 0x1602, 0x7E18 },
            { 0x174D, 0x7DDC },
            { 0x18AC, 0x7D9A },
            { 0x1A1E, 0x7D4F },
            { 0x1BA5, 0x7CFB },
            { 0x1D43, 0x7C9C },
            { 0x1EF8, 0x7C33 },
            { 0x20C5, 0x7BBC },
            { 0x22AB, 0x7B37 },
            { 0x24AD, 0x7AA2 },
            { 0x26CA, 0x79FB },
            { 0x2904, 0x7940 },
            { 0x2B5C, 0x786F },
            { 0x2DD3, 0x7784 },
            { 0x306A, 0x767E },
            { 0x3322, 0x7558 },
            { 0x35FD, 0x740F },
            { 0x38F9, 0x729F },
            { 0x3C18, 0x7104 },
            { 0x3F5B, 0x6F39 },
            { 0x42BF, 0x6D38 },
            { 0x4646, 0x6AFC },
            { 0x49EE, 0x687E },
            { 0x4DB4, 0x65B7 },
            { 0x5197, 0x62A0 },
            { 0x5593, 0x5F31 },
            { 0x59A3, 0x5B60 },
            { 0x5DC1, 0x5725 },
            { 0x61E6, 0x5276 },
            { 0x6609, 0x4D48 },
            { 0x6A1F, 0x4792 },
            { 0x6E19, 0x4149 },
            { 0x71E8, 0x3A63 },
            { 0x7579, 0x32D6 },
            { 0x78B4, 0x2A9A },
            { 0x7B7F, 0x21A8 },
            { 0x7DBC, 0x17FC },
            { 0x7F47, 0xD95 },
            { 0x7FFA, 0x276 }
        };

        uint16_t beepTable22050[113][2] = {
            { 0x4C, 0x8000 },
            { 0x51, 0x8000 },
            { 0x56, 0x8000 },
            { 0x5B, 0x8000 },
            { 0x60, 0x8000 },
            { 0x66, 0x8000 },
            { 0x6C, 0x8000 },
            { 0x72, 0x8000 },
            { 0x79, 0x8000 },
            { 0x80, 0x8000 },
            { 0x88, 0x8000 },
            { 0x90, 0x8000 },
            { 0x99, 0x8000 },
            { 0xA2, 0x8000 },
            { 0xAB, 0x8000 },
            { 0xB6, 0x7FFF },
            { 0xC0, 0x7FFF },
            { 0xCC, 0x7FFF },
            { 0xD8, 0x7FFF },
            { 0xE5, 0x7FFF },
            { 0xF2, 0x7FFF },
            { 0x101, 0x7FFF },
            { 0x110, 0x7FFF },
            { 0x120, 0x7FFF },
            { 0x131, 0x7FFF },
            { 0x144, 0x7FFE },
            { 0x157, 0x7FFE },
            { 0x16B, 0x7FFE },
            { 0x181, 0x7FFE },
            { 0x198, 0x7FFD },
            { 0x1B0, 0x7FFD },
            { 0x1CA, 0x7FFD },
            { 0x1E5, 0x7FFC },
            { 0x202, 0x7FFC },
            { 0x220, 0x7FFB },
            { 0x240, 0x7FFB },
            { 0x263, 0x7FFA },
            { 0x287, 0x7FFA },
            { 0x2AD, 0x7FF9 },
            { 0x2D6, 0x7FF8 },
            { 0x301, 0x7FF7 },
            { 0x32F, 0x7FF6 },
            { 0x360, 0x7FF5 },
            { 0x393, 0x7FF3 },
            { 0x3C9, 0x7FF2 },
            { 0x403, 0x7FF0 },
            { 0x440, 0x7FEE },
            { 0x481, 0x7FEC },
            { 0x4C5, 0x7FE9 },
            { 0x50E, 0x7FE6 },
            { 0x55B, 0x7FE3 },
            { 0x5AC, 0x7FE0 },
            { 0x602, 0x7FDC },
            { 0x65E, 0x7FD7 },
            { 0x6BF, 0x7FD2 },
            { 0x725, 0x7FCD },
            { 0x792, 0x7FC7 },
            { 0x805, 0x7FC0 },
            { 0x87F, 0x7FB8 },
            { 0x900, 0x7FAF },
            { 0x989, 0x7FA5 },
            { 0xA19, 0x7F9A },
            { 0xAB3, 0x7F8D },
            { 0xB55, 0x7F7F },
            { 0xC01, 0x7F70 },
            { 0xCB7, 0x7F5E },
            { 0xD78, 0x7F4A },
            { 0xE45, 0x7F34 },
            { 0xF1D, 0x7F1B },
            { 0x1002, 0x7EFF },
            { 0x10F4, 0x7EDF },
            { 0x11F4, 0x7EBC },
            { 0x1304, 0x7E94 },
            { 0x1423, 0x7E68 },
            { 0x1553, 0x7E36 },
            { 0x1694, 0x7DFE },
            { 0x17E8, 0x7DC0 },
            { 0x194F, 0x7D79 },
            { 0x1ACA, 0x7D2A },
            { 0x1C5C, 0x7CD2 },
            { 0x1E03, 0x7C6E },
            { 0x1FC3, 0x7BFF },
            { 0x219B, 0x7B82 },
            { 0x238E, 0x7AF7 },
            { 0x259B, 0x7A5A },
            { 0x27C5, 0x79AA },
            { 0x2A0C, 0x78E5 },
            { 0x2C72, 0x7809 },
            { 0x2EF8, 0x7712 },
            { 0x319E, 0x75FE },
            { 0x3465, 0x74C9 },
            { 0x374E, 0x736F },
            { 0x3A5A, 0x71ED },
            { 0x3D89, 0x703D },
            { 0x40DB, 0x6E5A },
            { 0x444F, 0x6C40 },
            { 0x47E5, 0x69E7 },
            { 0x4B9B, 0x6749 },
            { 0x4F6E, 0x6460 },
            { 0x535D, 0x6122 },
            { 0x5762, 0x5D88 },
            { 0x5B79, 0x5989 },
            { 0x5F9C, 0x551B },
            { 0x63C1, 0x5035 },
            { 0x67E0, 0x4ACC },
            { 0x6BEB, 0x44D5 },
            { 0x6FD4, 0x3E47 },
            { 0x738A, 0x3717 },
            { 0x76F7, 0x2F3D },
            { 0x7A03, 0x26B0 },
            { 0x7C93, 0x1D6B },
            { 0x7E85, 0x136B },
            { 0x7FB4, 0x8B1 }
        };
        
        uint16_t beepTable16000[108][2] = {
            { 0x69, 0x8000 },
            { 0x6F, 0x8000 },
            { 0x76, 0x8000 },
            { 0x7D, 0x8000 },
            { 0x85, 0x8000 },
            { 0x8C, 0x8000 },
            { 0x95, 0x8000 },
            { 0x9E, 0x8000 },
            { 0xA7, 0x8000 },
            { 0xB1, 0x8000 },
            { 0xBB, 0x7FFF },
            { 0xC7, 0x7FFF },
            { 0xD2, 0x7FFF },
            { 0xDF, 0x7FFF },
            { 0xEC, 0x7FFF },
            { 0xFA, 0x7FFF },
            { 0x109, 0x7FFF },
            { 0x119, 0x7FFF },
            { 0x12A, 0x7FFF },
            { 0x13B, 0x7FFE },
            { 0x14E, 0x7FFE },
            { 0x162, 0x7FFE },
            { 0x177, 0x7FFE },
            { 0x18D, 0x7FFE },
            { 0x1A5, 0x7FFD },
            { 0x1BE, 0x7FFD },
            { 0x1D8, 0x7FFD },
            { 0x1F4, 0x7FFC },
            { 0x212, 0x7FFC },
            { 0x232, 0x7FFB },
            { 0x253, 0x7FFB },
            { 0x276, 0x7FFA },
            { 0x29C, 0x7FF9 },
            { 0x2C4, 0x7FF8 },
            { 0x2EE, 0x7FF7 },
            { 0x31A, 0x7FF6 },
            { 0x34A, 0x7FF5 },
            { 0x37C, 0x7FF4 },
            { 0x3B1, 0x7FF2 },
            { 0x3E9, 0x7FF1 },
            { 0x424, 0x7FEF },
            { 0x463, 0x7FED },
            { 0x4A6, 0x7FEA },
            { 0x4ED, 0x7FE8 },
            { 0x538, 0x7FE5 },
            { 0x587, 0x7FE1 },
            { 0x5DB, 0x7FDE },
            { 0x634, 0x7FD9 },
            { 0x693, 0x7FD5 },
            { 0x6F7, 0x7FCF },
            { 0x760, 0x7FCA },
            { 0x7D1, 0x7FC3 },
            { 0x847, 0x7FBB },
            { 0x8C5, 0x7FB3 },
            { 0x94A, 0x7FAA },
            { 0x9D8, 0x7F9F },
            { 0xA6D, 0x7F93 },
            { 0xB0B, 0x7F86 },
            { 0xBB3, 0x7F77 },
            { 0xC65, 0x7F66 },
            { 0xD21, 0x7F53 },
            { 0xDE8, 0x7F3E },
            { 0xEBA, 0x7F26 },
            { 0xF9A, 0x7F0C },
            { 0x1086, 0x7EEE },
            { 0x1180, 0x7ECC },
            { 0x1288, 0x7EA7 },
            { 0x13A0, 0x7E7D },
            { 0x14C8, 0x7E4D },
            { 0x1602, 0x7E18 },
            { 0x174D, 0x7DDC },
            { 0x18AC, 0x7D9A },
            { 0x1A1E, 0x7D4F },
            { 0x1BA5, 0x7CFB },
            { 0x1D43, 0x7C9C },
            { 0x1EF8, 0x7C33 },
            { 0x20C5, 0x7BBC },
            { 0x22AB, 0x7B37 },
            { 0x24AD, 0x7AA2 },
            { 0x26CA, 0x79FB },
            { 0x2904, 0x7940 },
            { 0x2B5C, 0x786F },
            { 0x2DD3, 0x7784 },
            { 0x306A, 0x767E },
            { 0x3322, 0x7558 },
            { 0x35FD, 0x740F },
            { 0x38F9, 0x729F },
            { 0x3C18, 0x7104 },
            { 0x3F5B, 0x6F39 },
            { 0x42BF, 0x6D38 },
            { 0x4646, 0x6AFC },
            { 0x49EE, 0x687E },
            { 0x4DB4, 0x65B7 },
            { 0x5197, 0x62A0 },
            { 0x5593, 0x5F31 },
            { 0x59A3, 0x5B60 },
            { 0x5DC1, 0x5725 },
            { 0x61E6, 0x5276 },
            { 0x6609, 0x4D48 },
            { 0x6A1F, 0x4792 },
            { 0x6E19, 0x4149 },
            { 0x71E8, 0x3A63 },
            { 0x7579, 0x32D6 },
            { 0x78B4, 0x2A9A },
            { 0x7B7F, 0x21A8 },
            { 0x7DBC, 0x17FC },
            { 0x7F47, 0xD95 },
            { 0x7FFA, 0x276 }
        };
};

#endif