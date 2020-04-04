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

#ifndef   __TEE_INTERNAL_API_IMPL_H__
#define   __TEE_INTERNAL_API_IMPL_H__

#if TBASE_API_LEVEL >= 3

/*------------------------------------------------------------------------------
         Constants
------------------------------------------------------------------------------*/
// Property type constants
#define TEE_PROPERTY_TYPE_NONE                  0
#define TEE_PROPERTY_TYPE_BOOLEAN               1
#define TEE_PROPERTY_TYPE_UINT32                2
#define TEE_PROPERTY_TYPE_BINARY                3
#define TEE_PROPERTY_TYPE_STRING                4
#define TEE_PROPERTY_TYPE_UUID                  5

// Predefined offsets to  properties
#define TEE_PROPERTY_GPUUID_VALUE_OFFSET        20
#define TEE_PROPERTY_DATASIZE_VALUE_OFFSET      152
#define TEE_PROPERTY_STACKSIZE_VALUE_OFFSET     180

#endif /* TBASE_API_LEVEL */

#endif /* __TEE_INTERNAL_API_IMPL_H__ */
