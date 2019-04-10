/*
    PADRING -- a padring generator for ASICs.

    Copyright (c) 2019, Niels Moseley <niels@symbioticeda.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
    
*/

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "logging.h"

static uint32_t gs_loglevel = LOG_INFO;

void setLogLevel(uint32_t level)
{
    gs_loglevel = level;
}

void doLog(uint32_t t, const std::string &txt)
{
    doLog(t, txt.c_str());
}

void doLog(uint32_t t, const char *format, ...)
{   
    if (t < gs_loglevel)
    {
        return;
    }

    FILE *sout = stdout;

    switch(t)
    {
    case LOG_INFO:
        fprintf(sout, "[INFO] ");
        break;
    case LOG_DEBUG:
        fprintf(sout, "[DBG ] ");
        break;
    case LOG_WARN:
        fprintf(sout, "[WARN] ");
        break;
    case LOG_ERROR:
        sout = stderr;
        fprintf(sout, "[ERR ] ");
        break;
    case LOG_VERBOSE:
        fprintf(stdout, "[VERB] ");
        break;
    default:
        break;
    }

    //FIXME: change to C++ style
    va_list argptr;
    va_start(argptr, format);
    vfprintf(sout, format, argptr);
    fflush(sout);
    va_end(argptr);
}
