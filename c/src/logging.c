#include "logging.h"
#include "usbcan_util.h"

static int begin_length = 0;

/**
 * @brief LOG_INFO
 * 
 * @param stream 
 * @param fmt 
 * @param ... 
 */
void LOG_INFO(FILE *stream, const char *fmt, ...)
{
    if(!stream)
    {
        return;
    }
    va_list ap;
    fprintf(stream, CLBLU FMTBLD "INFO:\t" FMTRST CLBLU);

    va_start(ap, fmt);
    vfprintf(stream, fmt, ap);
    va_end(ap);
    fprintf(stream, CLRST "\n");
}

/**
 * @brief LOG_WARN
 * 
 * @param stream 
 * @param fmt 
 * @param ... 
 */
void LOG_WARN(FILE *stream, const char *fmt, ...)
{
    if(!stream)
    {
        return;
    }
    va_list ap;
    fprintf(stream, CLYEL FMTBLD "WARN:\t" FMTRST CLYEL);

    va_start(ap, fmt);
    vfprintf(stream, fmt, ap);
    va_end(ap);
    fprintf(stream, CLRST "\n");
}

/**
 * @brief LOG_ERROR
 * 
 * @param stream 
 * @param fmt 
 * @param ... 
 */
void LOG_ERROR(FILE *stream, const char *fmt, ...)
{
    if(!stream)
    {
        return;
    }
    va_list ap;
    fprintf(stream, CLRED FMTBLD "ERROR:\t" FMTRST CLRED);

    va_start(ap, fmt);
    vfprintf(stream, fmt, ap);
    va_end(ap);
    fprintf(stream, CLRST "\n");
}

/**
 * @brief _LOG_ASSERT_
 * 
 * @param stream 
 * @param cond 
 * @param persist 
 * @param x 
 * @param c 
 * @param y 
 * @param fx 
 * @param fy 
 */
void _LOG_ASSERT_(FILE *stream, bool cond, bool persist, const char *x, const char *c, const char *y, float fx, float fy)
{
    if(!stream)
    {
        return;
    }
    if(!cond || persist)
    {
        fprintf(stream, CLCYA FMTBLD "ASSERT:\t" CLRST "%s[%f] %s %s[%f] -> %s%s",
                x, fx, c, y, fy, cond ? CLGRN : CLRED, cond ? LOG_PASS : LOG_FAIL);

        fprintf(stream, CLRST "\n");
    }

    if(!cond)
    {
        exit(1);
    }
}

/**
 * @brief LOG_BEGIN
 * 
 * @param fmt 
 * @param ... 
 */
void LOG_BEGIN(FILE *stream, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    fputs(FMTBLD CLYEL "TEST: " CLRST FMTBLD, stream);
    begin_length = vfprintf(stream, fmt, ap);

    va_end(ap);
    fprintf(stream, CLRST "\n");
    fflush(stream);
}

/**
 * @brief LOG_END
 * 
 */
void LOG_END()
{
    begin_length = 0;
}

/**
 * @brief LOG_DUMP
 * 
 * @param stream 
 * @param label 
 * @param b 
 * @param l 
 */
void LOG_DUMP(FILE *stream, const char *label, uint8_t *b, int l)
{
    if(!stream)
    {
        return;
    }
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

    fprintf(stream, "%6" PRIu64 ".%03" PRIu64 ".%03" PRIu64 " %+4" PRId64 ".%03" PRIu64 ".%03" PRIu64 " %s%s[%3d]:",
            (uint64_t)(usec / 1000000ull), (uint64_t)((usec % 1000000ull) / 1000ull), (uint64_t)(usec % 1000ull),
            (uint64_t)(dusec / 1000000ull), (int64_t)((dusec % 1000000ull) / 1000ull), (uint64_t)(dusec % 1000ull),
            label, &space[CLIPH(strlen(label), sizeof(space) - 1)], l);

    for(; l--;)
    {
        fprintf(stream, "%.2X ", *b++);
    }
    fprintf(stream, "\n");
}
