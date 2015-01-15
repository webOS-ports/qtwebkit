#ifndef LOGGING_H_
#define LOGGING_H_

#include <PmLogLib.h>

extern PmLogContext LogContext;

#define DEBUG(...) \
	PmLogDebug(LogContext, ##__VA_ARGS__)

#endif
