#include "Logging.h"

#ifdef LOG_DISABLED
void Logging::init(int level, long baud) {}
void Logging::init(int level, long baud, Stream* additionalLogger) {}
void Logging::setAdditionalLogger(Stream* additionalLogger) {}
void Logging::disableNewline(bool disabled) {}
void Logging::error(const char* msg, ...){}
void Logging::info(const char* msg, ...){}
void Logging::debug(const char* msg, ...){}
void Logging::verbose(const char* msg, ...){}
void Logging::println() {}
void Logging::println(const char *msg) {}
void Logging::printfln(const char *msg, ...) {}
void Logging::print(const char *msg) {}
void Logging::printf(const char* msg, ...){}
void Logging::printFormat(const char *format, va_list args) {}
void Logging::printFormat(Stream *stream, const char *format, va_list args) {}
void Logging::enableAdditionalLogger(bool enabled) {}
#else
void Logging::init(int level, long baud) {
    _level = constrain(level,LOG_LEVEL_NOOUTPUT,LOG_LEVEL_VERBOSE);
    _baud = baud;
    Serial.begin(_baud);
	
	HardwareSerial &serial = Serial;
	_stream = &static_cast<Stream&>(serial);
	_disableNewline = false;
	
	info("Start logging, additionalLogger=%i", _additionalLogger);
}
void Logging::init(int level, long baud, Stream* additionalLogger) {
	_enableAdditionalLogger = true;
	setAdditionalLogger(additionalLogger);
	init(level, baud);
}
void Logging::setAdditionalLogger(Stream* additionalLogger) {
	_additionalLogger = additionalLogger;
}
void Logging::disableNewline(bool disabled) {
	_disableNewline = disabled;
}

void Logging::error(const char* msg, ...){
    if (LOG_LEVEL_ERRORS <= _level) {
		print("E: ");
        va_list args;
        va_start(args, msg);
        printFormat(msg, args);
		println();
    }
}


void Logging::info(const char* msg, ...){
    if (LOG_LEVEL_INFOS <= _level) {
		print("I:  ");
        va_list args;
        va_start(args, msg);
        printFormat(msg,args);
		println();
    }
}

void Logging::debug(const char* msg, ...){
    if (LOG_LEVEL_DEBUG <= _level) {
		print("D:   ");
        va_list args;
        va_start(args, msg);
        printFormat(msg,args);
		println();
    }
}

void Logging::verbose(const char* msg, ...){
    if (LOG_LEVEL_VERBOSE <= _level) {
		print("V:    ");
        va_list args;
        va_start(args, msg);
        printFormat(msg,args);
		println();
    }
}
void Logging::println() {
	if (!_disableNewline)
		print("\r\n");
}
void Logging::println(const char *msg) {
	print(msg);
	println();
}
void Logging::printfln(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	printFormat(msg, args);
	println();
}
void Logging::print(const char *msg) {
	_stream->print(msg);
	if (_enableAdditionalLogger && _additionalLogger != 0)
		_additionalLogger->print(msg);
}
void Logging::printf(const char* msg, ...){
	va_list args;
	va_start(args, msg);
	printFormat(msg, args);
}
void Logging::printFormat(const char *format, va_list args) {
	printFormat(_stream, format, args);
	if (_enableAdditionalLogger && _additionalLogger != 0) {
		printFormat(_additionalLogger, format, args);
	}
}

void Logging::printFormat(Stream *stream, const char *format, va_list args) {
    // loop through format string
    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            if (*format == '\0') break;
            if (*format == '%') {
                stream->print(*format);
                continue;
            }
            if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				stream->print(s);
				continue;
			}
            if( *format == 'd' || *format == 'i') {
				stream->print(va_arg( args, int ),DEC);
				continue;
			}
            if( *format == 'f') {
				stream->print(va_arg( args, double ),DEC);
				continue;
			}
            if( *format == 'x' ) {
				stream->print(va_arg( args, int ),HEX);
				continue;
			}
            if( *format == 'X' ) {
				stream->print("0x");
				stream->print(va_arg( args, int ),HEX);
				continue;
			}
            if( *format == 'b' ) {
				stream->print(va_arg( args, int ),BIN);
				continue;
			}
            if( *format == 'B' ) {
				stream->print("0b");
				stream->print(va_arg( args, int ),BIN);
				continue;
			}
            if( *format == 'l' ) {
				stream->print(va_arg( args, long ),DEC);
				continue;
			}

            if( *format == 'c' ) {
				stream->print(va_arg( args, int ));
				continue;
			}
            if( *format == 't' ) {
				if (va_arg( args, int ) == 1) {
					stream->print("T");
				}
				else {
					stream->print("F");				
				}
				continue;
			}
            if( *format == 'T' ) {
				if (va_arg( args, int ) == 1) {
					stream->print("true");
				}
				else {
					stream->print("false");				
				}
				continue;
			}

        }
        stream->print(*format);
    }
 }


void Logging::enableAdditionalLogger(bool enabled) {
	_enableAdditionalLogger = enabled;
}
 
#endif 

Logging Log = Logging();


 
 
  




