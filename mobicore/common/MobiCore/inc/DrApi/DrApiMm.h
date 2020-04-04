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

#ifndef __DRAPIMM_H__
#define __DRAPIMM_H__

#include "DrApiCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
/** Definitions */

/** Memory mapping attributes. */
#define MAP_READABLE            (1U << 0)               /**< mapping gives  have the ability to do read access. */
#define MAP_WRITABLE            (1U << 1)               /**< mapping gives  have the ability to do write access. */
#define MAP_EXECUTABLE          (1U << 2)               /**< mapping gives  have the ability to do program execution. */
#define MAP_UNCACHED            (1U << 3)               /**< mapping gives  have uncached memory access. */
#define MAP_IO                  (1U << 4)               /**< mapping gives  have memory mapped I/O access. Will ignore MAP_UNCACHED, as this would be implied anyway. */

//------------------------------------------------------------------------------
/** Maximum number of parameter . */
#define MAX_MAR_LIST_LENGTH 	8                      /**< Maximum list of possible marshaling parameters. */
/** Marshaled union. */
typedef struct {
    uint32_t     functionId;                       /**< Function identifier. */
    union {
        uint32_t                            parameter[MAX_MAR_LIST_LENGTH];   /* untyped parameter list (expands union to 8 entries) */
    } payload;
} drApiMarshalingParam_t, *drApiMarshalingParam_ptr;


//------------------------------------------------------------------------------
/** Address translation from trustlet to Driver address space.
 * Translates an address/pointer given by a Trustlet to the Driver mapping.
 * Checks correct address range and null pointer.
 *
 * @param addr Address in trustlet address space.
 * @returns address in Driver virtual space
 * @returns NULL if address is equal to NULL or if address is out of D3-D8 address space
 */
addr_t drApiAddrTranslateAndCheck(addr_t addr);

//------------------------------------------------------------------------------
#if TBASE_API_LEVEL >= 3

/** Addresses translation for a buffer from trustlet to driver address space.
 * Checks that the buffer given by its start address and its length fits correct address range.
 * Translate the start address/pointer given by a trustlet to the driver's mapping.
 *
 * @param addr Start address of the buffer in trustlet address space.
 * @param len Length of the buffer
 * @returns start address in Driver virtual space
 * @returns NULL if address is equal to NULL or if the buffer is out of D3-D8 address space
 */
addr_t drApiAddrTranslateAndCheckBuffer(addr_t adr, uint32_t len);

#endif /* TBASE_API_LEVEL */

//------------------------------------------------------------------------------
/** Maps requesting client and return translated pointer to request parameters
 *
 * @param ipcReqClient client requesting a service
 * @param params pointer to marshaled parameter in client address space
 * @returns pointer to parameter for request in the current address space
 * @returns NULL in case of error
 */
drApiMarshalingParam_ptr drApiMapClientAndParams(
    threadid_t  ipcReqClient,
    uint32_t    params
);

//------------------------------------------------------------------------------

/** Maps a physical page to a virtual address.
 *  All addresses and lengths must be multiples of page size (4K).
 *
 * @param startVirt virtual address in Driver address space
 * @param len Length of area
 * @param startPhys physical address of hardware
 * @param attr mapping attributes
 * @returns DRAPI_OK in case of success
 * @returns E_DRAPI_INVALID_PARAMETER in case any input parameter is not page size aligned or designated virtual memory area does not fit into D1-D2 address range
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */

drApiResult_t drApiMapPhys(
    const addr_t      startVirt,
    const uint32_t    len,
    const addr_t      startPhys,
    const uint32_t    attr
);
#if TBASE_API_LEVEL >= 3
drApiResult_t drApiMapPhys64(
    const addr_t      startVirt,
    const uint32_t    len,
    const uint64_t    startPhys,
    const uint32_t    attr
);
#endif /* TBASE_API_LEVEL */

/** Removes mapping for virtual pages.
 *  All addresses and lengths must be multiples of page size (4K).
 *
 * @param startVirt virtual address in Driver address space
 * @param len Length of area
 * @returns DRAPI_OK in case of success
 * @returns E_DRAPI_INVALID_PARAMETER in case any input parameter is not page size aligned or designated virtual memory area does not fit into D1-D2 address range
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */

drApiResult_t drApiUnmap(
    const addr_t    startVirt,
    const uint32_t  len
);

//------------------------------------------------------------------------------
/** The function removes mapping for a single page.
 *
 * @param startVirt virtual address in Driver address space
 * @returns DRAPI_OK in case of success
 * @returns E_DRAPI_INVALID_PARAMETER in case any input parameter is not page size aligned or designated virtual memory area does not fit into D1-D2 address range
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiUnmapPage4KB(
    const page4KB_ptr   virtPage
);

//------------------------------------------------------------------------------
/** Maps a single physical page to a virtual address
 *
 * @param physPage virtual address in Driver address space
 * @param startPhys physical address of hardware
 * @param attr mapping attributes
 * @returns DRAPI_OK in case of success
 * @returns E_DRAPI_INVALID_PARAMETER in case any input parameter is not page size aligned or designated virtual memory area does not fit into D1-D2 address range
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiMapPhysPage4KB(
    const page4KB_ptr   virtPage,
    const page4KB_ptr   physPage,
    const uint32_t      attr
);
#if TBASE_API_LEVEL >= 3
drApiResult_t drApiMapPhysPage4KB64(
    const page4KB_ptr   virtPage,
    const uint64_t      physPage,
    const uint32_t      attr
);
#endif /* TBASE_API_LEVEL */

//------------------------------------------------------------------------------
/** Maps a physical page with hardware interface
 *  This is prepared auxiliary function that at first removes mapping of the virtPage (if present)
 *  and then maps it with MAP_READABLE | MAP_WRITABLE | MAP_IO attributes
 *
 * @param physPage virtual address in Driver address space
 * @param startPhys physical address of hardware
 * @returns DRAPI_OK in case of success
 * @returns E_DRAPI_INVALID_PARAMETER in case any input parameter is not page size aligned or designated virtual memory area does not fit into D1-D2 address range
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiMapPhysPage4KBWithHardware(
    const page4KB_ptr   virtPage,
    const page4KB_ptr   physPage
);
#if TBASE_API_LEVEL >= 3
drApiResult_t drApiMapPhysPage4KBWithHardware64(
    const page4KB_ptr   virtPage,
    const uint64_t      physPage
);
#endif /* TBASE_API_LEVEL */

//------------------------------------------------------------------------------
/** Converts virtual address (in Driver address space) to physical address
 *
 * @param taskid   Reserved for Future Use. It must be set to zero.
 * @param virtAddr virtual address in Driver address space
 * @param physAddr physical address
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiVirt2Phys(
        const taskid_t      taskid,
        const addr_t        virtAddr,
        addr_t *            physAddr
);
#if TBASE_API_LEVEL >= 3
drApiResult_t drApiVirt2Phys64(
        const taskid_t      taskid,
        const addr_t        virtAddr,
        uint64_t *          physAddr
);
#endif /* TBASE_API_LEVEL */

//------------------------------------------------------------------------------
/** Clean all data cache
 *
 * NOTE: Currently addr is always NULL.
 * When extending this for not-clean-all,  one can assume that there is
 * more parameters ONLY if addr is non-NULL.
 * Also this ensures that more specific cleans are mapped to cleanall, if specific clean is not supported.
 *
 * @param none
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiCacheDataCleanAll( void );


//------------------------------------------------------------------------------
/** Cleans and invalidates all data cache
 *
 * NOTE: Currently addr is always NULL.
 * When extending this for not-clean-invalidate-all,  one can assume that there is
 * more parameters ONLY if addr is non-NULL.
 * Also this ensures that more specific cleans are mapped to cleanall, if specific clean is not supported.
 *
 * @param none
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiCacheDataCleanInvalidateAll( void );


//------------------------------------------------------------------------------
#if TBASE_API_LEVEL >= 3

/** Clean a portion of the data cache by MVA
 *
 * @param virtAddrStart
 * @param virtAddrEnd
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiCacheDataCleanRange(
    addr_t *virtAddrStart,
    uint32_t len,
    uint32_t flags
);


//------------------------------------------------------------------------------
/** Cleans and invalidates a portion of data cache by MVA
 *
 * @param virtAddrStart
 * @param virtAddrEnd
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiCacheDataCleanInvalidateRange(
    addr_t *virtAddrStart,
    uint32_t len,
    uint32_t flags
);

//------------------------------------------------------------------------------
/** Cache clean attributes */
#define DRAPI_CACHE_ALL        (0xff)
#define DRAPI_CACHE_L1_ONLY    (0x1)
#define DRAPI_CACHE_L1_L2      (0x2)
#endif /* TBASE_API_LEVEL */

//------------------------------------------------------------------------------
/** Memory type attributes */
#define DRAPI_PHYS_MEM_TYPE_HIGH_SECURE   (1U<<0)                  /**< High secure memory. (Typically iRam) */
#define DRAPI_PHYS_MEM_TYPE_SECURE        (1U<<1)                  /**< Secure memory in Dram */
#define DRAPI_PHYS_MEM_TYPE_NON_SECURE    (1U<<2)                  /**< NonSecure memory in Dram. Accessible from NonSecure world */

/** Returns physical memory type (secure or non-secure)
 *
 * @param type pointer to address where type is returned
 * @param addr start address of checked memory
 * @param size size checked memory
 * @returns E_OK in case of success
 */

drApiResult_t drApiGetPhysMemType(
        uint32_t *type,
        addr_t addr,
        uint32_t size
        );
#if TBASE_API_LEVEL >= 3
drApiResult_t drApiGetPhysMemType64(
        uint32_t *type,
        uint64_t addr,
        uint32_t size
        );
#endif /* TBASE_API_LEVEL */

#ifdef __cplusplus
}
#endif

#endif // __DRAPIMM_H__
