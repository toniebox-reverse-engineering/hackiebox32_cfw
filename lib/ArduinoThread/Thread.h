/*
 	Thread.h - An runnable object

	Thread is responsable for holding the "action" for something,
	also, it responds if it "should" or "should not" run, based on
	the current time;

	For instructions, go to https://github.com/ivanseidel/ArduinoThread

	Created by Ivan Seidel Gomes, March, 2013.
	Released into the public domain.
*/

#ifndef Thread_h
#define Thread_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif

#include <inttypes.h>
#include <functional>
#include <ThreadCallbackHandler.h>

/*
	Uncomment this line to enable ThreadName Strings.

	It might be usefull if you are logging thread with Serial,
	or displaying a list of threads...
*/
#define USE_THREAD_NAMES	1

class Thread{
protected:
	// Desired interval between runs
	unsigned long interval;

	// Last runned time in Ms
	unsigned long last_run;

	// Scheduled run in Ms (MUST BE CACHED)	
	unsigned long _cached_next_run;

	void _prepare(unsigned long _interval = 0);

	/*
		IMPORTANT! Run after all calls to run()
		Updates last_run and cache next run.
		NOTE: This MUST be called if extending
		this class and implementing run() method
	*/
	void runned(unsigned long time);

	// Default is to mark it runned "now"
	void runned() { runned(millis()); }
	
	ThreadCallbackHandler _callbackHandler;

public:

	// If the current Thread is enabled or not
	bool enabled;

	// ID of the Thread (initialized from memory adr.)
	int ThreadID;

	uint8_t priority;

	#ifdef USE_THREAD_NAMES
		// Thread Name (used for better UI).
		char ThreadName[32];
	#endif

	Thread(unsigned long _interval = 0);
	Thread(ThreadCallbackHandler callbackHandler, unsigned long _interval = 0);

	// Set the desired interval for calls, and update _cached_next_run
	virtual void setInterval(unsigned long _interval);
	unsigned long getInterval();

	// Return if the Thread should be runned or not
	virtual bool shouldRun(unsigned long time);

	// Default is to check whether it should run "now"
	bool shouldRun() { return shouldRun(millis()); }

	// Callback set
	void onRun(ThreadCallbackHandler callbackHandler);

	// Runs Thread
	virtual void run();
};

#endif
