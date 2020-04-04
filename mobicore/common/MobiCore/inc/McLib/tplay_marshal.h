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

#ifndef __TPLAY_MARSHAL_H__
#define __TPLAY_MARSHAL_H__ __FILE__

#include "DrApi/DrApiCommon.h"
#include "TlApi/TlApiError.h"
#include "TlApi/TPlay.h"

/**
 * Driver ID
 */
#define TPLAY_DR_ID TB_DRV_ID_TPLAY

/**
 * Function id definitions
 */
#define FID_DR_OPEN_SESSION             1
#define FID_DR_CLOSE_SESSION            2
#define FID_DR_PROCESS_DRM_CONTENT      3
#define FID_DR_CHECK_LINK               4
#define FID_DR_UNKNOWN_CMD              100 /* For test purposes */

/**
 * Secure cipher request data structure.
 * This structure helps define a buffer that contains mixed encrypted and clear data.
 */
typedef struct {
    uint8_t                         sHandle;
    tlApiDrmDecryptContext_t           decryptCtx;
    uint8_t                         *input;
    tlApiDrmInputSegmentDescriptor_t   inputDesc;
    uint32_t                        processMode;
    uint8_t                         *rfu;
} tlDrmApiDrmContent_t, *tlDrmApiDrmContent_ptr;

/**
 * Link status data structure.
 */
typedef struct {
    uint8_t                         sHandle;
    tlApiDrmLink_t                   link;
} tlDrmApiLink_t, *tlDrmApiLink_ptr;

/**
 * Union of marshaling parameters. */
/* If adding any function, add the marshaling structure here
 */
typedef struct {
    uint32_t     functionId;  /* Function identifier. */
    union {
        uint8_t                 *returned_sHandle;
        uint8_t                 sHandle_to_close;
        tlDrmApiDrmContent_t    drmContent;
        tlDrmApiLink_t          link;
        int32_t                 retVal;      /* Return value */
    } payload;
} tplayMarshalingParam_t, *tplayMarshalingParam_ptr;


#endif // __TPLAY_MARSHAL_H__


