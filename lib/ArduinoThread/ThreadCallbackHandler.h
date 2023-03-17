#ifndef ThreadCallbackHandler_h
#define ThreadCallbackHandler_h

#include <functional>

class ThreadCallbackHandler {
    public:
        ThreadCallbackHandler(){}
        ThreadCallbackHandler(std::function<void()> callback) { _callbackMember = callback; }
        ThreadCallbackHandler(void (*callback)(void)) { _callbackFunction = callback; }

        void callback() {
            if(_callbackMember != NULL)
                _callbackMember();

            if(_callbackFunction != NULL)
                _callbackFunction();
        }

    private:
        std::function<void()> _callbackMember;
	    void (*_callbackFunction)(void);		
};

#endif