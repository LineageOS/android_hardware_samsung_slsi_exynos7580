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

#ifndef __TEE_UUID_ATTESTATION_H__
#define __TEE_UUID_ATTESTATION_H__

#include "tee_type.h"

// Sizes of the fields of attestation structure
#define AT_MAGIC_SIZE           8
#define AT_SIZE_SIZE            sizeof(uint32_t)
#define AT_VERSION_SIZE         sizeof(uint32_t)
#define AT_UUID_SIZE            sizeof(TEE_UUID)

// Sizes of the fields used to generate signature
#define AT_TAG_SIZE             20
#define AT_SHA1_HASH_SIZE       20

// Max size of RSA modulus supported 
#define AT_MODULUS_MAX_SIZE     256
// Max size of RSA public exponent supported 
#define AT_PUBLIC_EXPO_MAX_SIZE 4

// Attestation version
#define AT_VERSION              1

// Name space ID (the UUID of the RSA OID)
const uint8_t       RSA_OID_UUID[AT_UUID_SIZE] = {0x6b, 0x8e, 0x02, 0x6b, 0x63, 0xc1, 0x5d, 0x58, 0xb0, 0x64, 0x00, 0xd3, 0x51, 0x89, 0xce, 0x65};
// Magic word
const char          MAGIC[AT_MAGIC_SIZE] = "TAUUID\0";

// Tag for signature generation
const char          TAG[AT_TAG_SIZE]="Trusted Application";

// Public key structure
typedef struct uuid_public_key {
    uint32_t    type;           // TEE TYPE RSA KEYPAIR: 0xa0000030
    uint16_t    modulus_bytes;  // Length of the modulus in bytes
    uint16_t    exponent_bytes; // Length of the exponent in bytes
    uint8_t     data[];         // Key material
} uuid_public_key;

// Attestation structure
typedef struct uuid_attestation {
    uint8_t         magic[AT_MAGIC_SIZE];   // Magic word: "TAUUID\0\0"
    uint32_t        size;                   // Attestation size (4 bytes)
    uint32_t        version;                // Version number: 1 (4 bytes)
    uint8_t         uuid[AT_UUID_SIZE];     // UUID
    uuid_public_key key;                    // Public key
} uuid_attestation;

#endif /* __TEE_UUID_ATTESTATION_H__ */
