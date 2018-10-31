#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifdef _WIN32

#define CLRED ""
#define CLGRN ""
#define CLYEL ""
#define CLBLU ""
#define CLCYA ""
#define CLRST ""

#define FMTBLD ""
#define FMTRST ""
#else

#define CLRED "\e[31m"
#define CLGRN "\e[32m"
#define CLYEL "\e[33m"
#define CLBLU "\e[34m"
#define CLCYA "\e[36m"
#define CLRST "\e[0m"

#define FMTBLD "\033[1m"
#define FMTRST "\033[0m"
#endif

void LOG_DUMP(FILE *stream, const char *label, uint8_t *b, int l);

void LOG_INFO(FILE *stream, const char *fmt, ...);
void LOG_WARN(FILE *stream, const char *fmt, ...);
void LOG_ERROR(FILE *stream, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
