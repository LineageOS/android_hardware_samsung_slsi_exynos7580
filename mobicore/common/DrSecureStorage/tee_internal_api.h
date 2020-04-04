/*
 * Copyright (c) 2012 Trusted Logic Mobility SAS.
 * All Rights Reserved.
 *
 * The present software is the confidential and proprietary information of
 * Trusted Logic Mobility SAS. You shall not disclose the present software
 * and shall use it only in accordance with the terms of the license
 * agreement you entered into with Trusted Logic Mobility SAS.
 * "Trusted Logic" is a registered trademark of Trusted Logic SAS.
 * This software may be subject to export or import laws in certain
 * countries.
 */

//TODO: Remove functions that are not provided in potato release

#ifndef   __TEE_INTERNAL_API_H__
#define   __TEE_INTERNAL_API_H__

typedef uint32_t TEE_Result;

#define TEE_SUCCESS                      ((TEE_Result)0x00000000)

/**
 * Generic error code : Generic error
 **/
#define TEE_ERROR_GENERIC                ((TEE_Result)0xFFFF0000)

/**
 * Generic error code : The underlying security system denies the access to the
 * object
 **/
#define TEE_ERROR_ACCESS_DENIED          ((TEE_Result)0xFFFF0001)

/**
 * Generic error code : The pending operation is cancelled.
 **/
#define TEE_ERROR_CANCEL                 ((TEE_Result)0xFFFF0002)

/**
 * Generic error code : The underlying system detects a conflict
 **/
#define TEE_ERROR_ACCESS_CONFLICT        ((TEE_Result)0xFFFF0003)

/**
 * Generic error code : Too much data for the operation or some data remain
 * unprocessed by the operation.
 **/
#define TEE_ERROR_EXCESS_DATA            ((TEE_Result)0xFFFF0004)

/**
 * Generic error code : Error of data format
 **/
#define TEE_ERROR_BAD_FORMAT             ((TEE_Result)0xFFFF0005)

/**
 * Generic error code : The specified parameters are invalid
 **/
#define TEE_ERROR_BAD_PARAMETERS         ((TEE_Result)0xFFFF0006)

/**
 * Generic error code : Illegal state for the operation.
 **/
#define TEE_ERROR_BAD_STATE              ((TEE_Result)0xFFFF0007)

/**
 * Generic error code : The item is not found
 **/
#define TEE_ERROR_ITEM_NOT_FOUND         ((TEE_Result)0xFFFF0008)

/**
 * Generic error code : The specified operation is not implemented
 **/
#define TEE_ERROR_NOT_IMPLEMENTED        ((TEE_Result)0xFFFF0009)

/**
 * Generic error code : The specified operation is not supported
 **/
#define TEE_ERROR_NOT_SUPPORTED          ((TEE_Result)0xFFFF000A)

/**
 * Generic error code : Insufficient data is available for the operation.
 **/
#define TEE_ERROR_NO_DATA                ((TEE_Result)0xFFFF000B)

/**
 * Generic error code : Not enough memory to perform the operation
 **/
#define TEE_ERROR_OUT_OF_MEMORY          ((TEE_Result)0xFFFF000C)

/**
 * Generic error code : The service is currently unable to handle the request;
 * try later
 **/
#define TEE_ERROR_BUSY                   ((TEE_Result)0xFFFF000D)

/**
 * Generic error code : security violation
 **/
#define TEE_ERROR_SECURITY               ((TEE_Result)0xFFFF000F)

/**
 * Generic error code : the buffer is too short
 **/
#define TEE_ERROR_SHORT_BUFFER           ((TEE_Result)0xFFFF0010)

/**
 * Generic error code : the operation is not terminated
 **/
#define TEE_PENDING                      ((TEE_Result)0xFFFF2000)

/**
 * Generic error code : A timeout occurred
 **/
#define TEE_ERROR_TIMEOUT                ((TEE_Result)0xFFFF3001)

/**
 * Generic error code : Overflow
 **/
#define TEE_ERROR_OVERFLOW               ((TEE_Result)0xFFFF300F)

/*------------------------------------------------------------------------------
   Communication Error Codes
------------------------------------------------------------------------------*/

/**
 * Generic communication error
 **/
#define TEE_ERROR_COMMUNICATION          ((TEE_Result)0xFFFF000E)

/**
 * Error of communication: The target of the connection is dead
 **/
#define TEE_ERROR_TARGET_DEAD            ((TEE_Result)0xFFFF3024)

/*------------------------------------------------------------------------------
   Storage Error Codes
------------------------------------------------------------------------------*/

/** File system error code: not enough space to complete the operation. */
#define TEE_ERROR_STORAGE_NO_SPACE       ((TEE_Result)0xFFFF3041)

/*------------------------------------------------------------------------------
   Crypto error codes
------------------------------------------------------------------------------*/
#define TEE_ERROR_MAC_INVALID          ((TEE_Result)0xFFFF3071)

#define TEE_ERROR_SIGNATURE_INVALID    ((TEE_Result)0xFFFF3072)

/*------------------------------------------------------------------------------
   Date error codes
------------------------------------------------------------------------------*/
#define TEE_ERROR_TIME_NOT_SET         ((TEE_Result)0xFFFF5000)

#define TEE_ERROR_TIME_NEEDS_RESET     ((TEE_Result)0xFFFF5001)


/** Definition of an UUID (from RFC 4122 http://www.ietf.org/rfc/rfc4122.txt) */
typedef struct
{
   uint32_t timeLow;
   uint16_t timeMid;
   uint16_t timeHiAndVersion;
   uint8_t clockSeqAndNode[8];
} TEE_UUID;

/** Type definition for a TEE Identity */
typedef struct TEE_Identity
{
	uint32_t login;
	TEE_UUID uuid;
} TEE_Identity;

typedef uint32_t S_HANDLE;

#define S_VAR_NOT_USED(variable) do{(void)(variable);}while(0);

#define OUT
#define IN

#define TEE_EXPORT
#define TA_EXPORT


/*---------------------------------------------------------
      Common type definition
---------------------------------------------------------*/

typedef union
{

   struct
   {
      void*    buffer;
      size_t   size;
   } memref;

   struct
   {
      uint32_t a;
      uint32_t b;
   } value;

}
TEE_Param;

typedef S_HANDLE  TEE_TASessionHandle;
typedef S_HANDLE  TEE_PropSetHandle;
typedef S_HANDLE  TEE_ObjectHandle;
typedef S_HANDLE  TEE_ObjectEnumHandle;
typedef S_HANDLE  TEE_OperationHandle;

#define TEE_HANDLE_NULL ((S_HANDLE)0)

#define TEE_PARAM_TYPES(t0,t1,t2,t3)  ((t0) | ((t1) << 4) | ((t2) << 8) | ((t3) << 12))
#define TEE_PARAM_TYPE_GET(t, i) (((t) >> (i*4)) & 0xF)

typedef enum {
               TEE_DATA_SEEK_SET,
               TEE_DATA_SEEK_CUR,
               TEE_DATA_SEEK_END
             } TEE_Whence;

typedef struct TEE_Time
{
   uint32_t seconds;
   uint32_t millis;
} TEE_Time;

typedef struct {
                  uint32_t algorithm;
                  uint32_t operationClass;
                  uint32_t mode;
                  uint32_t digestLength;
                  uint32_t maxKeySize;
                  uint32_t keySize;
                  uint32_t requiredKeyUsage;
                  uint32_t handleState;
                } TEE_OperationInfo;


typedef enum {
               TEE_MODE_ENCRYPT,
               TEE_MODE_DECRYPT,
               TEE_MODE_SIGN,
               TEE_MODE_VERIFY,
               TEE_MODE_MAC,
               TEE_MODE_DIGEST,
               TEE_MODE_DERIVE
              } TEE_OperationMode;


typedef struct {
                  uint32_t attributeID;
                  union
                  {
                     struct
                     {
                        void* buffer;
                        size_t length;
                     }ref;
                     struct
                     {
                        uint32_t a, b;
                     }value;
                  }content;
               } TEE_Attribute;

typedef struct {
                  uint32_t objectType;
                  uint32_t objectSize;
                  uint32_t maxObjectSize;
                  uint32_t objectUsage;
                  uint32_t dataSize;
                  uint32_t dataPosition;
                  uint32_t handleFlags;
               } TEE_ObjectInfo;


/*------------------------------------------------------------------------------
         Constants
------------------------------------------------------------------------------*/

#define TEE_TIMEOUT_INFINITE                 0xFFFFFFFF

/* Login types */
#define TEE_LOGIN_PUBLIC                     0x00000000
#define TEE_LOGIN_USER                       0x00000001
#define TEE_LOGIN_GROUP                      0x00000002
#define TEE_LOGIN_APPLICATION                0x00000004
#define TEE_LOGIN_APPLICATION_USER           0x00000005
#define TEE_LOGIN_APPLICATION_GROUP          0x00000006
#define TEE_LOGIN_TRUSTED_APP                0xF0000000

/* Parameter types */
#define TEE_PARAM_TYPE_NONE                  0x0
#define TEE_PARAM_TYPE_VALUE_INPUT           0x1
#define TEE_PARAM_TYPE_VALUE_OUTPUT          0x2
#define TEE_PARAM_TYPE_VALUE_INOUT           0x3
#define TEE_PARAM_TYPE_MEMREF_INPUT          0x5
#define TEE_PARAM_TYPE_MEMREF_OUTPUT         0x6
#define TEE_PARAM_TYPE_MEMREF_INOUT          0x7

#define TEE_MEMORY_ACCESS_READ               0x00000001
#define TEE_MEMORY_ACCESS_WRITE              0x00000002
#define TEE_MEMORY_ACCESS_ANY_OWNER          0x00000004

#define TEE_ORIGIN_API                       1
#define TEE_ORIGIN_COMMS                     2
#define TEE_ORIGIN_TEE                       3
#define TEE_ORIGIN_TRUSTED_APP               4

/* Property Sets Pseudo Handles */
#define TEE_PROPSET_CURRENT_TA               (TEE_PropSetHandle)0xFFFFFFFF
#define TEE_PROPSET_CURRENT_CLIENT           (TEE_PropSetHandle)0xFFFFFFFE
#define TEE_PROPSET_TEE_IMPLEMENTATION       (TEE_PropSetHandle)0xFFFFFFFD

/* Allocation hints */
#define TEE_ALLOCATION_HINT_ZEROED 0x00000000

/* Crypto Usage Constants */
#define TEE_USAGE_EXTRACTABLE                0x00000001
#define TEE_USAGE_ENCRYPT                    0x00000002
#define TEE_USAGE_DECRYPT                    0x00000004
#define TEE_USAGE_MAC                        0x00000008
#define TEE_USAGE_SIGN                       0x00000010
#define TEE_USAGE_VERIFY                     0x00000020
#define TEE_USAGE_DERIVE                     0x00000040

/* Crypto Handle Flag Constants */
#define TEE_HANDLE_FLAG_PERSISTENT           0x00010000
#define TEE_HANDLE_FLAG_INITIALIZED          0x00020000
#define TEE_HANDLE_FLAG_KEY_SET              0x00040000
#define TEE_HANDLE_FLAG_EXPECT_TWO_KEYS      0x00080000

/* Crypto Operation Constants */
#define TEE_OPERATION_CIPHER                 1
#define TEE_OPERATION_MAC                    3
#define TEE_OPERATION_AE                     4
#define TEE_OPERATION_DIGEST                 5
#define TEE_OPERATION_ASYMMETRIC_CIPHER      6
#define TEE_OPERATION_ASYMMETRIC_SIGNATURE   7
#define TEE_OPERATION_KEY_DERIVATION         8

/* Crypto Algortithm Constants */
#define TEE_ALG_AES_ECB_NOPAD                0x10000010
#define TEE_ALG_AES_CBC_NOPAD                0x10000110
#define TEE_ALG_AES_CTR                      0x10000210
#define TEE_ALG_AES_CTS                      0x10000310
#define TEE_ALG_AES_XTS                      0x10000410
#define TEE_ALG_AES_CBC_MAC_NOPAD            0x30000110
#define TEE_ALG_AES_CBC_MAC_PKCS5            0x30000510
#define TEE_ALG_AES_CMAC                     0x30000610
#define TEE_ALG_AES_CCM                      0x40000710
#define TEE_ALG_AES_GCM                      0x40000810
#define TEE_ALG_DES_ECB_NOPAD                0x10000011
#define TEE_ALG_DES_CBC_NOPAD                0x10000111
#define TEE_ALG_DES_CBC_MAC_NOPAD            0x30000111
#define TEE_ALG_DES_CBC_MAC_PKCS5            0x30000511
#define TEE_ALG_DES_CMAC                     0x30000611
#define TEE_ALG_DES3_ECB_NOPAD               0x10000013
#define TEE_ALG_DES3_CBC_NOPAD               0x10000113
#define TEE_ALG_DES3_CBC_MAC_NOPAD           0x30000113
#define TEE_ALG_DES3_CBC_MAC_PKCS5           0x30000513
#define TEE_ALG_DES3_CMAC                    0x30000613
#define TEE_ALG_RSASSA_PKCS1_V1_5_MD5        0x70001830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA1       0x70002830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA224     0x70003830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA256     0x70004830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA384     0x70005830
#define TEE_ALG_RSASSA_PKCS1_V1_5_SHA512     0x70006830
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA1   0x70212930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA224 0x70313930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA256 0x70414930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA384 0x70515930
#define TEE_ALG_RSASSA_PKCS1_PSS_MGF1_SHA512 0x70616930
#define TEE_ALG_RSAES_PKCS1_V1_5             0x60000130
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA1   0x60210230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA224 0x60310230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA256 0x60410230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA384 0x60510230
#define TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA512 0x60610230
#define TEE_ALG_RSA_NOPAD                    0x60000030
#define TEE_ALG_DSA_SHA1                     0x70002131
#define TEE_ALG_DH_DERIVE_SHARED_SECRET      0x80000032
#define TEE_ALG_MD5                          0x50000001
#define TEE_ALG_SHA1                         0x50000002
#define TEE_ALG_SHA224                       0x50000003
#define TEE_ALG_SHA256                       0x50000004
#define TEE_ALG_SHA384                       0x50000005
#define TEE_ALG_SHA512                       0x50000006
#define TEE_ALG_HMAC_MD5                     0x30000001
#define TEE_ALG_HMAC_SHA1                    0x30000002
#define TEE_ALG_HMAC_SHA224                  0x30000003
#define TEE_ALG_HMAC_SHA256                  0x30000004
#define TEE_ALG_HMAC_SHA384                  0x30000005
#define TEE_ALG_HMAC_SHA512                  0x30000006

/* Storage ID Values */
#define TEE_STORAGE_PRIVATE                  0x00000001

/* Data Flags */
#define TEE_DATA_FLAG_ACCESS_READ            0x00000001
#define TEE_DATA_FLAG_ACCESS_WRITE           0x00000002
#define TEE_DATA_FLAG_ACCESS_WRITE_META      0x00000004
#define TEE_DATA_FLAG_SHARE_READ             0x00000010
#define TEE_DATA_FLAG_SHARE_WRITE            0x00000020
#define TEE_DATA_FLAG_CREATE                 0x00000200
#define TEE_DATA_FLAG_EXCLUSIVE              0x00000400

/* Misc */
#define TEE_DATA_MAX_POSITION                0xFFFFFFFF
#define TEE_OBJECT_ID_MAX_LEN                0x40

/* Object Types */
#define TEE_TYPE_AES                         0xA0000010
#define TEE_TYPE_DES                         0xA0000011
#define TEE_TYPE_DES3                        0xA0000013
#define TEE_TYPE_HMAC_MD5                    0xA0000001
#define TEE_TYPE_HMAC_SHA1                   0xA0000002
#define TEE_TYPE_HMAC_SHA224                 0xA0000003
#define TEE_TYPE_HMAC_SHA256                 0xA0000004
#define TEE_TYPE_HMAC_SHA384                 0xA0000005
#define TEE_TYPE_HMAC_SHA512                 0xA0000006
#define TEE_TYPE_RSA_PUBLIC_KEY              0xA0000030
#define TEE_TYPE_RSA_KEYPAIR                 0xA1000030
#define TEE_TYPE_DSA_PUBLIC_KEY              0xA0000031
#define TEE_TYPE_DSA_KEYPAIR                 0xA1000031
#define TEE_TYPE_DH_KEYPAIR                  0xA1000032
#define TEE_TYPE_GENERIC_SECRET              0xA0000000

/* Object Attribute Identifier Flags - bits 28 and 29 indicate the type (value/ref) and its visibility */
#define TEE_ATTR_FLAG_VALUE                  0x20000000
#define TEE_ATTR_FLAG_PUBLIC                 0x10000000

/* Operation and Object Attribute ID Values */
#define TEE_ATTR_SECRET_VALUE                0xC0000000
#define TEE_ATTR_RSA_MODULUS                 0xD0000130
#define TEE_ATTR_RSA_PUBLIC_EXPONENT         0xD0000230
#define TEE_ATTR_RSA_PRIVATE_EXPONENT        0xC0000330
#define TEE_ATTR_RSA_PRIME1                  0xC0000430 // p
#define TEE_ATTR_RSA_PRIME2                  0xC0000530 // q
#define TEE_ATTR_RSA_EXPONENT1               0xC0000630 // dp
#define TEE_ATTR_RSA_EXPONENT2               0xC0000730 // dq
#define TEE_ATTR_RSA_COEFFICIENT             0xC0000830 // iq
#define TEE_ATTR_DSA_PRIME                   0xD0001031 // p
#define TEE_ATTR_DSA_SUBPRIME                0xD0001131 // q
#define TEE_ATTR_DSA_BASE                    0xD0001231 // g
#define TEE_ATTR_DSA_PUBLIC_VALUE            0xD0000131 // y
#define TEE_ATTR_DSA_PRIVATE_VALUE           0xC0000231 // x
#define TEE_ATTR_DH_PRIME                    0xD0001032 // p
#define TEE_ATTR_DH_SUBPRIME                 0xD0001132 // q
#define TEE_ATTR_DH_BASE                     0xD0001232 // g
#define TEE_ATTR_DH_X_BITS                   0xF0001332 // type "Value" not "Ref"
#define TEE_ATTR_DH_PUBLIC_VALUE             0xD0000132 // y
#define TEE_ATTR_DH_PRIVATE_VALUE            0xC0000232 // x
#define TEE_ATTR_RSA_OAEP_LABEL              0xD0000930
#define TEE_ATTR_RSA_PSS_SALT_LENGTH         0xF0000A30 // type "Value" not "Ref"

/*------------------------------------------------------------------------------
      Trusted Application Interface
 ------------------------------------------------------------------------------*/

TEE_Result TA_EXPORT TA_CreateEntryPoint(void);

void TA_EXPORT TA_DestroyEntryPoint(void);

TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(uint32_t    nParamTypes,
                                              TEE_Param   pParams[4],
                                              void**      ppSessionContext);

void TA_EXPORT TA_CloseSessionEntryPoint(void*       pSessionContext);

TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(
                                                void*       pSessionContext,
                                                uint32_t    nCommandID,
                                                uint32_t    nParamTypes,
                                                TEE_Param   pParams[4]
                                               );


/*------------------------------------------------------------------------------
      Trusted Core Framework APIs
 ------------------------------------------------------------------------------*/

void TEE_EXPORT TEE_Panic(TEE_Result nPanicCode);


/*------------------------------------------------------------------------------
      Property Access Functions
 ------------------------------------------------------------------------------*/

TEE_Result TEE_EXPORT TEE_GetPropertyAsString(
   TEE_PropSetHandle propsetOrEnumerator,
   char* name,
   char* valueBuffer,
    size_t* valueBufferLen );

TEE_Result TEE_EXPORT TEE_GetPropertyAsBool(
   TEE_PropSetHandle propsetOrEnumerator,
   char* name,
   bool* value );

TEE_Result TEE_EXPORT TEE_GetPropertyAsU32 (
   TEE_PropSetHandle propsetOrEnumerator,
   char* name,
   uint32_t* value );

TEE_Result TEE_EXPORT TEE_GetPropertyAsBinaryBlock(
   TEE_PropSetHandle propsetOrEnumerator,
   char* name,
   void* valueBuffer,
   size_t* valueBufferLen );

TEE_Result TEE_EXPORT TEE_GetPropertyAsUUID(
   TEE_PropSetHandle propsetOrEnumerator,
   char* name,
   TEE_UUID* value );

TEE_Result TEE_EXPORT TEE_GetPropertyAsIdentity(
   TEE_PropSetHandle propsetOrEnumerator,
   char* name,
   TEE_Identity* value );

TEE_Result TEE_EXPORT TEE_AllocatePropertyEnumerator(
   TEE_PropSetHandle* enumerator );

void TEE_EXPORT TEE_FreePropertyEnumerator(
   TEE_PropSetHandle enumerator );

void TEE_EXPORT TEE_StartPropertyEnumerator(
   TEE_PropSetHandle enumerator,
   TEE_PropSetHandle propSet );

void TEE_EXPORT TEE_ResetPropertyEnumerator(
   TEE_PropSetHandle enumerator );

TEE_Result TEE_EXPORT TEE_GetPropertyName(
   TEE_PropSetHandle enumerator,
   void* nameBuffer,
   size_t* nameBufferLen );

TEE_Result TEE_EXPORT TEE_GetNextProperty(
   TEE_PropSetHandle enumerator );


/*------------------------------------------------------------------------------
      Internal Client API
 ------------------------------------------------------------------------------*/

TEE_Result TEE_EXPORT TEE_OpenTASession( TEE_UUID* destination, // [in]
                                         uint32_t cancellationRequestTimeout,
                                         uint32_t paramTypes,
                                         TEE_Param params[4], // [inout]
                                         TEE_TASessionHandle* session, // [out]
                                         uint32_t* returnOrigin
                                       );

void TEE_EXPORT TEE_CloseTASession(TEE_TASessionHandle session);

TEE_Result TEE_EXPORT TEE_InvokeTACommand(TEE_TASessionHandle session,
                                          uint32_t cancellationRequestTimeout,
                                          uint32_t commandID,
                                          uint32_t paramTypes,
                                          TEE_Param params[4],
                                          uint32_t* returnOrigin
                                         );

/*------------------------------------------------------------------------------
      Cancellation Functions
 ------------------------------------------------------------------------------*/

bool TEE_EXPORT TEE_GetCancellationFlag( void );
bool TEE_EXPORT TEE_UnmaskCancellation( void );
bool TEE_EXPORT TEE_MaskCancellation( void );


/*------------------------------------------------------------------------------
      Memory Management Functions
 ------------------------------------------------------------------------------*/

TEE_Result TEE_EXPORT TEE_CheckMemoryAccessRights(uint32_t accessFlags,
                                                 void* buffer, size_t size
                                                );

void TEE_EXPORT TEE_SetInstanceData( void* instanceData );

TEE_EXPORT void* TEE_GetInstanceData( void );

/**
 * Allocates a block of memory from a heap. The address of the allocated block
 * is aligned on a 8-bytes boundary. A block allocated by {malloc} must
 * be freed by {free}.
 *
 * @param   nSize    Number of bytes to be allocated. A zero value is invalid.
 *
 * @return  A pointer to the allocated memory block or
 *          NULL if the block cannot be allocated.
 **/
TEE_EXPORT void* TEE_Malloc(uint32_t size, uint32_t hint);

/**
 * Reallocates a block of memory from a heap.
 * This function enables you to resize a memory block.
 *
 * If <tt>pBlock</tt> is NULL, {realloc} is equivalent to {malloc}.
 * In particular, if nNewSize is 0, the function returns NULL.
 *
 * If <tt>pBlock</tt> is not NULL and <tt>nNewSize</tt> is 0, then
 * {realloc} is equivalent to {free} and returns NULL.
 *
 * If <tt>nNewSize</tt> is less or equal to the current size of the block,
 * the block is trucated, the content of the block is left unchanged and
 * the function returns <tt>pBlock</tt>.
 *
 * If <tt>nNewSize</tt> is greater than the current size of the block, the size
 * of the block is increased. The whole content of the block is copied at the
 * beginning of the new block. If possible, the block is enlarged in place and
 * the function retuns <tt>pBlock</tt>. If this is not possible, a new block
 * is allocated with the new size, the content of the current block is copied,
 * the current block is freed and the function retuns the pointer on the new
 * block.
 * @param   pBlock   Pointer to the block of memory that the function
 *                   reallocates. This value may be null or returned by an
 *                   earlier call to the {malloc} or {realloc}
 *                   function.
 *
 * @param   nNewSize New size of the memory block, in bytes.
 *                   This value may be zero. A memory block's size can be
 *                   increased or decreased by using this function.
 *
 * @return  A pointer to the reallocated memory block or
 *          NULL if <tt>nNewSize</tt> is zero or if an error is detected.
 **/
TEE_EXPORT void* TEE_Realloc(void* buffer, uint32_t newSize);

/**
 * Frees a memory block allocated from a heap by the {malloc} or
 * {realloc} function.
 *
 * This function does nothing if pBlock is set to NULL.
 *
 * @param   pBlock   Pointer to the memory block to be freed.
 *                   This pointer is returned by an earlier call
 *                   to the {malloc} or {realloc} function.
 **/
void TEE_EXPORT TEE_Free(void *buffer);

void TEE_EXPORT TEE_MemMove(void* dest, void* src, uint32_t size);

int32_t TEE_EXPORT TEE_MemCompare(void* buffer1, void* buffer2, uint32_t size);

void TEE_EXPORT TEE_MemFill(void* buffer, uint32_t x, uint32_t size );


/*------------------------------------------------------------------------------
      Time Functions
 ------------------------------------------------------------------------------*/

TEE_Result TEE_EXPORT TEE_Wait(uint32_t timeout);

void TEE_EXPORT TEE_GetSystemTime(TEE_Time* time);

void TEE_EXPORT TEE_GetREETime(TEE_Time* time);

TEE_Result TEE_EXPORT TEE_GetTAPersistentTime(TEE_Time* time);

TEE_Result TEE_EXPORT TEE_SetTAPersistentTime(TEE_Time* time);


/*------------------------------------------------------------------------------
      Arithmetical Functions
------------------------------------------------------------------------------*/

typedef uint32_t TEE_BigInt;
typedef uint32_t TEE_BigIntFMMContext;
typedef uint32_t TEE_BigIntFMM;

#define TEE_BigIntSizeInU32(n) ((((n)+31)/32)+2)

void TEE_EXPORT TEE_BigIntInit(
   OUT TEE_BigInt *bigInt,
   IN  size_t len);

int32_t TEE_EXPORT TEE_BigIntCmp(
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2);

int32_t TEE_EXPORT TEE_BigIntCmpS32(
   IN TEE_BigInt *op,
   IN int32_t shortVal);

TEE_Result TEE_EXPORT TEE_BigIntConvertFromOctetString(
   OUT TEE_BigInt *dest,
   IN  uint8_t *buffer,
   IN  size_t  bufferLen,
   IN int32_t sign);

TEE_Result TEE_EXPORT TEE_BigIntConvertToOctetString(
   OUT void *buffer,
   OUT size_t *bufferLen,
   IN TEE_BigInt *bigInt);

void TEE_EXPORT TEE_BigIntConvertFromS32(
   OUT TEE_BigInt *dest,
   IN int32_t shortVal);

TEE_Result TEE_EXPORT TEE_BigIntConvertToS32(
   OUT int32_t *dest,
   IN TEE_BigInt *src);

void TEE_EXPORT TEE_BigIntShiftRight(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op,
   IN size_t bits);

bool TEE_EXPORT TEE_BigIntGetBit(
   IN TEE_BigInt *src,
   IN uint32_t bitIndex);

uint32_t TEE_EXPORT TEE_BigIntGetBitCount(
   IN TEE_BigInt *src);

void TEE_EXPORT TEE_BigIntAdd(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2);

void TEE_EXPORT TEE_BigIntSub(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2);

size_t TEE_EXPORT TEE_BigIntFMMContextSizeInU32(
   size_t modulusSizeInBits);

size_t TEE_EXPORT TEE_BigIntFMMSizeInU32(
   size_t modulusSizeInBits);

void TEE_EXPORT TEE_BigIntInitFMMContext(
   OUT TEE_BigIntFMMContext *context,
   size_t len,
   IN TEE_BigInt *modulus);

void TEE_EXPORT TEE_BigIntInitFMM(
   OUT TEE_BigIntFMM *bigIntFMM,
   size_t len);

void TEE_EXPORT TEE_BigIntNeg(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op);

void TEE_EXPORT TEE_BigIntMul(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2);

void TEE_EXPORT TEE_BigIntSquare(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op);

void TEE_EXPORT TEE_BigIntDiv(
   OUT TEE_BigInt *dest_q,
   OUT TEE_BigInt *dest_r,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2);

void TEE_EXPORT TEE_BigIntMod(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op,
   IN TEE_BigInt *n);

void TEE_EXPORT TEE_BigIntAddMod(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2,
   IN TEE_BigInt *n);

void TEE_EXPORT TEE_BigIntSubMod(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2,
   IN TEE_BigInt *n);

void TEE_EXPORT TEE_BigIntMulMod(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2,
   IN TEE_BigInt *n);

void TEE_EXPORT TEE_BigIntSquareMod(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op,
   IN TEE_BigInt *n);

void TEE_EXPORT TEE_BigIntInvMod(
   OUT TEE_BigInt *dest,
   IN TEE_BigInt *op,
   IN TEE_BigInt *n);

bool TEE_EXPORT TEE_BigIntRelativePrime(
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2);

void TEE_EXPORT TEE_BigIntComputeExtendedGcd(
   OUT TEE_BigInt *gcd,
   OUT TEE_BigInt *u,
   OUT TEE_BigInt *v,
   IN TEE_BigInt *op1,
   IN TEE_BigInt *op2);

int32_t TEE_EXPORT TEE_BigIntIsProbablePrime(
   IN TEE_BigInt *op,
   uint32_t confidenceLevel);

void TEE_EXPORT TEE_BigIntConvertToFMM(
   OUT TEE_BigIntFMM *dest,
   IN TEE_BigInt *src,
   IN TEE_BigInt *n,
   IN TEE_BigIntFMMContext *context);

void TEE_EXPORT TEE_BigIntConvertFromFMM(
   OUT TEE_BigInt *dest,
   IN TEE_BigIntFMM *src,
   IN TEE_BigInt *n,
   IN TEE_BigIntFMMContext *context);

void TEE_EXPORT TEE_BigIntComputeFMM(
   OUT TEE_BigIntFMM *dest,
   IN TEE_BigIntFMM *op1,
   IN TEE_BigIntFMM *op2,
   IN TEE_BigInt *n,
   IN TEE_BigIntFMMContext *context);

 /*------------------------------------------------------------------------------
      Cryptographic Operations API
 ------------------------------------------------------------------------------*/

 /*------------------------------------------------------------------------------
      Generic Operations Functions
 ------------------------------------------------------------------------------*/
TEE_Result TEE_EXPORT TEE_AllocateOperation(TEE_OperationHandle* operation,
                                            uint32_t             algorithm,
                                            uint32_t             mode,
                                            uint32_t             maxKeySize);

void TEE_EXPORT TEE_FreeOperation(TEE_OperationHandle operation);

void TEE_EXPORT TEE_GetOperationInfo(TEE_OperationHandle operation,
                                     TEE_OperationInfo*  operationInfo);


void TEE_EXPORT TEE_ResetOperation(TEE_OperationHandle operation);

TEE_Result TEE_EXPORT TEE_SetOperationKey(TEE_OperationHandle operation,
                                          TEE_ObjectHandle    key);

TEE_Result TEE_EXPORT TEE_SetOperationKey2(TEE_OperationHandle operation,
                                           TEE_ObjectHandle    key1,
                                           TEE_ObjectHandle    key2);

void TEE_EXPORT TEE_CopyOperation(TEE_OperationHandle dstOperation,
                                  TEE_OperationHandle srcOperation);


/* Message Digest Functions                                                  */
void TEE_EXPORT TEE_DigestUpdate(TEE_OperationHandle operation,
                                 void*               chunk,
                                 size_t              chunkSize);

TEE_Result TEE_EXPORT TEE_DigestDoFinal(TEE_OperationHandle operation,
                                        void*               chunk,
                                        size_t              chunkLen,
                                        void*               hash,
                                        size_t*             hashLen);

/* Symmetric Cipher Functions                                                 */
void TEE_EXPORT TEE_CipherInit(TEE_OperationHandle operation,
                               void*               IV,
                               size_t              IVLen);

TEE_Result TEE_EXPORT TEE_CipherUpdate(TEE_OperationHandle operation,
                                       void*               srcData,
                                       size_t              srcLen,
                                       void*               destData,
                                       size_t*             destLen);

TEE_Result TEE_EXPORT TEE_CipherDoFinal(TEE_OperationHandle operation,
                                        void*               srcData,
                                        size_t              srcLen,
                                        void*               destData,
                                        size_t*             destLen);

/*----------------------------------------------------------------------------
  MAC Functions
-----------------------------------------------------------------------------*/
void TEE_EXPORT TEE_MACInit(TEE_OperationHandle operation,
                            void*               IV,
                            size_t              IVLen);

void TEE_EXPORT TEE_MACUpdate(TEE_OperationHandle operation,
                              void*               chunk,
                              size_t              chunkSize);

TEE_Result TEE_EXPORT TEE_MACComputeFinal(TEE_OperationHandle operation,
                                          void*               message,
                                          size_t              messageLen,
                                          void*               mac,
                                          size_t*             macLen);

TEE_Result TEE_EXPORT TEE_MACCompareFinal(TEE_OperationHandle operation,
                                          void*               message,
                                          size_t              messageLen,
                                          void*               mac,
                                          size_t              macLen);

/*----------------------------------------------------------------------------
   Authenticated Encryption Functions ----------------------------------------
  ---------------------------------------------------------------------------*/
TEE_Result TEE_EXPORT TEE_AEInit(TEE_OperationHandle operation,
                                 void*               nonce,
                                 size_t              nonceLen,
                                 uint32_t            tagLen,
                                 uint32_t            AADLen,
                                 uint32_t            payloadLen);

void TEE_EXPORT TEE_AEUpdateAAD(TEE_OperationHandle operation,
                                void*               AADdata,
                                size_t              AADdataLen);

TEE_Result TEE_EXPORT TEE_AEUpdate(TEE_OperationHandle operation,
                                   void*               srcData,
                                   size_t              srcLen,
                                   void*               destData,
                                   size_t*             destLen);

 TEE_Result TEE_EXPORT TEE_AEEncryptFinal(TEE_OperationHandle operation,
                                          void*               srcData,
                                          size_t              srcLen,
                                          void*               destData,
                                          size_t*             destLen,
                                          void*               tag,
                                          size_t*             tagLen);

TEE_Result TEE_EXPORT TEE_AEDecryptFinal(TEE_OperationHandle operation,
                                         void*               srcData,
                                         size_t              srcLen,
                                         void*               destData,
                                         size_t*             destLen,
                                         void*               tag,
                                         size_t              tagLen);

/*-----------------------------------------------------------------------------
 Asymmetric Functions
-----------------------------------------------------------------------------*/
TEE_Result TEE_EXPORT TEE_AsymmetricEncrypt(TEE_OperationHandle operation,
                                            TEE_Attribute*      params,
                                            uint32_t            paramCount,
                                            void*               srcData,
                                            size_t              srcLen,
                                            void*               destData,
                                            size_t*             destLen);

TEE_Result TEE_EXPORT TEE_AsymmetricDecrypt(TEE_OperationHandle operation,
                                            TEE_Attribute*      params,
                                            uint32_t            paramCount,
                                            void*               srcData,
                                            size_t              srcLen,
                                            void*               destData,
                                            size_t*             destLen);

TEE_Result TEE_EXPORT TEE_AsymmetricSignDigest(TEE_OperationHandle operation,
                                               TEE_Attribute*      params,
                                               uint32_t            paramCount,
                                               void*               digest,
                                               size_t              digestLen,
                                               void*               signature,
                                               size_t*             signatureLen);

TEE_Result TEE_EXPORT TEE_AsymmetricVerifyDigest(TEE_OperationHandle operation,
                                                 TEE_Attribute*      params,
                                                 uint32_t            paramCount,
                                                 void*               digest,
                                                 size_t              digestLen,
                                                 void*               signature,
                                                 size_t              signatureLen);

/*-----------------------------------------------------------------------------
 Key Derivation Functions
-----------------------------------------------------------------------------*/
void TEE_EXPORT TEE_DeriveKey(TEE_OperationHandle operation,
                              TEE_Attribute*      params,
                              uint32_t            paramCount,
                              TEE_ObjectHandle    derivedKey);


/*-----------------------------------------------------------------------------
 Random Data Generation Function
-----------------------------------------------------------------------------*/
void TEE_EXPORT TEE_GenerateRandom(void*  randomBuffer,
                                   size_t randomBufferLen);


/*------------------------------------------------------------------------------
      Trusted Storage Functions
------------------------------------------------------------------------------*/

/* Generic Object Functions */
void TEE_EXPORT TEE_GetObjectInfo(TEE_ObjectHandle object,
                                  TEE_ObjectInfo*  objectInfo);

void TEE_EXPORT TEE_RestrictObjectUsage(TEE_ObjectHandle object,
                                        uint32_t         objectUsage);

TEE_Result TEE_EXPORT TEE_GetObjectBufferAttribute(TEE_ObjectHandle object,
                                                   uint32_t         attributeID,
                                                   void*            buffer,
                                                   size_t*          size);

TEE_Result TEE_EXPORT TEE_GetObjectValueAttribute(TEE_ObjectHandle object,
                                                  uint32_t         attributeID,
                                                  uint32_t*        a,
                                                  uint32_t*        b);

void TEE_EXPORT TEE_CloseObject(TEE_ObjectHandle object);

/* Transient Object Functions */
TEE_Result TEE_EXPORT TEE_AllocateTransientObject(uint32_t          objectType,
                                                  uint32_t          maxObjectSize,
                                                  TEE_ObjectHandle* object);

void TEE_EXPORT TEE_FreeTransientObject(TEE_ObjectHandle object);

void TEE_EXPORT TEE_ResetTransientObject(TEE_ObjectHandle object);

TEE_Result TEE_EXPORT TEE_PopulateTransientObject(TEE_ObjectHandle object,
                                                  TEE_Attribute*   attrs,
                                                  uint32_t         attrCount);

void TEE_EXPORT TEE_InitRefAttribute(TEE_Attribute* attr,
                                     uint32_t       attributeID,
                                     void*          buffer,
                                     size_t         length);

void TEE_EXPORT TEE_InitValueAttribute(TEE_Attribute* attr,
                                       uint32_t       attributeID,
                                       uint32_t       a,
                                       uint32_t       b);

void TEE_EXPORT TEE_CopyObjectAttributes(TEE_ObjectHandle destObject,
                                         TEE_ObjectHandle srcObject);

TEE_Result TEE_EXPORT TEE_GenerateKey(TEE_ObjectHandle object,
                                      uint32_t         keySize,
                                      TEE_Attribute*   params,
                                      uint32_t         paramCount);

/* Persistant Object Functions */
TEE_Result TEE_EXPORT TEE_CreatePersistentObject(uint32_t          storageID,
                                                 void*             objectID,
                                                 size_t            objectIDLen,
                                                 uint32_t          flags,
                                                 TEE_ObjectHandle  attributes,
                                                 void*             initialData,
                                                 size_t            initialDataLen,
                                                 TEE_ObjectHandle* object);

TEE_Result TEE_EXPORT TEE_OpenPersistentObject(uint32_t          storageID,
                                               void*             objectID,
                                               size_t            objectIDLen,
                                               uint32_t          flags,
                                               TEE_ObjectHandle* object);

void TEE_EXPORT TEE_CloseAndDeletePersistentObject(TEE_ObjectHandle object);

TEE_Result TEE_EXPORT TEE_RenamePersistentObject(TEE_ObjectHandle object,
                                                 void*            newObjectID,
                                                 size_t           newObjectIDLen);

TEE_Result TEE_EXPORT TEE_AllocatePersistentObjectEnumerator(TEE_ObjectEnumHandle* objectEnumerator);

void TEE_EXPORT TEE_FreePersistentObjectEnumerator(TEE_ObjectEnumHandle objectEnumerator);

void TEE_EXPORT TEE_ResetPersistentObjectEnumerator(TEE_ObjectEnumHandle objectEnumerator);

TEE_Result TEE_EXPORT TEE_StartPersistentObjectEnumerator(TEE_ObjectEnumHandle objectEnumerator,
                                                          uint32_t             storageID);

TEE_Result TEE_EXPORT TEE_GetNextPersistentObject(TEE_ObjectEnumHandle objectEnumerator,
                                                  TEE_ObjectInfo*      objectInfo,
                                                  void*                objectID,
                                                  size_t*              objectIDLen);

TEE_Result TEE_EXPORT TEE_ReadObjectData(TEE_ObjectHandle object,
                                         void*            buffer,
                                         size_t           size,
                                         uint32_t*        count);

TEE_Result TEE_EXPORT TEE_WriteObjectData(TEE_ObjectHandle object,
                                          void*            buffer,
                                          size_t           size);

TEE_Result TEE_EXPORT TEE_TruncateObjectData(TEE_ObjectHandle object,
                                             uint32_t         size);

TEE_Result TEE_EXPORT TEE_SeekObjectData(TEE_ObjectHandle object,
                                         int32_t          offset,
                                         TEE_Whence       whence);

#endif /* __TEE_INTERNAL_API_H__ */
