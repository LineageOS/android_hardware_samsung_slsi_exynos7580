/** @addtogroup DRIVER_TUI
 * @{
 * @file drApiMarshal.h
 * Marshaling types and declarations for TUI driver and trustlet.
 *
 * Functions for the marshaling of function ID and parameters.
 *
 * <Copyright goes here>
 */
#ifndef __TLAPIMARSHAL_TUI_H__
#define __TLAPIMARSHAL_TUI_H__

#include "TlApi/TlApiError.h"
#include "TlApi/TlApiTui.h"

/** Invalid session id. Returned in case of an error. */
#define DR_SID_INVALID      0xffffffff

/** Each function must have its own ID.
 * Extend this list if you add a new function.
 */
typedef enum {
    FID_DR_NONE = 0,
    FID_DR_OPEN_SESSION,
    FID_DR_CLOSE_SESSION,
    FID_DR_RESERVED0,
    FID_DR_SET_IMAGE,
    FID_DR_GET_SCREEN_INFO,
    FID_DR_GET_TOUCH_EVENT,
    FID_DR_RESERVED1,
} tuiFuncID_t;

/** Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */
typedef struct {
    union{
        uint32_t                    *version;
        uint32_t                    *securityLevel;
        tlApiTuiImage_ptr           image;
        tlApiTuiScreenInfo_ptr      screenInfo;
        tlApiTuiTouchEvent_ptr      touchEvent;
    }content;
    tlApiTuiCoordinates_t           coordinates;

} tuiMarshalingData_t, *tuiMarshalingData_ptr;

/** Union of marshaling parameters. */
/* If adding any function, add the marshaling structure here */
typedef struct {
    uint32_t     functionId;                       /**< Function identifier. */
    union {
        tuiMarshalingData_t  params;
        tlApiResult_t       retVal; /** Return value */
    } payload;

} tuiMarshalingParam_t, *tuiMarshalingParam_ptr;

#endif // __TLAPIMARSHAL_TUI_H__

/** @} */
