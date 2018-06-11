#include "logging.h"
#include "usbcan_util.h"

#define LOG_STREAM stderr

void LOG_INFO(const char *fmt, ...)
{
    va_list ap;
    fprintf(LOG_STREAM, CLBLU FMTBLD "INFO:\t" FMTRST CLBLU);

    va_start(ap, fmt);
    vfprintf(LOG_STREAM, fmt, ap);
    va_end(ap);
    fprintf(LOG_STREAM, CLRST "\n");
}

void LOG_WARN(const char *fmt, ...)
{
    va_list ap;
    fprintf(LOG_STREAM, CLYEL FMTBLD "WARN:\t" FMTRST CLYEL);

    va_start(ap, fmt);
    vfprintf(LOG_STREAM, fmt, ap);
    va_end(ap);
    fprintf(LOG_STREAM, CLRST "\n");
}

void LOG_ERROR(const char *fmt, ...)
{
    va_list ap;
    fprintf(LOG_STREAM, CLRED FMTBLD "ERROR:\t" FMTRST CLRED);

    va_start(ap, fmt);
    vfprintf(LOG_STREAM, fmt, ap);
    va_end(ap);
    fprintf(LOG_STREAM, CLRST"\n");
}

void _LOG_ASSERT_(bool cond, bool persist, const char *x, const char *c, const char *y, float fx, float fy)
{
    if(!cond || persist)
    {
        fprintf(LOG_STREAM, CLCYA FMTBLD "ASSERT:\t" CLRST "%s[%f] %s %s[%f] -> %s%s",
                        x, fx, c, y, fy, cond ? CLGRN : CLRED, cond ? LOG_PASS : LOG_FAIL);

        fprintf(LOG_STREAM, CLRST "\n");
    }

    if(!cond)
    {
        exit(1);
    }
}

void dump(const char *label, uint8_t *b, int l)
{
    static int nz = 0;
    static struct timeval tv_z;
    static struct timeval tv_p;
    struct timeval tv;
    uint64_t usec, dusec;
    const char space[] = "                   ";

    if(!nz)
    {
        gettimeofday(&tv_z, NULL);
        tv_p = tv_z;
        nz++;
    }
    gettimeofday(&tv, NULL);

    usec = 1000000ull * (tv.tv_sec - tv_z.tv_sec) + (tv.tv_usec - tv_z.tv_usec);
    dusec = 1000000ull * (tv.tv_sec - tv_p.tv_sec) + (tv.tv_usec - tv_p.tv_usec);
    tv_p = tv;

    fprintf(LOG_STREAM, "%6" PRIu64 ".%03" PRIu64 ".%03" PRIu64 " %+4" PRId64 ".%03" PRIu64 ".%03" PRIu64 " %s%s[%3d]:",
            (uint64_t)(usec / 1000000ull), (uint64_t)((usec % 1000000ull) / 1000ull), (uint64_t)(usec % 1000ull),
            (uint64_t)(dusec / 1000000ull), (int64_t)((dusec % 1000000ull) / 1000ull), (uint64_t)(dusec % 1000ull),
            label, &space[CLIPH(strlen(label), sizeof(space) - 1)], l);

    for(; l--;)
    {
        fprintf(LOG_STREAM, "%.2X ", *b++);
    }
    fprintf(LOG_STREAM, "\n");
}

