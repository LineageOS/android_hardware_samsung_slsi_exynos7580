/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __DRAPILOGGING_H__
#define __DRAPILOGGING_H__

#include "DrApi/DrApiCommon.h"
#include "DrApi/DrApiError.h"

/** Formatted logging functions.
 *
 * drApiLogvPrintf, drApiLogPrintf
 *
 * Minimal printf-like function to print logging message to NWd log.
 *
 * Supported formatters:
 *   %s String, NULL value emit "<NULL>".
 *   %x %X hex
 *   %p pointer (hex with fixed width of 8)
 *   %d %i signed decimal
 *   %u unsigned decimal
 *   %t timestamp (if available in platform). NOTE: This does not consume any value in parameter list.
 *   %% outputs single %
 *
 *   %s, %x, %d, and %u support width (example %5s). Width is interpreted as minimum number of characters.
 *   Hex number is left padded using '0' to desired width.
 *   Decimal number is left padded using ' ' to desired width.
 *   String is right padded to desired length.
 *
 *   Newline is used to terminate logging line.
 *
 * @param fmt     Formatter
 * @param args    Argument list.
 */
_DRAPI_EXTERN_C void drApiLogvPrintf(
        const char *fmt,
        va_list args);

_DRAPI_EXTERN_C void drApiLogPrintf(
        const char *fmt,
        ...);

#if defined(__INCLUDE_DEBUG)

    #define drDbgPrintf(...)   drApiLogPrintf(__VA_ARGS__)
    #define drDbgvPrintf(...)  drApiLogvPrintf(__VA_ARGS__)

#else

   #define drDbgPrintf(...)    DUMMY_FUNCTION()
   #define drDbgvPrintf(...)   DUMMY_FUNCTION()

#endif // __INCLUDE_DEBUG

#ifndef EOL
    #define EOL "\n"
#endif

#define drDbgPrintLnf(...)      do{drDbgPrintf(__VA_ARGS__);dbgS(EOL);}while(FALSE)


#endif // __DRAPILOGGING_H__

