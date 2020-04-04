 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 *
 *
 * @addtogroup TlApi
 * @{
 * @file
 * Marshaling types and declarations.
 *
 * Functions for the marshaling of function ID and parameters.
 *
 */
 
#ifndef __TLAPICRYPTOMARSHAL_H__
#define __TLAPICRYPTOMARSHAL_H__


#include "mcDriverId.h"

#include "TlApi/TlApiError.h"
#include "TlApi/TlApiCrypto.h"
#include "TlApi/TlApiSecurity.h"
#include "TlApi/TlApiMcSystem.h"
#include "TlApi/TlApiTime.h"


/** Each function must have its own ID.
 * Extend this list if you add a new function.
 */
typedef enum {
    FID_KPD_GRAB_KEYPAD = 0,            /**< Function to reserve the keypad. */
    FID_KPD_GET_C,                      /**< Function to read a character from the keypad. */
    FID_KPD_GET_MULTIPLE_C,             /**< Function to read multiple characters from the keypad. */
    FID_KPD_RELEASE_KEYPAD              /**< Function to release the grabbed keypad. */
} kpdFuncID_t;


/** Each function must have its own ID.
 * Extend this list if you add a new function.
 */
typedef enum {
    FID_CRYPTO_MD                       =  0,   /**< ID for message digest algorithms. */
    FID_CRYPTO_MD_INIT                  =  1,   /**< Function to init a message digest. */
    FID_CRYPTO_MD_INIT_WITH_DATA        =  2,   /**< Function to init a message digest with data. */
    FID_CRYPTO_MD_UPDATE                =  3,   /**< Function to update a message digest. */
    FID_CRYPTO_MD_DOFINAL               =  4,   /**< Function to finalize a message digest. */

    FID_CRYPTO_SIG                      =  5,   /**< ID for signature algorithms. */
    FID_CRYPTO_SIG_INIT                 =  6,   /**< Function to init a signature. */
    FID_CRYPTO_SIG_INIT_WITH_DATA       =  7,   /**< Function to init a signature with data. */
    FID_CRYPTO_SIG_UPDATE               =  8,   /**< Function to update a signature. */
    FID_CRYPTO_SIG_SIGN                 =  9,   /**< Function to make a signature. */
    FID_CRYPTO_SIG_VERIFY               = 10,   /**< Function to verify a signature. */

    FID_CRYPTO_RNG                      = 11,   /**< ID for RNG algorithms. */
    FID_CRYPTO_RNG_GENERATE_DATA        = 12,   /**< Function to generate random data. */

    FID_CRYPTO_CIPHER                   = 13,   /**< ID for cipher algorithms. */
    FID_CRYPTO_CIPHER_INIT              = 14,   /**< Function to init a cipher. */
    FID_CRYPTO_CIPHER_INIT_WITH_DATA    = 15,   /**< Function to init a cipher with data. */
    FID_CRYPTO_CIPHER_UPDATE            = 16,   /**< Function to update a cipher. */
    FID_CRYPTO_CIPHER_DOFINAL           = 17,   /**< Function to finalize a cipher. */

    FID_CRYPTO_SESSION_ABORT            = 18,   /**< Function to abort a crypto session. */

    FID_CRYPTO_GENERATE_KEY_PAIR        = 19,   /**< Function to generate a key pair. */
    FID_CRYPTO_GENERATE_KEY_PAIR_BUFFER = 20,   /**< Function to generate a key pair into a buffer. */

    FID_SECURITY_WRAP_OBJECT            = 21,   /**< Function to wrap given data and create a secure object. */
    FID_SECURITY_UNWRAP_OBJECT          = 22,   /**< Function to unwrap given secure object and create plaintext data. */
    FID_SECURITY_GET_SUID               = 23,   /**< Function to acquire System on chip Unique ID. */
    FID_SECURITY_IS_DEVICE_BOUND        = 24,   /**< Currently not used. */
    FID_SECURITY_BIND_DEVICE            = 25,   /**< Currently not used. */

    FID_SYSTEM_GET_VERSION              = 26,   /**< Function to get information about the underlying MobiCore version. */

    FID_SECURITY_GET_TIME_STAMP         = 27,   /**< Function to get a secure time stamp. */

    FID_SECURITY_DERIVE_KEY             = 28,   /**< Function to derive key. */

    FID_CRYPTO_MD_DOFINAL_WITHOUT_CLOSE = 29,   /**< Function to finalize a message digest without close. */
    FID_CRYPTO_OVERWRITE_KEY            = 30,   /**< Function to overwrite key. */
    FID_CRYPTO_MAC_INIT                 = 31,   /**< Function to initialize hmac structures with key that is earlier given. */
    FID_CRYPTO_SIG_SIGN_WITHOUT_CLOSE   = 32,   /**< Function to finalize a sign without close. */
    FID_CRYPTO_SIG_VERIFY_WITHOUT_CLOSE = 33,   /**< Function to finalize a verify without close. */
    FID_CRYPTO_CIPHER_WITHOUT_CLOSE     = 34,   /**< Function to finalize a cipher without close. */
    FID_SECURITY_ENDORSE                = 35,   /**< Function to create endorsement */

    FID_CRYPTO_CIPHER_UPDATE_IV         = 36,   /**< Function to update cipher IV */

} cryptoFuncID_t;


/** Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */
typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of session to be aborted           */
} tlApiCrAbort_t, *tlApiCrAbort_ptr;


typedef struct {
    tlApiKeyPair_t          *keyPair;                       /* reference to key pair structure           */
    tlApiKeyPairType_t      type;                           /* see enum keyPairType_t                    */
    size_t                  len;                            /* requested byte length of keys             */
    uint8_t                 *buffer;                        /* reference to buffer into which generated key components are written  */
    size_t                  bufferLen;                      /* length of buffer for generated components */
} tlApiCrGenerateKeyPair_t, *tlApiCrGenerateKeyPair_ptr;


typedef struct {
    tlApiCrSession_t        *pSessionHandle;                /* reference to generated Cipher session handle  */
    tlApiCipherAlg_t        alg;                            /* see enum cipherMode_t                     */
    tlApiCipherMode_t       mode;                           /* TLAPI_MODE_ENCRYPT or TLAPI_MODE_DECRYPT  */
    const tlApiKey_t        *key;                           /* key for this session                      */
    const uint8_t           *buffer;                        /* reference to algorithm specific data like initial values for CBC  */
    size_t                  bufferLen;                      /* length of buffer containing algorithm specific data  */
} tlApiCipherInit_t, *tlApiCipherInit_ptr;

typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running Cipher session        */
    const uint8_t           *srcData;                       /* reference to input data to be encrypted/decrypted  */
    size_t                  srcLen;                         /* byte length of input data to be encrypted/decrypted  */
    uint8_t                 *destData;                      /* reference to result area                  */
    size_t                  *destLen;                       /* [in] byte length of output buffer. [out] byte length of generated output data  */
} tlApiCipherUpdate_t, *tlApiCipherUpdate_ptr;


typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running Cipher session        */
    const uint8_t           *srcData;                       /* reference to input data to be encrypted/decrypted  */
    size_t                  srcLen;                         /* byte length of input data to be encrypted/decrypted  */
    uint8_t                 *destData;                      /* reference to result area                  */
    size_t                  *destLen;                       /* [in] byte length of buffer for output data. [out] byte length of generated output  */
} tlApiCipherDoFinal_t, *tlApiCipherDoFinal_ptr;


typedef struct {
    tlApiCrSession_t        *pSessionHandle;                /* reference to generated Signatures session handle  */
    const tlApiKey_t        *key;                           /* key for this session                      */
    tlApiSigMode_t          mode;                           /* TLAPI_MODE_SIGN or TLAPI_MODE_VERIFY      */
    tlApiSigAlg_t           alg;                            /* see enum of algorithms                    */
    const uint8_t           *buffer;                        /* reference to algorithm specific data like seed for hash  */
    size_t                  bufferLen;                      /* length of buffer containing algorithm specific data  */
} tlApiSignatureInit_t, *tlApiSignatureInit_ptr;


typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running Signature session     */
    const uint8_t           *message;                       /* reference to message to be signed/verified  */
    size_t                  messageLen;                     /* byte length of message                    */
} tlApiSignatureUpdate_t, *tlApiSignatureUpdate_ptr;


typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running Signature session     */
    const uint8_t           *message;                       /* reference to message to be signed         */
    size_t                  messageLen;                     /* byte length of message                    */
    uint8_t                 *signature;                     /* reference to generated signature          */
    size_t                  *signatureLen;                  /* [in] byte length of signature buffer. [out] byte length of generated signature  */
} tlApiSignatureSign_t, *tlApiSignatureSign_ptr;


typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running Signature session     */
    const uint8_t           *message;                       /* reference to message to be verified       */
    size_t                  messageLen;                     /* byte length of message                    */
    const uint8_t           *signature;                     /* reference to signature to be verified     */
    size_t                  signatureLen;                   /* byte length of signature                  */
    bool                    *validity;                      /* reference to verification result, TRUE if verified, otherwise FALSE  */
} tlApiSignatureVerify_t, *tlApiSignatureVerify_ptr;

typedef struct {
    tlApiCrSession_t        *pSessionHandle;                /* reference to generated Message Digest session handle  */
    tlApiMdAlg_t            alg;                            /* see enum mdAlg_t                          */
    const uint8_t           *buffer;                        /* reference to previously calculated hash data        */
    const uint8_t           *lenPreHashedData;              /* byte array in big endian format containing length of previously calculated hash  */
} tlApiMessageDigestInit_t, *tlApiMessageDigestInit_ptr;


typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running session Message Digest session  */
    const uint8_t           *message;                       /* reference to message to be hashed         */
    size_t                  messageLen;                     /* byte length of input data to be hashed    */
} tlApiMessageDigestUpdate_t, *tlApiMessageDigestUpdate_ptr;


typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running session Message Digest session  */
    const uint8_t           *message;                       /* reference to message to be hashed         */
    size_t                  messageLen;                     /* byte length of message                    */
    uint8_t                 *hash;                          /* reference to generated hash               */
    size_t                  *hashLen;                       /* [in] byte length of hash buffer. [out] byte length of generated hash data   */
} tlApiMessageDigestDoFinal_t, *tlApiMessageDigestDoFinal_ptr;


typedef struct {
    tlApiRngAlg_t           alg;                            /* see enum randomDataGenerationAlg_t        */
    uint8_t                 *randomBuffer;                  /* reference to generated random data        */
    size_t                  *randomLen;                      /* byte length of requested random data      */
} tlApiRandomGenerateData_t, *tlApiRandomGenerateData_ptr;


typedef struct {
    uint32_t                unused;                         /* place holder - unused !!!                 */
} tlApiGrabKeypad_t, *tlApiGrabKeypad_ptr;


typedef struct {
    uint32_t                *c;                             /* pointer to the rx-character               */
    uint32_t                timeout;                        /* time in milliseconds to wait              */
} tlApiGetKeypadEvent_t, *tlApiGetKeypadEvent_ptr;


typedef struct {
    uint32_t                unused;                         /* place holder - unused !!!                 */
} tlApiReleaseKeypadGrab_t, *tlApiReleaseKeypadGrab_ptr;


typedef struct {
    const void              *src;                   /* Source data. */
    size_t                  plainLen;               /* Length of header. */
    size_t                  encryptedLen;           /* Length of payload to encrypt. */
    void                    *dest;                  /* Destination buffer (secure object). */
    size_t                  *destLen;               /* [in] Length of the destination buffer.
                                                       [out] Length of output data. */
    mcSoContext_t           context;                /* Context of operation. */
    mcSoLifeTime_t          lifetime;               /** Secure object lifetime and flags combined. */
    const tlApiSpTrustletId_t *consumer;            /* NULL or trustlet identifier for delegated wrapping. */
} tlApiWrapObject_t, *tlApiWrapObject_ptr;


typedef struct {
    void            *src;           /* Source data (secure object). */
    void            *dest;          /* Destination buffer (unwrapped data). */
    size_t          *destLen;       /* [in] Length of the destination buffer.
                                       [out] Length of output data. */
    size_t          srcLen;         /* [in] Length of the source buffer. */
    uint32_t        flags;          /* [in] Flags for unwrap. */
} tlApiUnwrapObject_t, *tlApiUnwrapObject_ptr;


typedef struct {
    mcSuid_t                *suid;                          /**< System on chip Unique ID                  */
} tlApiGetSuid_t, *tlApiGetSuid_ptr;


typedef struct {
    mcVersionInfo_t        *mcVersionInfo;                  /**< Version of MobiCore                        */
} tlApiGetMcVersion_t, *tlApiGetMcVersion_ptr;


typedef struct {
    timestamp_t             *ts;                            /**< Returned time stamp                          */
    tsSource_t              source;                         /**< Requested timer source                       */
} tlApiGetTimeStamp_t, *tlApiGetTimeStamp_ptr;


typedef struct {
    const void          *seed;      /* Source seed. */
    size_t              seedLen;    /* Length of seed. */
    void                *dest;      /* Destination buffer (key). */
    size_t              destLen;   /* Length of the destination buffer. */
    mcSoContext_t       context;    /* Context of operation. */
    mcSoLifeTime_t      lifetime;   /** key lifetime. */
} tlApiDeriveKey_t, *tlApiDeriveKey_ptr;

typedef struct {
    tlApiCrSession_t        sessionHandle;                  /* handle of a running session Message Digest session  */
    const uint8_t           *message;                       /* reference to message to be hashed         */
    size_t                  messageLen;                     /* byte length of message                    */
    uint8_t                 *hash;                          /* reference to generated hash               */
    size_t                  *hashLen;                       /* [in] byte length of hash buffer. [out] byte length of generated hash data   */
} tlApiMessageDigestDoFinalWithOutClose_t, *tlApiMessageDigestDoFinalWithOutClose_ptr;

typedef struct {
    tlApiCrSession_t        sessionHandle;
    const tlApiKey_t        *keyData;
} tlApiOverwriteKey_t, *tlApiOverwriteKey_ptr;

typedef struct {
    tlApiCrSession_t        sessionHandle;
    uint8_t                 *iv;
    size_t                  ivLen;
} tlApiMacInit_t, *tlApiMacInit_ptr;

typedef struct {
    tlApiCrSession_t        sessionHandle;
    const uint8_t           *iv;
    size_t                  ivLen;
} tlApiCipherUpdateIV_t, *tlApiCipherUpdateIV_ptr;

#define STH_PUBLIC_FILE_NAME_SIZE 20
typedef struct S_UUID_t {
    uint32_t timeLow;
    uint16_t timeMid;
    uint16_t timeHiAndVersion;
    uint8_t clockSeqAndNode[8];
} UUID_t;

typedef enum {
    STH_MESSAGE_TYPE_LOOK = 0,
    STH_MESSAGE_TYPE_READ,
    STH_MESSAGE_TYPE_WRITE,
    STH_MESSAGE_TYPE_DELETE
} tlApiStorageOps_t;                                                                        

typedef struct {
    uint32_t status;
    uint8_t type;
    uint8_t reserved0;
    uint16_t flags;
    uint32_t payloadLen;
    UUID_t     uuid;
    unsigned char filename[STH_PUBLIC_FILE_NAME_SIZE];
    unsigned char payload[];
} STH_TA_message_t; 

/*typedef struct {
    storageOps_t operation;
    union {
        storageLook_t look;
        storageRead_t read;
        storageWrite_t write;
        storageDelete_t del;
    }payload;
} marshalingParam;*/ // merge to marshalingParam

typedef struct {
    const void          *msg;      /* Source message. */
    size_t              msgLen;    /* Length of message. */
    void                *dst;      /* Destination buffer (endorsement). */
    size_t              *dstLen;   /* Length of the destination buffer. */
    mcScope_t           scope;     /* Scope of the endorsement. */
} tlApiEndorse_t;

/** Maximum number of parameter . */
#define MAX_MAR_LIST_LENGTH 8                      /**< Maximum list of possible marshaling parameters. */

/** Marshaled union. */
typedef struct {
    uint32_t     functionId;                       /**< Function identifier. */
    union {
        tlApiCrAbort_t                      crAbort;
        tlApiCrGenerateKeyPair_t            crGenerateKeyPair;
        tlApiCipherInit_t                   cipherInit;
        tlApiCipherUpdate_t                 cipherUpdate;
        tlApiCipherDoFinal_t                cipherDoFinal;
        tlApiSignatureInit_t                signatureInit;
        tlApiSignatureUpdate_t              signatureUpdate;
        tlApiSignatureSign_t                signatureSign;
        tlApiSignatureVerify_t              signatureVerify;
        tlApiMessageDigestInit_t            messageDigestInit;
        tlApiMessageDigestUpdate_t          messageDigestUpdate;
        tlApiMessageDigestDoFinal_t         messageDigestDoFinal;
        tlApiRandomGenerateData_t           randomGenerateData;
        tlApiGrabKeypad_t                   grabKeypad;
        tlApiGetKeypadEvent_t               getKeypadEvent;
        tlApiReleaseKeypadGrab_t            releaseKeypadGrab;
        tlApiWrapObject_t                   wrapObject;
        tlApiUnwrapObject_t                 unwrapObject;
        tlApiGetSuid_t                      getSuid;
        tlApiGetMcVersion_t                 getVersion;
        tlApiGetTimeStamp_t                 getTimeStamp;
        tlApiDeriveKey_t                    deriveKey;
        tlApiOverwriteKey_t                 overwriteKey;
        tlApiMacInit_t                      macInit;
        tlApiEndorse_t                      endorse;
        tlApiCipherUpdateIV_t               cipherUpdateIV;
        tlApiResult_t                       retVal;
        STH_TA_message_t                    taMessage;
        uint32_t                            parameter[MAX_MAR_LIST_LENGTH];   /* untyped parameter list (expands union to 8 entries) */
    } payload;
} marshalingParamCR_t, *marshalingParamCR_ptr;

#endif // __TLAPICRYPTOMARSHAL_H__

/** @} */
