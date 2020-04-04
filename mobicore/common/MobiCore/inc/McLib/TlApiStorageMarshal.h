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


#ifndef  __TEE_STORAGE_H__
#define  __TEE_STORAGE_H__
#include <mcstd.h>
#include <mcutil.h>

#include "tee_internal_api.h"
#include "TlApi/TlApi.h"

#include "TlApiCryptoMarshal.h"
#include "TlApiImpl.h"

#define MC_FLAG_FILE_MAY_EXIST ( 1 << 0 )
#define MC_DRV_ID_STORAGE 0x0200

#define TLAPI_FNC_CR_SIGN_INIT_ID       19
#define TLAPI_FNC_CR_SIGN_UPDATE_ID     20
#define TLAPI_FNC_CR_SIGN_SIGN_ID       21
#define TLAPI_FNC_CR_SIGN_VERIFY_ID     22


/* ** should be put in TlApiMarshal.h ** */
typedef struct
{
    uint32_t        dataLen;
    uint32_t        position;
    unsigned char   objectID[TEE_OBJECT_ID_MAX_LEN];
    unsigned char   data[0];
}_TEE_PersistentData;


typedef struct
{
    uint32_t            flags;
    uint8_t             version;
    uint8_t             objectIDLen;
    uint16_t            cryptoLen;
    _TEE_PersistentData *persistent;
    unsigned char       crypto[0];
}TEE_FileObject;

typedef struct
{
    uint32_t            flags;
    uint8_t             version;
    uint8_t             objectIDLen;
    uint16_t            cryptoLen;
    _TEE_PersistentData *persistent;
}__TEE_ObjectHandle;

typedef struct
{
    char header[5];
    unsigned char version;
    uint16_t cryptoLen;
    uint32_t dataLen;
}STH_plaintext;

typedef struct {
    uint8_t objectIDLen;
    unsigned char reserved1[7];
    unsigned char objectID[TEE_OBJECT_ID_MAX_LEN];
    unsigned char data[];
}STH_ciphertext;

typedef struct {
    char header[6];
    unsigned char version;
    unsigned char objectIDLen;
    TEE_UUID uuid;
    unsigned char objectID[TEE_OBJECT_ID_MAX_LEN];
} STH_file_name;

#define PLAIN_TEXT_HEADER ("fname")


#endif // __TEE_STORAGE_H__
