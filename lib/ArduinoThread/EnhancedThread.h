#ifndef EnhancedThread_h
#define EnhancedThread_h

#include <Thread.h>
#include <MemoryFree.h>

#define FEATURE_FLAG_THREADSTATS

class EnhancedThread : public Thread {
  public:
    EnhancedThread(unsigned long _interval = 0)
      : Thread(_interval) { resetStats(); };
    EnhancedThread(ThreadCallbackHandler callbackHandler, unsigned long _interval = 0)
      : Thread(callbackHandler, _interval) { resetStats(); };
    void
      run(void),
      runIfNeeded(void),
      reset(void),
      setRunOnce(bool);
    
    #ifdef USE_THREAD_NAMES
    void setName(const char* name);
    #endif

    unsigned long getInterval(void);

    void resetStats();
    void logStats();
    #ifdef FEATURE_FLAG_THREADSTATS
    const static uint8_t STAT_SAMPLES = 10;
    const static uint8_t STAT_SAMPLES_INTERVAL = 10;
    struct LOOP_STATS {
      uint16_t min;
      uint16_t max;
      uint16_t samples[STAT_SAMPLES];
      uint16_t minInterval; 
      uint16_t maxInterval;
      uint16_t samplesInterval[STAT_SAMPLES_INTERVAL];
    };
    LOOP_STATS loopStats;

    const static uint8_t STAT_SAMPLES_MEM = 10;
    struct MEMORY_STATS {
      uint16_t minHeapFree; 
      uint16_t maxHeapFree;
      uint16_t samplesHeapFree[STAT_SAMPLES][2];
      uint16_t minStackFree;
      uint16_t maxStackFree;
      uint16_t samplesStackFree[STAT_SAMPLES][2];
    };
    MEMORY_STATS memStats;
    #endif
    void sampleMemory(uint8_t id);

  private:
    bool _runOnce = false;

    bool _firstIntervalSample = true;
};

#endif
