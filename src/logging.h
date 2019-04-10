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

#ifndef logging_h
#define logging_h

#include <string>

typedef enum {LOG_VERBOSE = 1, LOG_DEBUG = 2, LOG_INFO = 3, LOG_WARN = 4, 
    LOG_ERROR = 8, LOG_QUIET = 255} logtype_t;

/** log something */
void doLog(uint32_t t, const char *format, ...);

/** log something */
void doLog(uint32_t t, const std::string &txt);

/** set the log level ... */
void setLogLevel(uint32_t level);

#endif
