#include "EnhancedThread.h"
#include "Logging.h"

void EnhancedThread::run() {
  unsigned long before_run = millis();
  unsigned long before_last_run = last_run;
  Thread::run();
  if (_runOnce)
    Thread::enabled = false;
  unsigned long after_run = millis();

  #ifdef FEATURE_FLAG_THREADSTATS
  unsigned long runtime = after_run - before_run;
  if (runtime < loopStats.min)
    loopStats.min = runtime;
  if (runtime > loopStats.max)
    loopStats.max = runtime;
  memmove(&loopStats.samples[1], &loopStats.samples[0], (STAT_SAMPLES-1)*2);
  loopStats.samples[0] = runtime;

  if (!_firstIntervalSample) {
    unsigned long calcInterval = before_run - before_last_run;
    if (calcInterval < loopStats.minInterval)
      loopStats.minInterval = calcInterval;
    if (calcInterval > loopStats.maxInterval)
      loopStats.maxInterval = calcInterval;
    memmove(&loopStats.samplesInterval[1], &loopStats.samplesInterval[0], (STAT_SAMPLES_INTERVAL-1)*2);
    loopStats.samplesInterval[0] = calcInterval;
  } else {
    _firstIntervalSample = false;
  }
  #endif
}

void EnhancedThread::runIfNeeded(void) {
  if(Thread::shouldRun())
    Thread::run();
}

void EnhancedThread::resetStats() {
  #ifdef FEATURE_FLAG_THREADSTATS
  loopStats.min = UINT16_MAX;
  loopStats.max = 0;
  for (uint8_t i=0; i<STAT_SAMPLES; i++)
    loopStats.samples[i] = 0;

  loopStats.minInterval = UINT16_MAX;
  loopStats.maxInterval = 0;
  for (uint8_t i=0; i<STAT_SAMPLES_INTERVAL; i++)
    loopStats.samplesInterval[i] = 0;

  memStats.minHeapFree = UINT16_MAX;
  memStats.minStackFree = UINT16_MAX;
  memStats.maxHeapFree = 0;
  memStats.maxStackFree = 0;
  for (uint8_t i=0; i<STAT_SAMPLES_MEM; i++) {
    memStats.samplesHeapFree[i][0] = UINT16_MAX;
    memStats.samplesHeapFree[i][1] = 0;
    memStats.samplesStackFree[i][0] = UINT16_MAX;
    memStats.samplesStackFree[i][1] = 0;
  }
  #endif
}
void EnhancedThread::logStats() {
  #ifdef USE_THREAD_NAMES
  Log.info("Thread statistics for %s, priority=%i, interval=%i", ThreadName, priority, interval);
  #else
  Log.info("Thread statistics for %i, priority=%i, interval=%i", ThreadID, priority, interval);
  #endif
  #ifdef FEATURE_FLAG_THREADSTATS
  Log.info(" #Measured runtime#");
  Log.info("  Min. %ims / Max. %ims", loopStats.min, loopStats.max);
  Log.disableNewline(true);
  Log.info("  Samples: ");
  for (uint8_t i=0; i<STAT_SAMPLES; i++) {
    Log.printf(" [%i] %ims,", i, loopStats.samples[i]);
  }
  Log.disableNewline(false);
  Log.println();
  Log.info(" #Measured interval#");
  Log.info("  Min. %ims / Max. %ims", loopStats.minInterval, loopStats.maxInterval);
  Log.disableNewline(true);
  Log.info("  Samples:");
  for (uint8_t i=0; i<STAT_SAMPLES_INTERVAL; i++) {
    Log.printf(" [%i] %ims,", i, loopStats.samplesInterval[i]);
  }
  Log.disableNewline(false);
  Log.println();
  Log.info("");
  Log.info(" #Memory Heap free#");
  Log.info("  Min. %ib / Max. %ib", memStats.minHeapFree, memStats.maxHeapFree);
  Log.disableNewline(true);
  Log.info("  Samples:");
  for (uint8_t i=0; i<STAT_SAMPLES_MEM; i++) {
    uint16_t heapFreeMin = memStats.samplesHeapFree[i][0];
    uint16_t heapFreeMax = memStats.samplesHeapFree[i][1];
    if (heapFreeMax)
      Log.printf(" [%i] %ib/%ib,", i, heapFreeMin, heapFreeMax);
  }
  Log.disableNewline(false);
  Log.println();
  Log.info(" #Memory Stack free#");
  Log.info("  Min. %ib / Max. %ib", memStats.minStackFree, memStats.maxStackFree);
  Log.disableNewline(true);
  Log.info("  Samples:");
  for (uint8_t i=0; i<STAT_SAMPLES_MEM; i++) {
    uint16_t stackFreeMin = memStats.samplesStackFree[i][0];
    uint16_t stackFreeMax = memStats.samplesStackFree[i][1];
    if (stackFreeMax)
      Log.printf(" [%i] %ib/%ib,", i, stackFreeMin, stackFreeMax);
  }
  Log.disableNewline(false);
  Log.println();
  Log.print("------------------------------------------------------");
  #else
  Log.error("Thread stats not active (FEATURE_FLAG_THREADSTATS not set)");
  #endif
}

void EnhancedThread::sampleMemory(uint8_t id) {
  #ifdef FEATURE_FLAG_THREADSTATS
  uint16_t freeHeap = (uint16_t)freeHeapMemory();
  if (freeHeap<memStats.minHeapFree)
    memStats.minHeapFree = freeHeap;
  if (freeHeap>memStats.maxHeapFree)
    memStats.maxHeapFree = freeHeap;  

  uint16_t freeStack = (uint16_t)freeStackMemory();
  if (freeStack<memStats.minStackFree)
    memStats.minStackFree = freeStack;
  if (freeStack>memStats.maxStackFree)
    memStats.maxStackFree = freeStack;

  if (id < STAT_SAMPLES_MEM) {
    if (freeHeap<memStats.samplesHeapFree[id][0])
      memStats.samplesHeapFree[id][0] = freeHeap;
    if (freeHeap>memStats.samplesHeapFree[id][1])
      memStats.samplesHeapFree[id][1] = freeHeap;

    if (freeStack<memStats.samplesStackFree[id][0])
      memStats.samplesStackFree[id][0] = freeStack;
    if (freeStack>memStats.samplesStackFree[id][1])
      memStats.samplesStackFree[id][1] = freeStack;
  } else {
    Log.error("Sampling memory with invalid id %i not possible.", id);
  }
  //Log.info("Sample memory id=%i, heap=%i, stack=%i", id, freeHeap, freeStack);
  #endif
}

#ifdef USE_THREAD_NAMES
void EnhancedThread::setName(const char* name) {
  sprintf(ThreadName, "%s (0x%X)", name, ThreadID);
}
#endif

void EnhancedThread::reset(void) {
  Thread::enabled = true;
  Thread::runned();
}

void EnhancedThread::setRunOnce(bool runOnce) {
  _runOnce = runOnce;
}
unsigned long EnhancedThread::getInterval(void) {
  return interval;
}
