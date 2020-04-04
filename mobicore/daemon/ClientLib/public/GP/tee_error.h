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

#ifndef __TEE_ERROR_H__
#define __TEE_ERROR_H__

#define TEE_SUCCESS                      ((TEE_Result)0x00000000)
#define TEEC_SUCCESS                   TEE_SUCCESS

#define TEE_ERROR_CORRUPT_OBJECT         ((TEE_Result)0xF0100001)
#define TEE_ERROR_CORRUPT_OBJECT_2       ((TEE_Result)0xF0100002)

/**
 * Generic error code : Generic error
 **/
#define TEE_ERROR_GENERIC                ((TEE_Result)0xFFFF0000)
#define TEEC_ERROR_GENERIC             TEE_ERROR_GENERIC

/**
 * Generic error code : The underlying security system denies the access to the
 * object
 **/
#define TEE_ERROR_ACCESS_DENIED          ((TEE_Result)0xFFFF0001)
#define TEEC_ERROR_ACCESS_DENIED       TEE_ERROR_ACCESS_DENIED

/**
 * Generic error code : The pending operation is cancelled.
 **/
#define TEE_ERROR_CANCEL                 ((TEE_Result)0xFFFF0002)
#define TEEC_ERROR_CANCEL              TEE_ERROR_CANCEL

/**
 * Generic error code : The underlying system detects a conflict
 **/
#define TEE_ERROR_ACCESS_CONFLICT        ((TEE_Result)0xFFFF0003)
#define TEEC_ERROR_ACCESS_CONFLICT     TEE_ERROR_ACCESS_CONFLICT

/**
 * Generic error code : Too much data for the operation or some data remain
 * unprocessed by the operation.
 **/
#define TEE_ERROR_EXCESS_DATA            ((TEE_Result)0xFFFF0004)
#define TEEC_ERROR_EXCESS_DATA         TEE_ERROR_EXCESS_DATA

/**
 * Generic error code : Error of data format
 **/
#define TEE_ERROR_BAD_FORMAT             ((TEE_Result)0xFFFF0005)
#define TEEC_ERROR_BAD_FORMAT          TEE_ERROR_BAD_FORMAT

/**
 * Generic error code : The specified parameters are invalid
 **/
#define TEE_ERROR_BAD_PARAMETERS         ((TEE_Result)0xFFFF0006)
#define TEEC_ERROR_BAD_PARAMETERS      TEE_ERROR_BAD_PARAMETERS


/**
 * Generic error code : Illegal state for the operation.
 **/
#define TEE_ERROR_BAD_STATE              ((TEE_Result)0xFFFF0007)
#define TEEC_ERROR_BAD_STATE           TEE_ERROR_BAD_STATE

/**
 * Generic error code : The item is not found
 **/
#define TEE_ERROR_ITEM_NOT_FOUND         ((TEE_Result)0xFFFF0008)
#define TEEC_ERROR_ITEM_NOT_FOUND      TEE_ERROR_ITEM_NOT_FOUND

/**
 * Generic error code : The specified operation is not implemented
 **/
#define TEE_ERROR_NOT_IMPLEMENTED        ((TEE_Result)0xFFFF0009)
#define TEEC_ERROR_NOT_IMPLEMENTED     TEE_ERROR_NOT_IMPLEMENTED

/**
 * Generic error code : The specified operation is not supported
 **/
#define TEE_ERROR_NOT_SUPPORTED          ((TEE_Result)0xFFFF000A)
#define TEEC_ERROR_NOT_SUPPORTED       TEE_ERROR_NOT_SUPPORTED

/**
 * Generic error code : Insufficient data is available for the operation.
 **/
#define TEE_ERROR_NO_DATA                ((TEE_Result)0xFFFF000B)
#define TEEC_ERROR_NO_DATA             TEE_ERROR_NO_DATA

/**
 * Generic error code : Not enough memory to perform the operation
 **/
#define TEE_ERROR_OUT_OF_MEMORY          ((TEE_Result)0xFFFF000C)
#define TEEC_ERROR_OUT_OF_MEMORY       TEE_ERROR_OUT_OF_MEMORY

/**
 * Generic error code : The service is currently unable to handle the request;
 * try later
 **/
#define TEE_ERROR_BUSY                   ((TEE_Result)0xFFFF000D)
#define TEEC_ERROR_BUSY                TEE_ERROR_BUSY

/**
 * Generic communication error
 **/
#define TEE_ERROR_COMMUNICATION          ((TEE_Result)0xFFFF000E)
#define TEEC_ERROR_COMMUNICATION       TEE_ERROR_COMMUNICATION

/**
 * Generic error code : security violation
 **/
#define TEE_ERROR_SECURITY               ((TEE_Result)0xFFFF000F)
#define TEEC_ERROR_SECURITY            TEE_ERROR_SECURITY

/**
 * Generic error code : the buffer is too short
 **/
#define TEE_ERROR_SHORT_BUFFER           ((TEE_Result)0xFFFF0010)
#define TEEC_ERROR_SHORT_BUFFER        TEE_ERROR_SHORT_BUFFER

/**
 * Generic error code : The pending operation is cancelled.
 */
#define TEE_ERROR_EXTERNAL_CANCEL        ((TEE_Result)0xFFFF0011)

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

/**
 * Error of communication: The target of the connection is dead
 **/
#define TEE_ERROR_TARGET_DEAD            ((TEE_Result)0xFFFF3024)
#define TEEC_ERROR_TARGET_DEAD         TEE_ERROR_TARGET_DEAD

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

#endif /* __TEE_ERROR_H__ */

