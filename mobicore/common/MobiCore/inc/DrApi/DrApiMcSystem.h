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

#ifndef __DRAPIMCSYSTEM_H__
#define __DRAPIMCSYSTEM_H__

#include "DrApiCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Get information about the implementation of the MobiCore Driver API version.
 *
 * @param drApiVersion     pointer to Driver Api version.
 * @returns DRAPI_OK if version has been set
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
_DRAPI_EXTERN_C drApiResult_t drApiGetVersion(
    uint32_t *drApiVersion);

/** Install fastcall handler
 *
 * @param entryTable     pointer to entry table.
 * @param fastcallId     fastcall ID.
 * @returns DRAPI_OK if version has been set
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
_DRAPI_EXTERN_C drApiResult_t drApiInstallFc(
    void *entryTable, uint32_t fastcallId);


_DRAPI_EXTERN_C drApiResult_t drApiFastCall(
    uint32_t *fastcall_registers,
    uint32_t size);
#ifdef __cplusplus
}
#endif

#endif // __DRAPIMCSYSTEM_H__

