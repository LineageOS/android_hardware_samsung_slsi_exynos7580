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

/**
 * @file   drSecureStorage_Api.h
 * @brief  Contains DCI command definitions and data structures
 *
 */

#ifndef __DRTEMPLATEAPI_H__
#define __DRTEMPLATEAPI_H__

#include "dci.h"
#include "tee_internal_api.h"


#define RW_DATA_SIZE 4096

/**
 * Command ID's for communication
 * FSD <--> STH
 */

#define STH_MESSAGE_TYPE_LOOK		0
#define STH_MESSAGE_TYPE_READ		1
#define STH_MESSAGE_TYPE_WRITE      2
#define STH_MESSAGE_TYPE_DELETE     3

#define CMD_ST_SYNC			5
#define NOTIFY_DCIH			6
#define NOTIFY_IPCH			7
/*... add more command ids when needed */

#define STH_PUBLIC_FILE_NAME_SIZE 20

typedef struct {
    uint32_t   status;
	uint8_t    type;
    uint8_t    reserved0;
    uint16_t   flags;
    uint32_t   payloadLen;
    TEE_UUID     uuid;
    unsigned char filename[STH_PUBLIC_FILE_NAME_SIZE];
    unsigned char payload[];
} STH_FSD_message_t;

typedef struct
{
    char header[5];
    unsigned char version;
    uint16_t cryptoLen;
    uint32_t dataLen;
}FSD_plaintext;

/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to be processed
 */
typedef struct {
    dciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process */
} cmd_t;


/**
 * Response structure
 */
typedef struct {
    dciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} rsp_t;

/**
 * DCI message data.
 */
typedef struct {
    union {
        cmd_t     command;
        rsp_t     response;
    };

    STH_FSD_message_t   sth_request;
} dciMessage_t;

/**
 * Driver UUID. Update accordingly after reserving UUID
 */
#define DRV_STH_UUID { { 0x07, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }


#endif // __DRTEMPLATEAPI_H__
