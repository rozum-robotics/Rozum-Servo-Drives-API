#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define CLRED "\e[31m"
#define CLGRN "\e[32m"
#define CLYEL "\e[33m"
#define CLBLU "\e[34m"
#define CLCYA "\e[36m"
#define CLRST "\e[0m"

#define FMTBLD "\033[1m"
#define FMTRST "\033[0m"

#define LOG_PASS (FMTBLD "PASS" FMTRST)
#define LOG_FAIL (FMTBLD "FAIL" FMTRST)


void dump(const char *label, uint8_t *b, int l);

void LOG_INFO(const char *fmt, ...);
void LOG_WARN(const char *fmt, ...);
void LOG_ERROR(const char *fmt, ...);
void _LOG_ASSERT_(bool cond, bool persist, const char *x, const char *c, const char *y, float fx, float fy);

#define STRFY(a) #a
#define LOG_ASSERT(x, c, y, persist) _LOG_ASSERT_(x c y, persist, STRFY(x), STRFY(c), STRFY(y), x, y)

#endif
