#include "Thread.h"
#include "ThreadController.h"

#include <Logging.h>

ThreadController::ThreadController(unsigned long _interval): Thread(){
	cached_size = 0;

	clear();
	setInterval(_interval);

	#ifdef USE_THREAD_NAMES
		sprintf((char*)ThreadName, "ThreadController");
	#endif
}

/*
	ThreadController run() (cool stuf)
*/
void ThreadController::run(){
	// Run this thread before
	_callbackHandler.callback();

	unsigned long time = millis();
	int checks = 0;
	for(int i = 0; i < MAX_THREADS && checks < cached_size; i++){
		// Object exists? Is enabled? Timeout exceeded?
		if(thread[i]){
			checks++;
			if(thread[i]->shouldRun(time)){
				thread[i]->run();
			}
		}
	}

	// ThreadController extends Thread, so we should flag as runned thread
	runned();
}


/*
	List controller (boring part)
*/
bool ThreadController::add(Thread* _thread){
	// Check if the Thread already exists on the array
	for(int i = 0; i < MAX_THREADS; i++){
		if(thread[i] != NULL && thread[i]->ThreadID == _thread->ThreadID) {
			#ifdef USE_THREAD_NAMES
			Log.debug("Thread %s already added to controller.", _thread->ThreadName);
			#else
			Log.info("Thread already added to controller.");
			#endif
			//return true;
		}
	}

	// Find an empty slot
	for(int i = 0; i < MAX_THREADS; i++){
		if(!thread[i]){
			// Found a empty slot, now add Thread
			thread[i] = _thread;
			cached_size++;
			#ifdef USE_THREAD_NAMES
			Log.debug("Added thread %s to controller.", thread[i]->ThreadName);
			#else
			Log.debug("Added thread %i to controller.", i);
			#endif
			return true;
		}
	}
	#ifdef USE_THREAD_NAMES
	Log.debug("Thread %s not added to controller, increase MAX_THREADS (%i).", _thread->ThreadName, MAX_THREADS);
	#else
	Log.debug("Thread not added to controller, increase MAX_THREADS (%i).", MAX_THREADS);
	#endif
	// Array is full
	return false;
}

void ThreadController::remove(int id){
	// Find Threads with the id, and removes
	for(int i = 0; i < MAX_THREADS; i++){
		if(thread[i]->ThreadID == id){
			thread[i] = NULL;
			cached_size--;
			return;
		}
	}
}

void ThreadController::remove(Thread* _thread){
	remove(_thread->ThreadID);
}

void ThreadController::clear(){
	for(int i = 0; i < MAX_THREADS; i++){
		thread[i] = NULL;
	}
	cached_size = 0;
}

int ThreadController::size(bool cached){
	if(cached)
		return cached_size;

	int size = 0;
	for(int i = 0; i < MAX_THREADS; i++){
		if(thread[i])
			size++;
	}
	cached_size = size;

	return cached_size;
}

Thread* ThreadController::get(int index){
	int pos = -1;
	for(int i = 0; i < MAX_THREADS; i++){
		if(thread[i] != NULL){
			pos++;

			if(pos == index)
				return thread[i];
		}
	}

	return NULL;
}

int compare(const void* a, const void* b) {
	if (a == NULL && b == NULL)
		return 0;
	if (a == NULL && b != NULL)
		return -1;
	if (a != NULL && b == NULL)
		return 1;

	Thread* threadA = *(Thread**)a;
	Thread* threadB = *(Thread**)b;

	if (threadA->priority < threadB->priority)
		return -1;
	 else if (threadA->priority > threadB->priority)
		return 1;
	
	
	if (threadA->getInterval() < threadB->getInterval())
		return -1;
	 else if (threadA->getInterval() > threadB->getInterval())
		return 1;
	
	return 0;
}
void ThreadController::sortThreads() {
	qsort(thread, size(false), sizeof(*thread), compare);
}