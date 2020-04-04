/** @addtogroup MCD_MCDIMPL_DAEMON_DEV
 * @{
 * @file
 */

/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <pthread.h>
#include <assert.h>
#include "McTypes.h"

#include "DeviceScheduler.h"
#include "DeviceIrqHandler.h"
#include "ExcDevice.h"
#include "Connection.h"
#include "TrustletSession.h"

#include "MobiCoreDevice.h"
#include "Mci/mci.h"
#include "mcLoadFormat.h"


#include "log.h"
#include "public/MobiCoreDevice.h"



//------------------------------------------------------------------------------
MobiCoreDevice::MobiCoreDevice()
{
    nq = NULL;
    mcFlags = NULL;
    mcVersionInfo = NULL;
    mcFault = false;
    mciReused = false;
    mcpMessage = NULL;
}

//------------------------------------------------------------------------------
MobiCoreDevice::~MobiCoreDevice()
{
    mciReused = false;
    mcFault = false;
    delete mcVersionInfo;
    mcVersionInfo = NULL;
    mcFlags = NULL;
    nq = NULL;
}

//------------------------------------------------------------------------------
TrustletSession *MobiCoreDevice::getTrustletSession(
    uint32_t sessionId
) {
    for (trustletSessionIterator_t iterator = trustletSessions.begin();
         iterator != trustletSessions.end();
         ++iterator)
    {
        TrustletSession *session = *iterator;
        if (session->sessionId == sessionId) {
            return session;
        }
    }
    return NULL;
}


//------------------------------------------------------------------------------
Connection *MobiCoreDevice::getSessionConnection(
    uint32_t sessionId,
    notification_t *notification
) {
    Connection *con = NULL;

    TrustletSession *session = getTrustletSession(sessionId);
    if (session != NULL)
    {
        con = session->notificationConnection;
        if (con == NULL)
        {
            session->queueNotification(notification);
        }
    }

    return con;
}


//------------------------------------------------------------------------------
TrustletSession* MobiCoreDevice::findSession(
    Connection *deviceConnection,
    uint32_t sessionId
) {
    TrustletSession *session = getTrustletSession(sessionId);
    if (session == NULL)
    {
        LOG_E("no session found with id=%d", sessionId);
    }
    else
    {
        // check is connection own this session
        if (session->deviceConnection != deviceConnection)
        {
            LOG_E("connection does not own session id=%d", sessionId);
            session = NULL;
        }
    }
    return session;
}


//------------------------------------------------------------------------------
bool MobiCoreDevice::open(
    Connection *connection
) {
    // Link this device to the connection
    connection->connectionData = this;
    return true;
}

//------------------------------------------------------------------------------
// send a close session message. Used in three cases:
// 1) during init to tell SWd to invalidate this session, if it is still open
//    from a prev Daemon instance
// 2) normal session close
// 3) close all session when Daemon terminates
mcResult_t MobiCoreDevice::sendSessionCloseCmd(
    uint32_t sessionId
) {
    // Write MCP close message to buffer
    mcpMessage->cmdClose.cmdHeader.cmdId = MC_MCP_CMD_CLOSE_SESSION;
    mcpMessage->cmdClose.sessionId = sessionId;

    mcResult_t mcRet = mshNotifyAndWait();
    if (mcRet != MC_MCP_RET_OK)
    {
        LOG_E("mshNotifyAndWait failed for CLOSE_SESSION, code %d.", mcRet);
        return mcRet;
    }

    // Check if the command response ID is correct
    if ((MC_MCP_CMD_CLOSE_SESSION | FLAG_RESPONSE) != mcpMessage->rspHeader.rspId) {
        LOG_E("invalid MCP response for CLOSE_SESSION");
        return MC_DRV_ERR_DAEMON_MCI_ERROR;
    }

    // Read MC answer from MCP buffer
    mcRet = mcpMessage->rspOpen.rspHeader.result;

    return mcRet;
}

//------------------------------------------------------------------------------
// internal API to close a session
mcResult_t MobiCoreDevice::closeSessionInternal(
    TrustletSession *session
) {
    LOG_I("closing session with id=%d", session->sessionId);

    mcResult_t mcRet = sendSessionCloseCmd(session->sessionId);
    if (mcRet != MC_MCP_RET_OK) {
        LOG_E("sendSessionCloseCmd error %d", mcRet);
        return MAKE_MC_DRV_MCP_ERROR(mcRet);
    }

//    // clean session WSM
//    LOG_I("unlocking session buffers!");
//    CWsm_ptr pWsm = session->popBulkBuff();
//    while (pWsm)
//    {
//        unlockWsmL2(pWsm->handle);
//        delete pWsm;
//        pWsm = session->popBulkBuff();
//    }

    return MC_DRV_OK;
}

//------------------------------------------------------------------------------
/**
 * Close device.
 *
 * Removes all sessions to a connection. Though, clientLib rejects the closeDevice()
 * command if still sessions connected to the device, this is needed to clean up all
 * sessions if client dies.
 */
void MobiCoreDevice::close(
    Connection *connection
) {
    // static mutex is the the same for each thread. So this serializes
    // multiple connections failing at the same time.
    static CMutex mutex;
    // Enter critical section
    mutex.lock();

    // 1. Iterate through device session to find connection
    // 2. Decide what to do with open Trustlet sessions
    // 3. Remove & delete deviceSession from vector

    // iterating over the list in reverse order, as the LIFO approach may
    // avoid some quirks. The assumption is that a driver is opened before a
    // TA, so we want to terminate the TA first and then the driver. This may
    // make this a bit easier for everbody.

    trustletSessionList_t::reverse_iterator revIt = trustletSessions.rbegin();
    while (revIt != trustletSessions.rend())
    {
        TrustletSession *session = *revIt;

        // wiredness of reverse iterators
        // * is is incremented to get the next lowe element
        // * to delete something from the list, we need the "normal" iterator.
        //   This is simpy "one off" the current revIt. So we can savely use
        //   the increment below in both cases.
        revIt++;

        if (session->deviceConnection == connection)
        {
            // close session, log any error but ignore it.
            //mcResult_t mcRet = closeSessionInternal(session);
            mcResult_t mcRet = closeSession(connection, session->sessionId);
            if (mcRet != MC_MCP_RET_OK) {
                LOG_I("device closeSession failed with %d", mcRet);
            }

            // removing an element from the list it tricky. Convert the revIt
            // to a "normal" iterator. Remember here that the revIt is one off,
            // but we have done an increment above, so we are fine here. So
            // after we have the normal iterator, ude it to delete and then
            // convert the returned iterator back to a reverse iterator, which
            // we will use for the loop.
            //trustletSessionIterator_t it = revIt.base();
            //it = trustletSessions.erase(it); // delete
            //revIt =  trustletSessionList_t::reverse_iterator(it);

            // free object
            //delete session;
        }
    }

    // Leave critical section
    mutex.unlock();

    // After the trustlet is done make sure to tell the driver to cleanup
    // all the orphaned drivers
    cleanupWsmL2();



    connection->connectionData = NULL;

    // Leave critical section
    mutex.unlock();
}


//------------------------------------------------------------------------------
void MobiCoreDevice::start(void)
{
    LOG_I("Starting DeviceIrqHandler...");
    // Start the irq handling thread
    DeviceIrqHandler::start("DevIrqHandler");

    if (schedulerAvailable())
    {
        LOG_I("Starting DeviceScheduler...");
        // Start the scheduling handling thread
        DeviceScheduler::start();
    }
    else
    {
        LOG_I("No DeviceScheduler available.");
    }

    LOG_I("Starting TAExitHandler...");
    TAExitHandler::start("TAExitHandler");

    if (mciReused)
    {
        // remove all pending sessions. 20 is as good a any other number, we-
        // actually should ass a MCP message that tells SWd to invalidate all-
        // session that are there besides the MSH session.
        for (int sessionId = 2; sessionId<20; sessionId++) {
            LOG_I("invalidating session %d",sessionId);
            mcResult_t mcRet = sendSessionCloseCmd(sessionId);
            if (mcRet != MC_MCP_RET_OK) {
                LOG_I("sendSessionCloseCmd error %d", mcRet);
            }
        }
    }

    return;
}


//------------------------------------------------------------------------------
void MobiCoreDevice::signalMcpNotification(void)
{
    mcpSessionNotification.signal();
}


//------------------------------------------------------------------------------
bool MobiCoreDevice::waitMcpNotification(void)
{
    int counter = 5; // retry 5 times
    while (1)
	{
        // In case of fault just return, nothing to do here
        if (mcFault)
		{
            return false;
        }
        // Wait 10 seconds for notification
        if ( mcpSessionNotification.wait(10) )
		{
			break; // seem we got one.
        }
        // No MCP answer received and mobicore halted, dump mobicore status
        // then throw exception
        LOG_I("No MCP answer received in 2 seconds.");
        if (getMobicoreStatus() == MC_STATUS_HALT)
		{
            dumpMobicoreStatus();
            mcFault = true;
            return false;
        }

        counter--;
        if (counter < 1)
		{
            mcFault = true;
            return false;
        }
    } // while(1)

    // Check healthiness state of the device
    if (DeviceIrqHandler::isExiting()) {
        LOG_I("waitMcpNotification(): IrqHandler thread died! Joining");
        DeviceIrqHandler::join();
        LOG_I("waitMcpNotification(): Joined");
        LOG_E("IrqHandler thread died!");
        return false;
    }

    if (DeviceScheduler::isExiting()) {
        LOG_I("waitMcpNotification(): Scheduler thread died! Joining");
        DeviceScheduler::join();
        LOG_I("waitMcpNotification(): Joined");
        LOG_E("Scheduler thread died!");
        return false;
    }

    if (TAExitHandler::isExiting()) {
        LOG_I("waitMcpNotification(): TAExitHandler thread died! Joining");
        TAExitHandler::join();
        LOG_I("waitMcpNotification(): Joined");
        LOG_E("TAExitHandler thread died!");
        return false;
    }
    return true;
}


//------------------------------------------------------------------------------
mcResult_t MobiCoreDevice::mshNotifyAndWait()
{
    // Notify MC about the availability of a new command inside the MCP buffer
    notify(SID_MCP);

    // Wait till response from MSH is available
    if (!waitMcpNotification())
    {
        LOG_E("waiting for MCP notification failed");
        return MC_DRV_ERR_DAEMON_MCI_ERROR;
    }

    return MC_DRV_OK;
}

//------------------------------------------------------------------------------
mcResult_t MobiCoreDevice::openSession(
    Connection                      *deviceConnection,
    loadDataOpenSession_ptr         pLoadDataOpenSession,
    uint32_t                        tciHandle,
    uint32_t                        tciLen,
    uint32_t                        tciOffset,
    mcDrvRspOpenSessionPayload_ptr  pRspOpenSessionPayload)
{
    do {
        uint64_t tci = 0;
        uint32_t len = 0;

        if (tciHandle != 0 && tciLen != 0) {
            // Check if we have a cont WSM or normal one
            if (findContiguousWsm(tciHandle,
                                  deviceConnection->socketDescriptor, &tci, &len)) {
                mcpMessage->cmdOpen.wsmTypeTci = WSM_CONTIGUOUS;
            mcpMessage->cmdOpen.adrTciBuffer = tci;
                mcpMessage->cmdOpen.ofsTciBuffer = 0;
            } else if ((tci = findWsmL2(tciHandle, deviceConnection->socketDescriptor))) {
                // We don't actually care about the len as the L2 table mapping is done
                // // and the TL will segfault if it's trying to access non-allocated memory
                len = tciLen;
                mcpMessage->cmdOpen.wsmTypeTci = WSM_L2;
            mcpMessage->cmdOpen.adrTciBuffer = tci;
                mcpMessage->cmdOpen.ofsTciBuffer = tciOffset;
            } else {
                LOG_E("Failed to find contiguous WSM %u", tciHandle);
                return MC_DRV_ERR_DAEMON_WSM_HANDLE_NOT_FOUND;
            }

            if (!lockWsmL2(tciHandle)) {
                LOG_E("Failed to lock contiguous WSM %u", tciHandle);
                return MC_DRV_ERR_DAEMON_WSM_HANDLE_NOT_FOUND;
            }
        } else if (tciHandle == 0 && tciLen == 0) {
            mcpMessage->cmdOpen.wsmTypeTci = WSM_INVALID;
            mcpMessage->cmdOpen.adrTciBuffer = (uint32_t)(tci);
            mcpMessage->cmdOpen.ofsTciBuffer = tciOffset;
        }

        if ((tciHandle != 0 && tciLen == 0) || tciLen > len) {
            LOG_E("Invalid TCI len from client - %u, tciHandle %d, Wsm len found %u",
                  tciLen, tciHandle, len);
            return MC_DRV_ERR_TCI_GREATER_THAN_WSM;
        }

        // Write MCP open message to buffer
        mcpMessage->cmdOpen.cmdHeader.cmdId = MC_MCP_CMD_OPEN_SESSION;
        mcpMessage->cmdOpen.uuid = pLoadDataOpenSession->tlHeader->mclfHeaderV2.uuid;
        mcpMessage->cmdOpen.lenTciBuffer = tciLen;
        LOG_I(" Using phys=%#llx, len=%d as TCI buffer", (uint64_t)tci, tciLen);

        // check if load data is provided
        mcpMessage->cmdOpen.wsmTypeLoadData = WSM_L2;
        mcpMessage->cmdOpen.adrLoadData = pLoadDataOpenSession->baseAddr;
        mcpMessage->cmdOpen.ofsLoadData = pLoadDataOpenSession->offs;
        mcpMessage->cmdOpen.lenLoadData = pLoadDataOpenSession->len;
        memcpy(&mcpMessage->cmdOpen.tlHeader, pLoadDataOpenSession->tlHeader, sizeof(*pLoadDataOpenSession->tlHeader));

        // Clear the notifications queue. We asume the race condition we have
        // seen in openSession never happens elsewhere
        notifications = std::queue<notification_t>();

        mcResult_t mcRet = mshNotifyAndWait();
        if (mcRet != MC_MCP_RET_OK)
        {
            LOG_E("mshNotifyAndWait failed for OPEN_SESSION, code %d.", mcRet);
            // Here Mobicore can be considered dead.
            if ((tciHandle != 0) && (tciLen != 0))
            {
                unlockWsmL2(tciHandle);
            }
            return mcRet;
        }

        // Check if the command response ID is correct
        if ((MC_MCP_CMD_OPEN_SESSION | FLAG_RESPONSE) != mcpMessage->rspHeader.rspId) {
            LOG_E("CMD_OPEN_SESSION got invalid MCP command response(0x%X)", mcpMessage->rspHeader.rspId);
            // Something is messing with our MCI memory, we cannot know if the Trustlet was loaded.
            // Had in been loaded, we are loosing track of it here.
            if (tciHandle != 0 && tciLen != 0) {
                unlockWsmL2(tciHandle);
            }
            return MC_DRV_ERR_DAEMON_MCI_ERROR;
        }

        mcRet = mcpMessage->rspOpen.rspHeader.result;

        if (mcRet != MC_MCP_RET_OK) {
            LOG_E("MCP OPEN returned code %d.", mcRet);
            if (tciHandle != 0 && tciLen != 0) {
                unlockWsmL2(tciHandle);
            }
            return MAKE_MC_DRV_MCP_ERROR(mcRet);
        }

        LOG_I(" After MCP OPEN, we have %d queued notifications",
              notifications.size());
        // Read MC answer from MCP buffer
        TrustletSession *trustletSession = new TrustletSession(
            deviceConnection,
            mcpMessage->rspOpen.sessionId);

        pRspOpenSessionPayload->sessionId = trustletSession->sessionId;
        pRspOpenSessionPayload->deviceSessionId = (uintptr_t)trustletSession;
        pRspOpenSessionPayload->sessionMagic = trustletSession->sessionMagic;

        trustletSession->gp_level=((mclfHeaderV24_ptr)&pLoadDataOpenSession->tlHeader->mclfHeaderV2)->gp_level;
        LOG_I(" Trusted App has gp_level %d",trustletSession->gp_level);
        trustletSession->sessionState = TrustletSession::TS_TA_RUNNING;

        trustletSessions.push_back(trustletSession);

        if (tciHandle != 0 && tciLen != 0) {
            trustletSession->addBulkBuff(new CWsm((void *)(uintptr_t)pLoadDataOpenSession->offs, pLoadDataOpenSession->len, tciHandle, 0));
        }

        // We have some queued notifications and we need to send them to them
        // trustlet session
        while (!notifications.empty()) {
            trustletSession->queueNotification(&notifications.front());
            notifications.pop();
        }

    } while (0);
    return MC_DRV_OK;
}

mcResult_t MobiCoreDevice::checkLoad(
    loadDataOpenSession_ptr         pLoadDataOpenSession,
    mcDrvRspOpenSessionPayload_ptr  pRspOpenSessionPayload __unused)
{
    do {
        // Write MCP open message to buffer
        mcpMessage->cmdCheckLoad.cmdHeader.cmdId = MC_MCP_CMD_CHECK_LOAD_TA;
        mcpMessage->cmdCheckLoad.uuid = pLoadDataOpenSession->tlHeader->mclfHeaderV2.uuid;

        // check if load data is provided
        mcpMessage->cmdCheckLoad.wsmTypeLoadData = WSM_L2;
        mcpMessage->cmdCheckLoad.adrLoadData = pLoadDataOpenSession->baseAddr;
        mcpMessage->cmdCheckLoad.ofsLoadData = pLoadDataOpenSession->offs;
        mcpMessage->cmdCheckLoad.lenLoadData = pLoadDataOpenSession->len;
        memcpy(&mcpMessage->cmdCheckLoad.tlHeader, pLoadDataOpenSession->tlHeader, sizeof(*pLoadDataOpenSession->tlHeader));

        // Clear the notifications queue. We asume the race condition we have
        // seen in openSession never happens elsewhere
        notifications = std::queue<notification_t>();

        mcResult_t mcRet = mshNotifyAndWait();
        if (mcRet != MC_MCP_RET_OK)
        {
            LOG_E("mshNotifyAndWait failed for CHECK_LOAD_TA, code %d.", mcRet);
            // Here Mobicore can be considered dead.
            return mcRet;
        }

        // Check if the command response ID is correct
        if ((MC_MCP_CMD_CHECK_LOAD_TA | FLAG_RESPONSE) != mcpMessage->rspHeader.rspId) {
            LOG_E("CMD_OPEN_SESSION got invalid MCP command response(0x%X)", mcpMessage->rspHeader.rspId);
            // Something is messing with our MCI memory, we cannot know if the Trustlet was loaded.
            // Had in been loaded, we are loosing track of it here.

            return MC_DRV_ERR_DAEMON_MCI_ERROR;
        }

        mcRet = mcpMessage->rspCheckLoad.rspHeader.result;

        if (mcRet != MC_MCP_RET_OK) {
            LOG_E("MCP CHECK_LOAD returned code %d.", mcRet);
            return MAKE_MC_DRV_MCP_ERROR(mcRet);
        }
        return MC_DRV_OK;

    } while (0);
    return MC_DRV_ERR_UNKNOWN;
}



//------------------------------------------------------------------------------
TrustletSession *MobiCoreDevice::registerTrustletConnection(
    Connection                    *connection,
    MC_DRV_CMD_NQ_CONNECT_struct *cmdNqConnect
)
{
    LOG_I(" Registering notification socket with Service session %d.",
          cmdNqConnect->sessionId);
    LOG_V("  Searching sessionId %d with sessionMagic %d",
          cmdNqConnect->sessionId,
          cmdNqConnect->sessionMagic);

    for (trustletSessionIterator_t iterator = trustletSessions.begin();
         iterator != trustletSessions.end();
         ++iterator)
    {
        TrustletSession *session = *iterator;

        if (session != (TrustletSession *)(uintptr_t) (cmdNqConnect->deviceSessionId)) {
            continue;
        }

        if ( (session->sessionMagic != cmdNqConnect->sessionMagic)
                || (session->sessionId != cmdNqConnect->sessionId)) {
            continue;
        }

        session->notificationConnection = connection;

        LOG_I(" Found Service session, registered connection.");

        return session;
    }

    LOG_I("registerTrustletConnection(): search failed");
    return NULL;
}


//------------------------------------------------------------------------------
/**
 * Need connection as well as according session ID, so that a client can not
 * close sessions not belonging to him.
 */
mcResult_t MobiCoreDevice::closeSession(
    Connection  *deviceConnection,
    uint32_t    sessionId
) {
    TrustletSession *session = findSession(deviceConnection,sessionId);
    if (session == NULL) {
        LOG_E("cannot close session with id=%d", sessionId);
        return MC_DRV_ERR_DAEMON_UNKNOWN_SESSION;
    }

    if (session->gp_level == 1) {
        mutex_connection.lock();
        LOG_I(" Closing GP TA session...");
        // Disconnect client from this session
        session->deviceConnection = NULL;
        // Free connection, i.e. close nq socket
        delete session->notificationConnection;
        session->notificationConnection = NULL;
        // If exit notification from task arrives during MCP_CLOSE,
        // Daemon can see that CA is already away
        session->sessionState = TrustletSession::TS_CLOSE_SEND;
        mutex_connection.unlock();
    }

    /* close session */
    mcResult_t mcRet = closeSessionInternal(session);

    if ((session->gp_level==1) && (mcRet == MAKE_MC_DRV_MCP_ERROR(MC_MCP_RET_ERR_CLOSE_TASK_FAILED))) {
        LOG_I(" Close session failed this time.");
        // We could have received a spurious notification of TA dead
        if (session->sessionState == TrustletSession::TS_TA_DEAD) {
            LOG_V(" TA died in the meantime, try closing again");
            // Try again
            mcRet = closeSessionInternal(session);
            assert(mcRet == MC_MCP_RET_OK);
        } else {
            // Clean up is deferred to handleTaExit()
            return MC_MCP_RET_OK;
        }
    }

    if (mcRet != MC_MCP_RET_OK) {
        LOG_E("closeSession failed with %d", mcRet);
        return MAKE_MC_DRV_MCP_ERROR(mcRet);
    }


    // clean session WSM
    LOG_I("unlocking session buffers!");
    CWsm_ptr pWsm = session->popBulkBuff();
    while (pWsm)
    {
        unlockWsmL2(pWsm->handle);
        delete pWsm;
        pWsm = session->popBulkBuff();
    }

    // remove sesson from list.
    for (trustletSessionIterator_t iterator = trustletSessions.begin();
         iterator != trustletSessions.end();
         ++iterator)
    {
        if (session == *iterator)
        {
            trustletSessions.erase(iterator);
            delete session;
            break;
        }
    }

    return MC_MCP_RET_OK;
}


//------------------------------------------------------------------------------
void MobiCoreDevice::queueUnknownNotification(
     notification_t notification
) {
    notifications.push(notification);
}

//------------------------------------------------------------------------------
mcResult_t MobiCoreDevice::notify(
    Connection  *deviceConnection,
    uint32_t    sessionId
) {
    TrustletSession *session = findSession(deviceConnection,sessionId);
    if (session == NULL)
    {
        LOG_E("cannot notify session with id=%d", sessionId);
        return MC_DRV_ERR_DAEMON_UNKNOWN_SESSION;
    }

    notify(sessionId);

    return MC_DRV_OK;
}

//------------------------------------------------------------------------------
mcResult_t MobiCoreDevice::mapBulk(
    Connection *deviceConnection,
    uint32_t  sessionId,
    uint32_t  handle,
    uint64_t  pAddrL2,
    uint32_t  offsetPayload,
    uint32_t  lenBulkMem,
    uint32_t  *secureVirtualAdr
) {
    TrustletSession *session = findSession(deviceConnection,sessionId);
    if (session == NULL) {
        LOG_E("cannot mapBulk on session with id=%d", sessionId);
        return MC_DRV_ERR_DAEMON_UNKNOWN_SESSION;
    }

    // TODO-2012-09-06-haenellu: considernot ignoring the error case, ClientLib
    //                           does not allow this.
    session->addBulkBuff(
                new CWsm((void *)(uintptr_t)offsetPayload,
                lenBulkMem,
                handle,
                pAddrL2));

    // Write MCP map message to buffer
    mcpMessage->cmdMap.cmdHeader.cmdId = MC_MCP_CMD_MAP;
    mcpMessage->cmdMap.sessionId = sessionId;
    mcpMessage->cmdMap.wsmType = WSM_L2;
    mcpMessage->cmdMap.adrBuffer = pAddrL2;
    mcpMessage->cmdMap.ofsBuffer = offsetPayload;
    mcpMessage->cmdMap.lenBuffer = lenBulkMem;

    mcResult_t mcRet = mshNotifyAndWait();
    if (mcRet != MC_MCP_RET_OK)
    {
        LOG_E("mshNotifyAndWait failed for MAP, code %d.", mcRet);
        return mcRet;
    }

    // Check if the command response ID is correct
    if (mcpMessage->rspHeader.rspId != (MC_MCP_CMD_MAP | FLAG_RESPONSE)) {
        LOG_E("invalid MCP response for CMD_MAP");
        return MC_DRV_ERR_DAEMON_MCI_ERROR;
    }

    mcRet = mcpMessage->rspMap.rspHeader.result;

    if (mcRet != MC_MCP_RET_OK) {
        LOG_E("MCP MAP returned code %d.", mcRet);
        return MAKE_MC_DRV_MCP_ERROR(mcRet);
    }

    *secureVirtualAdr = mcpMessage->rspMap.secureVirtualAdr;
    return MC_DRV_OK;
}


//------------------------------------------------------------------------------
mcResult_t MobiCoreDevice::unmapBulk(
    Connection  *deviceConnection,
    uint32_t    sessionId,
    uint32_t    handle,
    uint32_t    secureVirtualAdr,
    uint32_t    lenBulkMem
) {
    TrustletSession *session = findSession(deviceConnection,sessionId);
    if (session == NULL) {
        LOG_E("cannot unmapBulk on session with id=%d", sessionId);
        return MC_DRV_ERR_DAEMON_UNKNOWN_SESSION;
    }

    if (!session->findBulkBuff(handle, lenBulkMem)) {
        LOG_E("cannot unmapBulk with handle=%d", handle);
        return MC_DRV_ERR_DAEMON_WSM_HANDLE_NOT_FOUND;
    }

    // Write MCP unmap command to buffer
    mcpMessage->cmdUnmap.cmdHeader.cmdId = MC_MCP_CMD_UNMAP;
    mcpMessage->cmdUnmap.sessionId = sessionId;
    mcpMessage->cmdUnmap.wsmType = WSM_L2;
    mcpMessage->cmdUnmap.secureVirtualAdr = secureVirtualAdr;
    mcpMessage->cmdUnmap.lenVirtualBuffer = lenBulkMem;

    mcResult_t mcRet = mshNotifyAndWait();
    if (mcRet != MC_MCP_RET_OK)
    {
        LOG_E("mshNotifyAndWait failed for UNMAP, code %d.", mcRet);
        return mcRet;
    }

    // Check if the command response ID is correct
    if (mcpMessage->rspHeader.rspId != (MC_MCP_CMD_UNMAP | FLAG_RESPONSE)) {
        LOG_E("invalid MCP response for OPEN_SESSION");
        return MC_DRV_ERR_DAEMON_MCI_ERROR;
    }

    mcRet = mcpMessage->rspUnmap.rspHeader.result;

    if (mcRet != MC_MCP_RET_OK) {
        LOG_E("MCP UNMAP returned code %d.", mcRet);
        return MAKE_MC_DRV_MCP_ERROR(mcRet);
    }

    // Just remove the buffer
    // TODO-2012-09-06-haenellu: Haven't we removed it already?
    if (!session->removeBulkBuff(handle))
    {
        LOG_I("unmapBulk(): no buffer found found with handle=%u", handle);
    }

    return MC_DRV_OK;
}

mcResult_t MobiCoreDevice::getMobiCoreVersion(
    mcDrvRspGetMobiCoreVersionPayload_ptr pRspGetMobiCoreVersionPayload
) {
    // retunt info it we have already fetched it before
    if (mcVersionInfo != NULL)
    {
        pRspGetMobiCoreVersionPayload->versionInfo = *mcVersionInfo;
        return MC_DRV_OK;
    }

    // Write MCP unmap command to buffer
    mcpMessage->cmdGetMobiCoreVersion.cmdHeader.cmdId = MC_MCP_CMD_GET_MOBICORE_VERSION;

    mcResult_t mcRet = mshNotifyAndWait();
    if (mcRet != MC_MCP_RET_OK)
    {
        LOG_E("mshNotifyAndWait failed for GET_MOBICORE_VERSION, code %d.", mcRet);
        return mcRet;
    }

    // Check if the command response ID is correct
    if ((MC_MCP_CMD_GET_MOBICORE_VERSION | FLAG_RESPONSE) != mcpMessage->rspHeader.rspId) {
        LOG_E("invalid MCP response for GET_MOBICORE_VERSION");
        return MC_DRV_ERR_DAEMON_MCI_ERROR;
    }

    mcRet = mcpMessage->rspGetMobiCoreVersion.rspHeader.result;

    if (mcRet != MC_MCP_RET_OK) {
        LOG_E("MC_MCP_CMD_GET_MOBICORE_VERSION error %d", mcRet);
        return MAKE_MC_DRV_MCP_ERROR(mcRet);
    }

    pRspGetMobiCoreVersionPayload->versionInfo = mcpMessage->rspGetMobiCoreVersion.versionInfo;

    // Store MobiCore info for future reference.
    mcVersionInfo = new mcVersionInfo_t();
    *mcVersionInfo = pRspGetMobiCoreVersionPayload->versionInfo;
    return MC_DRV_OK;
}

//------------------------------------------------------------------------------
mcResult_t MobiCoreDevice::loadToken(Connection        *deviceConnection __unused,
                                     loadTokenData_ptr pLoadTokenData)
{
    do {
        mcpMessage->cmdLoadToken.cmdHeader.cmdId = MC_MCP_CMD_LOAD_TOKEN;
        mcpMessage->cmdLoadToken.wsmTypeLoadData = WSM_L2;
        mcpMessage->cmdLoadToken.adrLoadData = (uint32_t) pLoadTokenData->addr;
        mcpMessage->cmdLoadToken.ofsLoadData = pLoadTokenData->offs;
        mcpMessage->cmdLoadToken.lenLoadData = pLoadTokenData->len;

        /* Clear the notifications queue. We asume the race condition we have
         * seen in openSession never happens elsewhere
         */
        notifications = std::queue<notification_t>();

        mcResult_t mcRet = mshNotifyAndWait();
        if (mcRet != MC_MCP_RET_OK)
        {
            LOG_E("mshNotifyAndWait failed for LOAD_TOKEN, code 0x%x.", mcRet);
            /* Here <t-base can be considered dead. */
            return mcRet;
        }

        /* Check if the command response ID is correct */
        if ((MC_MCP_CMD_LOAD_TOKEN | FLAG_RESPONSE) !=
            mcpMessage->rspHeader.rspId) {
            LOG_E("CMD_LOAD_TOKEN got invalid MCP command response(0x%X)",
                  mcpMessage->rspHeader.rspId);
            return MC_DRV_ERR_DAEMON_MCI_ERROR;
        }

        mcRet = mcpMessage->rspLoadToken.rspHeader.result;

        if (mcRet != MC_MCP_RET_OK) {
            LOG_E("MCP LOAD_TOKEN returned code 0x%x.", mcRet);
            return MAKE_MC_DRV_MCP_ERROR(mcRet);
        }

    } while (0);

    return MC_DRV_OK;
}

/** @} */
