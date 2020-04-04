/**
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * Definition of the machine-specific integer types
 **/
#ifndef __TEE_TYPE_H__
#define __TEE_TYPE_H__

/* C99 integer types */
#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L) &&(!defined(ANDROID))

#include <limits.h>

/* Figure out if a 64-bit integer types is available */
#if \
    defined(_MSC_VER) || \
    defined(__SYMBIAN32__) || \
    defined(_WIN32_WCE) || \
    (defined(ULLONG_MAX) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFFULL) || \
    (defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xFFFFFFFFFFFFFFFFULL)
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
#define __S_TYPE_INT64_UNDEFINED
#endif

#if UINT_MAX == 0xFFFFFFFF
typedef unsigned int uint32_t;
typedef int int32_t;
#elif ULONG_MAX == 0xFFFFFFFF
typedef unsigned long uint32_t;
typedef long int32_t;
#else
#error This compiler is not supported.
#endif

#if USHRT_MAX == 0xFFFF
typedef unsigned short uint16_t;
typedef short  int16_t;
#else
#error This compiler is not supported.
#endif

#if UCHAR_MAX == 0xFF
typedef unsigned char uint8_t;
typedef signed char   int8_t;
#else
#error This compiler is not supported.
#endif

#if !defined(__cplusplus)
typedef unsigned char bool;
#define false ( (bool)0 )
#define true  ( (bool)1 )
#endif

#else  /* !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L */

#include <stdbool.h>
#include <stdint.h>

#endif  /* !(!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L) */

#include <stddef.h>

#ifndef NULL
#  ifdef __cplusplus
#     define NULL  0
#  else
#     define NULL  ((void *)0)
#  endif
#endif

#define IN
#define OUT

/*
 * Definition of other common types
 */

// to delete when all completed
typedef uint32_t TEE_Result;
typedef TEE_Result TEEC_Result;

typedef uint32_t TEE_HANDLE;

/** Definition of an UUID (from RFC 4122 http://www.ietf.org/rfc/rfc4122.txt) */
typedef struct TEE_UUID {
    uint32_t timeLow;
    uint16_t timeMid;
    uint16_t timeHiAndVersion;
    uint8_t clockSeqAndNode[8];
} TEE_UUID;
typedef TEE_UUID TEEC_UUID;

/** Type definition for a TEE Identity */
typedef struct TEE_Identity {
    uint32_t login;
    TEE_UUID uuid;
} TEE_Identity;

typedef struct __TEE_PropSetHandle {
    unsigned char reserved;
} __TEE_PropSetHandle;

typedef struct __TEE_PropSetHandle* TEE_PropSetHandle;

/* Property Sets Pseudo Handles */
#define TEE_PROPSET_CURRENT_TA               (TEE_PropSetHandle)0xFFFFFFFF
#define TEE_PROPSET_CURRENT_CLIENT           (TEE_PropSetHandle)0xFFFFFFFE
#define TEE_PROPSET_TEE_IMPLEMENTATION       (TEE_PropSetHandle)0xFFFFFFFD


/* DLL Import/Export directives */

#if defined(WIN32) || defined(__ARMCC_VERSION) || defined(__WINSCW__) || defined(_WIN32_WCE)
#  define S_DLL_EXPORT __declspec(dllexport)
#  define S_DLL_IMPORT __declspec(dllimport)
#  define S_NO_RETURN  __declspec(noreturn)
#elif defined(__GNUC__)
#  define S_DLL_EXPORT __attribute__ ((visibility ("default")))
#  define S_DLL_IMPORT __attribute__ ((visibility ("default")))
#  define S_NO_RETURN  __attribute__ ((noreturn))
#else
#  define S_DLL_EXPORT
#  define S_DLL_IMPORT
#  define S_NO_RETURN
#endif

#if defined(_MSC_VER)
#define __func__ __FUNCTION__
#endif

#endif /* __TEE_TYPE_H__ */
