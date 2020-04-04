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

#ifndef FASTCALL_H_
#define FASTCALL_H_

/** Firmware Driver and Fastcall Handlers
 *
 * Mobicore can have one firmware driver. Such driver is loaded
 * typically at boot, and cannot be unloaded. Firmware driver is
 * intended to act as system integration means, and is not intended
 * to act as typical peripheral driver.
 *
 * Fastcall handlers
 * Firmware driver can register fastcall handler in some platforms.
 * Fastcall handler is called for fastcalls unknown to Mobicore.
 *
 * Fastcall handlers get driver memory mappings in range of 0-2MB
 * at the time handler is installed. Driver must not unmap any of
 * these mappings. If new mappings are made, they cannot be relied
 * to be visible in fastcall hook function.
 *
 * Fastcall handler may map sections (of size 1M). These mappings are not
 * visible to driver.
 *
 * Fastcall handler gets access to number of registers at the time of
 * fastcall was made in NWd. Actual number of registers depends on
 * platform, but is always at least 4 (r0-r3).
 *
 * Notes on use of Fastcall handlers
 * Fastcall handlers
 * - cannot call any TlApi or DrApi functions.
 * - cannot have synchronization with firmware driver.
 * - do not have large stack. If stack is needed, switch to your own.
 * - may be executed concurrently in all CPUs.
 * - must not cause exceptions. There is no means to recover in case
 *   of exception.
 *
 */

#define FASTCALL_OWNER_TZOS (0xFF000000)
#define FASTCALL_OWNER_SIP  (0x81000000)
#define FASTCALL_OWNER_OEM  (0x83000000)
#define FASTCALL_OWNER_STD  (0x84000000)

#define FASTCALL_OWNER_MASK (0xFF000000)

#define FCH_L1_MAX  12

typedef word_t *fastcall_registers_t;

/** Context for FCs hook call
 *
 * This is (currently) shared between all processors
 */
struct fcContext {
    word_t size; // Size of context
    // Callback to modify L1 entry.
    // idx is the address in MBytes of the entry to modify (starting at 8MB).
    //     (for example idx=2 will modify the entry for address 10MB)
    // Return the virtual address corresponding to modified entry.
    // Return NULL if there is an error.
    // 64 bits version must be used to modify L1 table in LPAE mode.
    void *(*setL1Entry)(struct fcContext *context, word_t idx, word_t entry);
    word_t registers; // Number of registers available in fastcall
    void (*prepareIdenticalMapping)(struct fcContext *context, addr_t start, word_t length, word_t flags);
    void (*generateFcNotification)(struct fcContext *context);
    void *(*setL1Entry64)(struct fcContext *context, word_t idx, uint64_t entry);
};

/* Fastcall handler initialization
 *
 * Called once before any fastcall.
 * Can map memories for fastcall context.
 * This function must never cause any exceptions.
 * Call is executed in SVC mode.
 *
 * Return value
 *      0 Successfull initialization.
 *      Other cancel hook installation
 */
typedef word_t (*fcInitHook)(
        struct fcContext *context
);


/* Fastcall handler
 *
 * Executed possibly concurrently in all CPUs.
 * This function must never cause any exceptions.
 * Call is executed in IRQ mode.
 * On Entry
 *   fastcall_regs_t[0]...fastcall_regs_t[15] contain input registers.
 * On exit
 *   fastcall_regs_t[0]...fastcall_regs_t[N] contain r0...rN.
 *   If driver did not handle fastcall, registers must be unmodified.
 *   If driver handled fastcall registers r0...r3 may be modified.
 *   Registers beyond r3 must not be modified. Result of any
 *   modification is unpredictable.
 * Return values
 *   Nonzero return value means that fastcall is handled.
 */
typedef word_t (*fcEntryHook)(
        fastcall_registers_t *regs_t,
        struct fcContext *context
);


#endif /* FASTCALL_H_ */
