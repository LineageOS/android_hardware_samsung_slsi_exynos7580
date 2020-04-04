/** @addtogroup MCD_MCDIMPL_DAEMON_CONHDLR
 * @{
 * @file
 *
 * Entry of the MobiCore Driver.
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
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>

#include "mcVersion.h"
#include "mcVersionHelper.h"
#include "mc_linux.h"
#include "log.h"
#include "Mci/mci.h"

#include "MobiCoreDriverApi.h"
#include "MobiCoreDriverCmd.h"
#include "MobiCoreDriverDaemon.h"
#include "PrivateRegistry.h"
#include "MobiCoreDevice.h"
#include "NetlinkServer.h"
#include "FSD.h"

#define DRIVER_TCI_LEN 4096

MC_CHECK_VERSION(MCI, 0, 2);
MC_CHECK_VERSION(SO, 2, 0);
MC_CHECK_VERSION(MCLF, 2, 0);
MC_CHECK_VERSION(CONTAINER, 2, 0);

static void checkMobiCoreVersion(MobiCoreDevice *mobiCoreDevice);

#define LOG_I_RELEASE(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

pthread_mutex_t         syncMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t          syncCondition = PTHREAD_COND_INITIALIZER;
bool 					Th_sync=false;

//------------------------------------------------------------------------------
MobiCoreDriverDaemon::MobiCoreDriverDaemon(
    bool enableScheduler,
    bool loadDriver,
    std::vector<std::string> drivers)
{
    mobiCoreDevice = NULL;

    this->enableScheduler = enableScheduler;
    this->loadDriver = loadDriver;
    this->drivers = drivers;

    for (int i = 0; i < MAX_SERVERS; i++) {
        servers[i] = NULL;
    }
}

//------------------------------------------------------------------------------
MobiCoreDriverDaemon::~MobiCoreDriverDaemon(
    void
)
{
    // Unload any device drivers might have been loaded
    driverResourcesList_t::iterator it;
    for (it = driverResources.begin(); it != driverResources.end(); it++) {
        MobicoreDriverResources *res = *it;
        mobiCoreDevice->closeSession(res->conn, res->sessionId);
        mobiCoreDevice->unregisterWsmL2(res->pTciWsm);
    }
    delete mobiCoreDevice;
    for (int i = 0; i < MAX_SERVERS; i++) {
        delete servers[i];
        servers[i] = NULL;
    }
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::run(
    void
)
{
    const char *devNode = "/dev/" MC_ADMIN_DEVNODE;

    LOG_I_RELEASE("Daemon starting up...");
    LOG_I_RELEASE("Socket interface version is %u.%u", DAEMON_VERSION_MAJOR, DAEMON_VERSION_MINOR);

#ifdef MOBICORE_COMPONENT_BUILD_TAG
    LOG_I_RELEASE("%s", MOBICORE_COMPONENT_BUILD_TAG);
#else
#warning "MOBICORE_COMPONENT_BUILD_TAG is not defined!"
#endif

    LOG_I_RELEASE("Build timestamp is %s %s", __DATE__, __TIME__);

    int i;

    mobiCoreDevice = getDeviceInstance();

    LOG_I("Initializing Device, Daemon sheduler is %s",
          enableScheduler ? "enabled" : "disabled");

    // initialize device (setupo MCI)
    if (!mobiCoreDevice->initDevice(
                devNode,
                enableScheduler)) {
        LOG_E("Could not initialize <t-base (because %s could not be openend)!", devNode);
        return;
    }

    // start device (scheduler)
    mobiCoreDevice->start();

    // Load device driver if requested
    if (loadDriver) {
        for (unsigned int i = 0; i < drivers.size(); i++)
            loadDeviceDriver(drivers[i]);
    }

    /* Look for tokens in the registry and pass them to <t-base for endorsement
     * purposes.
     */
    LOG_I("Looking for suitable tokens");

    mcSoAuthTokenCont_t authtoken;
    mcSoRootCont_t rootcont;
    uint32_t sosize;
    uint8_t *p = NULL;

    mcResult_t ret = mcRegistryReadAuthToken(&authtoken);
    if (ret != MC_DRV_OK) {
        LOG_I("Failed to read AuthToken (ret=%u). Trying Root Container", ret);

        sosize = sizeof(rootcont);
        ret = mcRegistryReadRoot(&rootcont, &sosize);
        if (ret != MC_DRV_OK) {
            LOG_I("Failed to read Root Cont, (ret=%u)", ret);
            LOG_W("Device endorsements not supported!");
            sosize = 0;
        }
        else {
            LOG_I("Found Root Cont.");
            p = (uint8_t *) &rootcont;
        }
    } else {
        LOG_I("Found AuthToken.");
        p = (uint8_t *) &authtoken;
        sosize = sizeof(authtoken);
    }

    if (sosize) {
        LOG_I("Found token of size: %u", sosize);
        if (!loadToken(p, sosize)) {
            LOG_E("Failed to pass token to <t-base. "
                  "Device endorsements disabled");
        }
    }

    LOG_I("Creating socket servers");
    // Start listening for incoming TLC connections
    servers[0] = new NetlinkServer(this);
    servers[1] = new Server(this, SOCK_PATH);
    LOG_I("Successfully created servers");

    // Start all the servers
    for (i = 0; i < MAX_SERVERS; i++) {
        servers[i]->start(i ? "McDaemon.Server" : "NetlinkServer");
    }

    // Create the <t-base File Storage Daemon
    FSD *FileStorageDaemon = new FSD();
    // Start File Storage Daemon
    FileStorageDaemon->start("McDaemon.FSD");

    // then wait for them to exit
    for (i = 0; i < MAX_SERVERS; i++) {
        servers[i]->join();
    }
    //Wait for File Storage Daemon to exit
	FileStorageDaemon->join();
	delete FileStorageDaemon;
}

//------------------------------------------------------------------------------
bool MobiCoreDriverDaemon::checkPermission(Connection *connection __unused)
{
#ifdef REGISTRY_CHECK_PERMISSIONS
    struct ucred cred;
    if (!connection)
        return true;

    if (connection->getPeerCredentials(cred)) {
        gid_t gid = getegid();
        uid_t uid = geteuid();
        LOG_I("Peer connection has pid = %u and uid = %u gid = %u", cred.pid, cred.uid, cred.gid);
        LOG_I("Daemon has uid = %u gid = %u", cred.uid, cred.gid);
        // If the daemon and the peer have the same uid or gid then we're good
        if (gid == cred.gid || uid == cred.uid) {
            return true;
        }
        return false;

    }
    return false;
#else
    return true;
#endif
}

//------------------------------------------------------------------------------
MobiCoreDevice *MobiCoreDriverDaemon::getDevice(
    uint32_t deviceId
)
{
    // Always return the trustZoneDevice as it is currently the only one supported
    if (MC_DEVICE_ID_DEFAULT != deviceId)
        return NULL;
    return mobiCoreDevice;
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::dropConnection(
    Connection *connection
)
{
    // Check if a Device has already been registered with the connection
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);

    if (device != NULL) {
        // A connection has been found and has to be closed
        LOG_I("dropConnection(): closing still open device.");

        // Make sure nobody else writes the MCP, e.g. netlink/socket server, cleanup of TAs
        device->mutex_mcp.lock();
        device->close(connection);
        device->mutex_mcp.unlock();
    }
}


//------------------------------------------------------------------------------
size_t MobiCoreDriverDaemon::writeResult(
    Connection  *connection,
    mcResult_t  code
)
{
    if (0 != code) {
        LOG_V(" sending error code %d", code);
    }
    return connection->writeData(&code, sizeof(mcResult_t));
}

//------------------------------------------------------------------------------
bool MobiCoreDriverDaemon::loadDeviceDriver(
    std::string driverPath
)
{
    bool ret = false;
    CWsm_ptr pWsm = NULL, pTciWsm = NULL;
    regObject_t *regObj = NULL;
    Connection *conn = NULL;
    mcDrvRspOpenSession_t rspOpenSession;

    do {
        //mobiCoreDevice
        LOG_I("%s: loading %s", __FUNCTION__, driverPath.c_str());

        regObj = mcRegistryGetDriverBlob(driverPath.c_str());
        if (regObj == NULL) {
            break;;
        }

        LOG_I("registering L2 in kmod, p=%p, len=%i",
              regObj->value, regObj->len);

        pWsm = mobiCoreDevice->registerWsmL2(
                   (addr_t)(regObj->value), regObj->len, 0);
        if (pWsm == NULL) {
            LOG_E("allocating WSM for Trustlet failed");
            break;
        }
        // Initialize information data of open session command
        loadDataOpenSession_t loadDataOpenSession;
        loadDataOpenSession.baseAddr = pWsm->physAddr;
        loadDataOpenSession.offs = ((uintptr_t) regObj->value) & 0xFFF;
        loadDataOpenSession.len = regObj->len;
        loadDataOpenSession.tlHeader = (mclfHeader_ptr) (regObj->value + regObj->tlStartOffset);

        pTciWsm = mobiCoreDevice->allocateContiguousPersistentWsm(DRIVER_TCI_LEN);
        if (pTciWsm == NULL) {
            LOG_E("allocating WSM TCI for Trustlet failed");
            break;
        }

        conn = new Connection();
        uint32_t mcRet = mobiCoreDevice->openSession(
                             conn,
                             &loadDataOpenSession,
                             pTciWsm->handle,
                             pTciWsm->len,
                             0,
                             &(rspOpenSession.payload));

        // Unregister physical memory from kernel module.
        // This will also destroy the WSM object.
        mobiCoreDevice->unregisterWsmL2(pWsm);
        pWsm = NULL;

        // Free memory occupied by Trustlet data
        free(regObj);
        regObj = NULL;

        if (mcRet != MC_MCP_RET_OK) {
            LOG_E("open session error %d", mcRet);
            break;
        }

        ret = true;
    } while (false);
    // Free all allocated resources
    if (ret == false) {
        LOG_I("%s: Freeing previously allocated resources!", __FUNCTION__);
        if (pWsm != NULL) {
            if (!mobiCoreDevice->unregisterWsmL2(pWsm)) {
                // At least make sure we don't leak the WSM object
                delete pWsm;
            }
        }
        // No matter if we free NULL objects
        free(regObj);

        if (conn != NULL) {
            delete conn;
        }
    } else if (conn != NULL) {
        driverResources.push_back(new MobicoreDriverResources(
                                      conn, pTciWsm, rspOpenSession.payload.sessionId));
    }

    return ret;
}

#define RECV_PAYLOAD_FROM_CLIENT(CONNECTION, CMD_BUFFER) \
{ \
    void *payload = (void*)((uintptr_t)CMD_BUFFER + sizeof(mcDrvCommandHeader_t)); \
    uint32_t payload_len = sizeof(*CMD_BUFFER) - sizeof(mcDrvCommandHeader_t); \
    int32_t rlen = CONNECTION->readData(payload, payload_len); \
    if (rlen < 0) { \
        LOG_E("reading from Client failed"); \
        /* it is questionable, if writing to broken socket has any effect here. */ \
        writeResult(CONNECTION, MC_DRV_ERR_DAEMON_SOCKET); \
        return; \
    } \
    if (rlen != (int32_t)payload_len) {\
        LOG_E("wrong buffer length %i received from Client", rlen); \
        writeResult(CONNECTION, MC_DRV_ERR_DAEMON_SOCKET); \
        return; \
    } \
}

#define CHECK_DEVICE(DEVICE, CONNECTION) \
    if (DEVICE == NULL) \
    { \
        LOG_V("%s: no device associated with connection",__FUNCTION__); \
        writeResult(CONNECTION, MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN); \
        return; \
    }

//------------------------------------------------------------------------------
inline bool getData(Connection *con, void *buf, uint32_t len)
{
    uint32_t rlen = con->readData(buf, len);
    if (rlen < len || (int32_t)rlen < 0) {
        LOG_E("reading from Client failed");
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processOpenDevice(Connection *connection)
{
    MC_DRV_CMD_OPEN_DEVICE_struct cmdOpenDevice;
    RECV_PAYLOAD_FROM_CLIENT(connection, &cmdOpenDevice);

    // Check if device has been registered to the connection
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);
    if (NULL != device) {
        LOG_E("processOpenDevice(): device already set");
        writeResult(connection, MC_DRV_ERR_DEVICE_ALREADY_OPEN);
        return;
    }

    LOG_I(" Opening deviceId %d ", cmdOpenDevice.deviceId);

    // Get device for device ID
    device = getDevice(cmdOpenDevice.deviceId);

    // Check if a device for the given name has been found
    if (device == NULL) {
        LOG_E("invalid deviceId");
        writeResult(connection, MC_DRV_ERR_UNKNOWN_DEVICE);
        return;
    }

    // Register device object with connection
    device->open(connection);

    // Return result code to client lib (no payload)
    writeResult(connection, MC_DRV_OK);
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processCloseDevice(
    Connection  *connection
)
{
    // there is no payload to read

    // Device required
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);
    CHECK_DEVICE(device, connection);

    // No command data will be read
    // Unregister device object with connection
    device->close(connection);

    // there is no payload
    writeResult(connection, MC_DRV_OK);
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processOpenSession(Connection *connection, bool isGpUuid)
{
    MC_DRV_CMD_OPEN_SESSION_struct cmdOpenSession;
    RECV_PAYLOAD_FROM_CLIENT(connection, &cmdOpenSession);

    // Device required
    MobiCoreDevice  *device = (MobiCoreDevice *) (connection->connectionData);
    CHECK_DEVICE(device, connection);

    // Get service blob from registry
    regObject_t *regObj = mcRegistryGetServiceBlob(&cmdOpenSession.uuid, isGpUuid);
    if (NULL == regObj) {
        writeResult(connection, MC_DRV_ERR_TRUSTLET_NOT_FOUND);
        return;
    }
    if (regObj->len == 0) {
        free(regObj);
        writeResult(connection, MC_DRV_ERR_TRUSTLET_NOT_FOUND);
        return;
    }
    LOG_I(" Sharing Service loaded at %p with Secure World", (addr_t)(regObj->value));

    CWsm_ptr pWsm = device->registerWsmL2((addr_t)(regObj->value), regObj->len, 0);
    if (pWsm == NULL) {
        // Free memory occupied by Trustlet data
        free(regObj);
        LOG_E("allocating WSM for Trustlet failed");
        writeResult(connection, MC_DRV_ERR_DAEMON_KMOD_ERROR);
        return;
    }
    // Initialize information data of open session command
    loadDataOpenSession_t loadDataOpenSession;
    loadDataOpenSession.baseAddr = pWsm->physAddr;
    loadDataOpenSession.offs = ((uintptr_t) regObj->value) & 0xFFF;
    loadDataOpenSession.len = regObj->len;
    loadDataOpenSession.tlHeader = (mclfHeader_ptr) (regObj->value + regObj->tlStartOffset);

    mcDrvRspOpenSession_t rspOpenSession;
    mcResult_t ret = device->openSession(
                         connection,
                         &loadDataOpenSession,
                         cmdOpenSession.handle,
                         cmdOpenSession.len,
                         cmdOpenSession.tci,
                         &rspOpenSession.payload);

    // Unregister physical memory from kernel module.
    LOG_I(" Service buffer was copied to Secure world and processed. Stop sharing of buffer.");

    // This will also destroy the WSM object.
    if (!device->unregisterWsmL2(pWsm)) {
        // TODO-2012-07-02-haenellu: Can this ever happen? And if so, we should assert(), also TL might still be running.
        free(regObj);
        writeResult(connection, MC_DRV_ERR_DAEMON_KMOD_ERROR);
        return;
    }

    // Free memory occupied by Trustlet data
    free(regObj);

    if (ret != MC_DRV_OK) {
        LOG_E("Service could not be loaded.");
        writeResult(connection, ret);
    } else {
        rspOpenSession.header.responseId = ret;
        connection->writeData(
            &rspOpenSession,
            sizeof(rspOpenSession));
    }
}

//------------------------------------------------------------------------------
mcResult_t MobiCoreDriverDaemon::processLoadCheck(mcSpid_t spid, void *blob, uint32_t size)
{

    // Device required
    MobiCoreDevice  *device = getDevice(MC_DEVICE_ID_DEFAULT);

    if (device == NULL) {
        LOG_E(" No device found");
        return MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN;
    }

    // Get service blob from registry
    regObject_t *regObj = mcRegistryMemGetServiceBlob(spid, blob, size);
    if (NULL == regObj) {
        LOG_E(" mcRegistryMemGetServiceBlob failed");
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }
    if (regObj->len == 0) {
        free(regObj);
        LOG_E("mcRegistryMemGetServiceBlob returned registry object with length equal to zero");
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }
    LOG_I(" Sharing Service loaded at %p with Secure World", (addr_t)(regObj->value));

    CWsm_ptr pWsm = device->registerWsmL2((addr_t)(regObj->value), regObj->len, 0);
    if (pWsm == NULL) {
        // Free memory occupied by Trustlet data
        free(regObj);
        LOG_E("allocating WSM for Trustlet failed");
        return MC_DRV_ERR_DAEMON_KMOD_ERROR;
    }
    // Initialize information data of open session command
    loadDataOpenSession_t loadDataOpenSession;
    loadDataOpenSession.baseAddr = pWsm->physAddr;
    loadDataOpenSession.offs = ((uintptr_t) regObj->value) & 0xFFF;
    loadDataOpenSession.len = regObj->len;
    loadDataOpenSession.tlHeader = (mclfHeader_ptr) (regObj->value + regObj->tlStartOffset);

    mcDrvRspOpenSession_t rspOpenSession;
    mcResult_t ret = device->checkLoad(
                         &loadDataOpenSession,
                         &rspOpenSession.payload);

    // Unregister physical memory from kernel module.
    LOG_I(" Service buffer was copied to Secure world and processed. Stop sharing of buffer.");

    // This will also destroy the WSM object.
    if (!device->unregisterWsmL2(pWsm)) {
        // Free memory occupied by Trustlet data
        free(regObj);
        LOG_E("deallocating WSM for Trustlet failed");
        return MC_DRV_ERR_DAEMON_KMOD_ERROR;
    }

    // Free memory occupied by Trustlet data
    free(regObj);

    if (ret != MC_DRV_OK) {
        LOG_E("TA could not be loaded.");
        return ret;
    } else {
        return MC_DRV_OK;
    }
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processOpenTrustlet(Connection *connection)
{
    MC_DRV_CMD_OPEN_TRUSTLET_struct cmdOpenTrustlet;
    RECV_PAYLOAD_FROM_CLIENT(connection, &cmdOpenTrustlet);

    // Device required
    MobiCoreDevice  *device = (MobiCoreDevice *) (connection->connectionData);
    CHECK_DEVICE(device, connection);

    uint32_t total_len, rlen, len = cmdOpenTrustlet.trustlet_len;
    uint8_t *payload = (uint8_t *)malloc(len);
    uint8_t *p = payload;
    if (payload == NULL) {
        LOG_E("failed to allocate payload buffer");
        writeResult(connection, MC_DRV_ERR_DAEMON_SOCKET);
        return;
    }
    total_len = 0;
    while (total_len < len) {
        rlen = connection->readData(p, len - total_len);
        if ((int32_t)rlen < 0) {
            LOG_E("reading from Client failed");
            /* it is questionable, if writing to broken socket has any effect here. */
            writeResult(connection, MC_DRV_ERR_DAEMON_SOCKET);
            free(payload);
            return;
        }
        total_len += rlen;
        p += rlen;
    }

    // Get service blob from registry
    regObject_t *regObj = mcRegistryMemGetServiceBlob(cmdOpenTrustlet.spid, (uint8_t *)payload, len);

    // Free the payload object no matter what
    free(payload);
    if (regObj == NULL) {
        writeResult(connection, MC_DRV_ERR_TRUSTLET_NOT_FOUND);
        return;
    }

    if (regObj->len == 0) {
        free(regObj);
        writeResult(connection, MC_DRV_ERR_TRUSTLET_NOT_FOUND);
        return;
    }
    LOG_I(" Sharing Service loaded at %p with Secure World", (addr_t)(regObj->value));

    CWsm_ptr pWsm = device->registerWsmL2((addr_t)(regObj->value), regObj->len, 0);
    if (pWsm == NULL) {
        free(regObj);
        LOG_E("allocating WSM for Trustlet failed");
        writeResult(connection, MC_DRV_ERR_DAEMON_KMOD_ERROR);
        return;
    }
    // Initialize information data of open session command
    loadDataOpenSession_t loadDataOpenSession;
    loadDataOpenSession.baseAddr = pWsm->physAddr;
    loadDataOpenSession.offs = ((uintptr_t) regObj->value) & 0xFFF;
    loadDataOpenSession.len = regObj->len;
    loadDataOpenSession.tlHeader = (mclfHeader_ptr) (regObj->value + regObj->tlStartOffset);

    mcDrvRspOpenSession_t rspOpenSession;
    mcResult_t ret = device->openSession(
                         connection,
                         &loadDataOpenSession,
                         cmdOpenTrustlet.handle,
                         cmdOpenTrustlet.len,
                         cmdOpenTrustlet.tci,
                         &rspOpenSession.payload);

    // Unregister physical memory from kernel module.
    LOG_I(" Service buffer was copied to Secure world and processed. Stop sharing of buffer.");

    // This will also destroy the WSM object.
    if (!device->unregisterWsmL2(pWsm)) {
        free(regObj);
        // TODO-2012-07-02-haenellu: Can this ever happen? And if so, we should assert(), also TL might still be running.
        writeResult(connection, MC_DRV_ERR_DAEMON_KMOD_ERROR);
        return;
    }

    // Free memory occupied by Trustlet data
    free(regObj);

    if (ret != MC_DRV_OK) {
        LOG_E("Service could not be loaded.");
        writeResult(connection, ret);
    } else {
        rspOpenSession.header.responseId = ret;
        connection->writeData(
            &rspOpenSession,
            sizeof(rspOpenSession));
    }
}

//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processCloseSession(Connection *connection)
{
    MC_DRV_CMD_CLOSE_SESSION_struct cmdCloseSession;
    RECV_PAYLOAD_FROM_CLIENT(connection, &cmdCloseSession)

    // Device required
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);
    CHECK_DEVICE(device, connection);

    mcResult_t ret = device->closeSession(connection, cmdCloseSession.sessionId);

    // there is no payload
    writeResult(connection, ret);
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processNqConnect(Connection *connection)
{
    // Set up the channel for sending SWd notifications to the client
    // MC_DRV_CMD_NQ_CONNECT is only allowed on new connections not
    // associated with a device. If a device is registered to the
    // connection NQ_CONNECT is not allowed.

    // Read entire command data
    MC_DRV_CMD_NQ_CONNECT_struct cmd;
    RECV_PAYLOAD_FROM_CLIENT(connection, &cmd);

    // device must be empty since this is a new connection
    MobiCoreDevice *device = (MobiCoreDevice *)(connection->connectionData);
    if (device != NULL) {
        LOG_E("device already set\n");
        writeResult(connection, MC_DRV_ERR_NQ_FAILED);
        return;
    }

    // Remove the connection from the list of known client connections
    for (int i = 0; i < MAX_SERVERS; i++) {
        servers[i]->detachConnection(connection);
    }

    device = getDevice(cmd.deviceId);
    // Check if a device for the given name has been found
    if (NULL == device) {
        LOG_E("invalid deviceId");
        writeResult(connection, MC_DRV_ERR_UNKNOWN_DEVICE);
        return;
    }

    TrustletSession *ts = device->registerTrustletConnection(
                              connection,
                              &cmd);
    if (!ts) {
        LOG_E("registerTrustletConnection() failed!");
        writeResult(connection, MC_DRV_ERR_UNKNOWN);
        return;
    }

    writeResult(connection, MC_DRV_OK);
    ts->processQueuedNotifications();
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processNotify(Connection  *connection)
{
    // Read entire command data
    MC_DRV_CMD_NOTIFY_struct  cmd;
    //RECV_PAYLOAD_FROM_CLIENT(connection, &cmd);
    void *payload = (void *)((uintptr_t)&cmd + sizeof(mcDrvCommandHeader_t));
    uint32_t payload_len = sizeof(cmd) - sizeof(mcDrvCommandHeader_t);
    uint32_t rlen = connection->readData(payload, payload_len);
    if ((int) rlen < 0) {
        LOG_E("reading from Client failed");
        /* it is questionable, if writing to broken socket has any effect here. */
        // NOTE: notify fails silently
        //writeResult(connection, MC_DRV_RSP_SOCKET_ERROR);
        return;
    }
    if (rlen != payload_len) {
        LOG_E("wrong buffer length %i received from Client", rlen);
        // NOTE: notify fails silently
        //writeResult(connection, MC_DRV_RSP_PAYLOAD_LENGTH_ERROR);
        return;
    }

    // Device required
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);
    if (NULL == device) {
        LOG_V("%s: no device associated with connection", __FUNCTION__);
        // NOTE: notify fails silently
        // writeResult(connection,MC_DRV_RSP_DEVICE_NOT_OPENED);
        return;
    }

    device->notify(connection, cmd.sessionId);
    // NOTE: for notifications there is no response at all
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processMapBulkBuf(Connection *connection)
{
    MC_DRV_CMD_MAP_BULK_BUF_struct cmd;

    RECV_PAYLOAD_FROM_CLIENT(connection, &cmd);

    // Device required
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);
    CHECK_DEVICE(device, connection);

    if (!device->lockWsmL2(cmd.handle)) {
        LOG_E("Couldn't lock the buffer!");
        writeResult(connection, MC_DRV_ERR_DAEMON_WSM_HANDLE_NOT_FOUND);
        return;
    }

    uint32_t secureVirtualAdr = (uint32_t)NULL;
    uint64_t pAddrL2 = device->findWsmL2(cmd.handle, connection->socketDescriptor);

    if (pAddrL2 == 0) {
        LOG_E("Failed to resolve WSM with handle %u", cmd.handle);
        writeResult(connection, MC_DRV_ERR_DAEMON_WSM_HANDLE_NOT_FOUND);
        return;
    }

    // Map bulk memory to secure world
    mcResult_t mcResult = device->mapBulk(connection, cmd.sessionId, cmd.handle, pAddrL2,
                                          cmd.offsetPayload, cmd.lenBulkMem, &secureVirtualAdr);

    if (mcResult != MC_DRV_OK) {
        writeResult(connection, mcResult);
        return;
    }

    mcDrvRspMapBulkMem_t rsp;
    rsp.header.responseId = MC_DRV_OK;
    rsp.payload.sessionId = cmd.sessionId;
    rsp.payload.secureVirtualAdr = secureVirtualAdr;
    connection->writeData(&rsp, sizeof(mcDrvRspMapBulkMem_t));
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processUnmapBulkBuf(Connection *connection)
{
    MC_DRV_CMD_UNMAP_BULK_BUF_struct cmd;
    RECV_PAYLOAD_FROM_CLIENT(connection, &cmd)

    // Device required
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);
    CHECK_DEVICE(device, connection);

    // Unmap bulk memory from secure world
    uint32_t mcResult = device->unmapBulk(connection, cmd.sessionId, cmd.handle,
                                          cmd.secureVirtualAdr, cmd.lenBulkMem);

    if (mcResult != MC_DRV_OK) {
        LOG_V("MCP UNMAP returned code %d", mcResult);
        writeResult(connection, mcResult);
        return;
    }

    // TODO-2012-09-06-haenellu: Think about not ignoring the error case.
    device->unlockWsmL2(cmd.handle);

    writeResult(connection, MC_DRV_OK);
}


//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processGetVersion(
    Connection  *connection
)
{
    // there is no payload to read

    mcDrvRspGetVersion_t rspGetVersion;
    rspGetVersion.version = MC_MAKE_VERSION(DAEMON_VERSION_MAJOR, DAEMON_VERSION_MINOR);
    rspGetVersion.responseId = MC_DRV_OK;

    connection->writeData(&rspGetVersion, sizeof(mcDrvRspGetVersion_t));
}

//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processGetMobiCoreVersion(
    Connection  *connection
)
{
    // there is no payload to read

    // Device required
    MobiCoreDevice *device = (MobiCoreDevice *) (connection->connectionData);
    CHECK_DEVICE(device, connection);

    // Get <t-base version info from secure world.
    mcDrvRspGetMobiCoreVersion_t rspGetMobiCoreVersion;

    mcResult_t mcResult = device->getMobiCoreVersion(&rspGetMobiCoreVersion.payload);

    if (mcResult != MC_DRV_OK) {
        LOG_V("MC GET_MOBICORE_VERSION returned code %d", mcResult);
        writeResult(connection, mcResult);
        return;
    }

    rspGetMobiCoreVersion.header.responseId = MC_DRV_OK;
    connection->writeData(
        &rspGetMobiCoreVersion,
        sizeof(rspGetMobiCoreVersion));
}

//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processRegistryReadData(uint32_t commandId, Connection  *connection)
{
#define MAX_DATA_SIZE 512
mcDrvResponseHeader_t rspRegistry = { .responseId =
                                          MC_DRV_ERR_INVALID_OPERATION
                                        };
    void *buf = alloca(MAX_DATA_SIZE);
    uint32_t len = MAX_DATA_SIZE;
    mcSoAuthTokenCont_t auth;
    mcSpid_t spid;
    mcUuid_t uuid;

    if (!checkPermission(connection)) {
        connection->writeData(&rspRegistry, sizeof(rspRegistry));
        return;
    }

    switch (commandId) {
    case MC_DRV_REG_READ_AUTH_TOKEN:
        rspRegistry.responseId = mcRegistryReadAuthToken(&auth);
        buf = &auth;
        len = sizeof(mcSoAuthTokenCont_t);
        break;
    case MC_DRV_REG_READ_ROOT_CONT:
        rspRegistry.responseId = mcRegistryReadRoot(buf, &len);
        break;
    case MC_DRV_REG_READ_SP_CONT:
        if (!getData(connection, &spid, sizeof(spid)))
            break;
        rspRegistry.responseId = mcRegistryReadSp(spid, buf, &len);
        break;
    case MC_DRV_REG_READ_TL_CONT:
        if (!getData(connection, &uuid, sizeof(uuid)))
            break;
        if (!getData(connection, &spid, sizeof(spid)))
            break;
        rspRegistry.responseId = mcRegistryReadTrustletCon(&uuid, spid, buf, &len);
        break;
    default:
        break;
    }
    connection->writeData(&rspRegistry, sizeof(rspRegistry));
    if (rspRegistry.responseId != MC_DRV_ERR_INVALID_OPERATION)
        connection->writeData(buf, len);
}

//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processRegistryWriteData(uint32_t commandId, Connection *connection)
{
mcDrvResponseHeader_t rspRegistry = { .responseId =
                                          MC_DRV_ERR_INVALID_OPERATION
                                        };
    uint32_t soSize;
    void *so;

    if (!checkPermission(connection)) {
        connection->writeData(&rspRegistry, sizeof(rspRegistry));
        return;
    }

    // First read the SO data size
    if (!getData(connection, &soSize, sizeof(soSize))) {
        LOG_E("Failed to read SO data size");
        connection->writeData(&rspRegistry, sizeof(rspRegistry));
        return;
    }
    so = malloc(soSize);
    if (so == NULL) {
        LOG_E("Allocation failure");
        rspRegistry.responseId = MC_DRV_ERR_NO_FREE_MEMORY;
        connection->writeData(&rspRegistry, sizeof(rspRegistry));
        return;
    }

    switch (commandId) {
    case MC_DRV_REG_STORE_AUTH_TOKEN: {
        if (!getData(connection, so, soSize))
            break;
        rspRegistry.responseId = mcRegistryStoreAuthToken(so, soSize);
        break;
    }
    case MC_DRV_REG_WRITE_ROOT_CONT: {
        if (!getData(connection, so, soSize))
            break;
        rspRegistry.responseId = mcRegistryStoreRoot(so, soSize);
        break;
    }
    case MC_DRV_REG_WRITE_SP_CONT: {
        mcSpid_t spid;
        if (!getData(connection, &spid, sizeof(spid)))
            break;
        if (!getData(connection, so, soSize))
            break;
        rspRegistry.responseId = mcRegistryStoreSp(spid, so, soSize);
        break;
    }
    case MC_DRV_REG_WRITE_TL_CONT: {
        mcUuid_t uuid;
        mcSpid_t spid;
        if (!getData(connection, &uuid, sizeof(uuid)))
            break;
        if (!getData(connection, &spid, sizeof(spid)))
            break;
        if (!getData(connection, so, soSize))
            break;
        rspRegistry.responseId = mcRegistryStoreTrustletCon(&uuid, spid, so, soSize);
        break;
    }
    case MC_DRV_REG_WRITE_SO_DATA: {
        if (!getData(connection, so, soSize))
            break;
        rspRegistry.responseId = mcRegistryStoreData(so, soSize);
        break;
    }
    case MC_DRV_REG_STORE_TA_BLOB: {
        uint32_t blobSize = soSize;
        mcSpid_t spid;
        void     *blob;
        if (!getData(connection, &spid, sizeof(spid)))
            break;
        blob = malloc(blobSize);
        if (blob == NULL) {
            LOG_E("Allocation failure");
            rspRegistry.responseId = MC_DRV_ERR_NO_FREE_MEMORY;
            break;
        }
        if (!getData(connection, blob, blobSize)) {
            free(blob);
            break;
        }
        //LOG_I("processLoadCheck");
        rspRegistry.responseId = processLoadCheck(spid, blob, blobSize);
        if (rspRegistry.responseId != MC_DRV_OK){
            LOG_I("processLoadCheck failed");
            free(blob);
            break;
        }
        //LOG_I("mcRegistryStoreTABlob");
        rspRegistry.responseId = mcRegistryStoreTABlob(spid, blob, blobSize);
        free(blob);
        break;
    }
    default:
        break;
    }
    free(so);
    connection->writeData(&rspRegistry, sizeof(rspRegistry));
}

//------------------------------------------------------------------------------
void MobiCoreDriverDaemon::processRegistryDeleteData(uint32_t commandId, Connection *connection)
{
mcDrvResponseHeader_t rspRegistry = { .responseId =
                                          MC_DRV_ERR_INVALID_OPERATION
                                        };
    mcSpid_t spid;

    if (!checkPermission(connection)) {
        connection->writeData(&rspRegistry, sizeof(rspRegistry));
        return;
    }

    switch (commandId) {
    case MC_DRV_REG_DELETE_AUTH_TOKEN:
        rspRegistry.responseId = mcRegistryDeleteAuthToken();
        break;
    case MC_DRV_REG_DELETE_ROOT_CONT:
        rspRegistry.responseId = mcRegistryCleanupRoot();
        break;
    case MC_DRV_REG_DELETE_SP_CONT:
        if (!getData(connection, &spid, sizeof(spid)))
            break;
        rspRegistry.responseId = mcRegistryCleanupSp(spid);
        break;
    case MC_DRV_REG_DELETE_TL_CONT:
        mcUuid_t uuid;
        if (!getData(connection, &uuid, sizeof(uuid)))
            break;
        if (!getData(connection, &spid, sizeof(spid)))
            break;
        rspRegistry.responseId = mcRegistryCleanupTrustlet(&uuid, spid);
        break;
    default:
        break;
    }

    connection->writeData(&rspRegistry, sizeof(rspRegistry));
}

//------------------------------------------------------------------------------
bool MobiCoreDriverDaemon::handleConnection(
    Connection *connection
)
{
    bool ret = false;

    // This is the big lock around everything the Daemon does, including socket and MCI access
    static CMutex mutex;

    /* In case of RTM fault do not try to signal anything to MobiCore
     * just answer NO to all incoming connections! */
    if (mobiCoreDevice->getMcFault()) {
        LOG_I("Ignore request, <t-base has faulted before.");
        return false;
    }

    mutex.lock();
    mobiCoreDevice->mutex_mcp.lock();

    LOG_I("handleConnection()==== %p", connection);
    do {
        // Read header
        mcDrvCommandHeader_t mcDrvCommandHeader;
        ssize_t rlen = connection->readData(
                           &(mcDrvCommandHeader),
                           sizeof(mcDrvCommandHeader));

        if (rlen == 0) {
            LOG_V(" handleConnection(): Connection closed.");
            break;
        }
        if (rlen == -1) {
            LOG_E("Socket error.");
            break;
        }
        if (rlen == -2) {
            LOG_E("Timeout.");
            break;
        }
        ret = true;

        switch (mcDrvCommandHeader.commandId) {
            //-----------------------------------------
        case MC_DRV_CMD_OPEN_DEVICE:
            processOpenDevice(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_CLOSE_DEVICE:
            processCloseDevice(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_OPEN_SESSION:
            processOpenSession(connection, false);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_OPEN_TRUSTLET:
            processOpenTrustlet(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_OPEN_TRUSTED_APP:
            processOpenSession(connection, true);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_CLOSE_SESSION:
            processCloseSession(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_NQ_CONNECT:
            processNqConnect(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_NOTIFY:
            processNotify(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_MAP_BULK_BUF:
            processMapBulkBuf(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_UNMAP_BULK_BUF:
            processUnmapBulkBuf(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_GET_VERSION:
            processGetVersion(connection);
            break;
            //-----------------------------------------
        case MC_DRV_CMD_GET_MOBICORE_VERSION:
            processGetMobiCoreVersion(connection);
            break;
            //-----------------------------------------
            /* Registry functionality */
            // Write Registry Data
        case MC_DRV_REG_STORE_AUTH_TOKEN:
        case MC_DRV_REG_WRITE_ROOT_CONT:
        case MC_DRV_REG_WRITE_SP_CONT:
        case MC_DRV_REG_WRITE_TL_CONT:
        case MC_DRV_REG_WRITE_SO_DATA:
        case MC_DRV_REG_STORE_TA_BLOB:
            processRegistryWriteData(mcDrvCommandHeader.commandId, connection);
            break;
            //-----------------------------------------
            // Read Registry Data
        case MC_DRV_REG_READ_AUTH_TOKEN:
        case MC_DRV_REG_READ_ROOT_CONT:
        case MC_DRV_REG_READ_SP_CONT:
        case MC_DRV_REG_READ_TL_CONT:
            processRegistryReadData(mcDrvCommandHeader.commandId, connection);
            break;
            //-----------------------------------------
            // Delete registry data
        case MC_DRV_REG_DELETE_AUTH_TOKEN:
        case MC_DRV_REG_DELETE_ROOT_CONT:
        case MC_DRV_REG_DELETE_SP_CONT:
        case MC_DRV_REG_DELETE_TL_CONT:
            processRegistryDeleteData(mcDrvCommandHeader.commandId, connection);
            break;
            //-----------------------------------------
        default:
            LOG_E("Unknown command: %d=0x%x",
                  mcDrvCommandHeader.commandId,
                  mcDrvCommandHeader.commandId);
            ret = false;
            break;
        }
    } while (0);
    mobiCoreDevice->mutex_mcp.unlock();
    mutex.unlock();
    LOG_I("handleConnection()<-------");

    return ret;
}

//------------------------------------------------------------------------------
/**
 * Print daemon command line options
 */

void printUsage(
    int argc __unused,
    char *args[]
)
{
#ifdef MOBICORE_COMPONENT_BUILD_TAG
    fprintf(stderr, "<t-base Driver Daemon %u.%u. \"%s\" %s %s\n", DAEMON_VERSION_MAJOR, DAEMON_VERSION_MINOR, MOBICORE_COMPONENT_BUILD_TAG, __DATE__, __TIME__);
#else
#warning "MOBICORE_COMPONENT_BUILD_TAG is not defined!"
#endif

    fprintf(stderr, "usage: %s [-mdsbhp]\n", args[0]);
    fprintf(stderr, "Start <t-base Daemon\n\n");
    fprintf(stderr, "-h\t\tshow this help\n");
    fprintf(stderr, "-b\t\tfork to background\n");
    fprintf(stderr, "-s\t\tdisable daemon scheduler(default enabled)\n");
    fprintf(stderr, "-r DRIVER\t<t-base driver to load at start-up\n");
}

//------------------------------------------------------------------------------
/**
 * Signal handler for daemon termination
 * Using this handler instead of the standard libc one ensures the daemon
 * can cleanup everything -> read() on a FD will now return EINTR
 */
void terminateDaemon(
    int signum
)
{
    LOG_E("Signal %d received\n", signum);
}

//------------------------------------------------------------------------------
/**
 * Main entry of the <t-base Driver Daemon.
 */
int main(int argc, char *args[])
{
    // Create the <t-base Driver Singleton
    MobiCoreDriverDaemon *mobiCoreDriverDaemon = NULL;
    // Process signal action
    struct sigaction action;

    // Read the Command line options
    extern char *optarg;
    extern int optopt;
    int c, errFlag = 0;
    // Scheduler enabled by default
    int schedulerFlag = 1;
    // Autoload driver at start-up
    int driverLoadFlag = 0;
    std::vector<std::string> drivers;
    // By default don't fork
    bool forkDaemon = false;

    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&syncMutex, NULL);
    pthread_cond_init (&syncCondition, NULL);

    while ((c = getopt(argc, args, "r:sbhp:")) != -1) {
        switch (c) {
        case 'h': /* Help */
            errFlag++;
            break;
        case 's': /* Disable Scheduler */
            schedulerFlag = 0;
            break;
        case 'b': /* Fork to background */
            forkDaemon = true;
            break;
        case 'r': /* Load <t-base driver at start-up */
            driverLoadFlag = 1;
            drivers.push_back(optarg);
            break;
        case ':':       /* -r operand */
            fprintf(stderr, "Option -%c requires an operand\n", optopt);
            errFlag++;
            break;
        case '?':
            fprintf(stderr, "Unrecognized option: -%c\n", optopt);
            errFlag++;
        }
    }
    if (errFlag) {
        printUsage(argc, args);
        exit(2);
    }

    // We should fork the daemon to background
    if (forkDaemon == true) {

        /* ignore terminal has been closed signal */
        signal(SIGHUP, SIG_IGN);

        int i = fork();
        if (i < 0) {
            exit(1);
        }
        // Parent
        else if (i > 0) {
            exit(0);
        }

        // obtain a new process group */
        setsid();
        /* close all descriptors */
        for (i = sysconf(_SC_OPEN_MAX); i >= 0; --i) {
            close(i);
        }
        // STDIN, STDOUT and STDERR should all point to /dev/null */
        i = open("/dev/null", O_RDWR);
        dup(i);
        dup(i);
        /* ignore tty signals */
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
    }

    // Set up the structure to specify the new action.
    action.sa_handler = terminateDaemon;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
    signal(SIGPIPE, SIG_IGN);

    mobiCoreDriverDaemon = new MobiCoreDriverDaemon(
        /* Scheduler status */
        schedulerFlag,
        /* Auto Driver loading */
        driverLoadFlag,
        drivers);

    // Start the driver
    mobiCoreDriverDaemon->run();

    delete mobiCoreDriverDaemon;

    pthread_mutex_destroy(&syncMutex);
    pthread_cond_destroy(&syncCondition);

    // This should not happen
    LOG_E("Exiting <t-base Daemon");

    return EXIT_FAILURE;
}

//------------------------------------------------------------------------------
static void __unused checkMobiCoreVersion(
    MobiCoreDevice *mobiCoreDevice
)
{
    bool failed = false;

    // Get MobiCore version info.
    mcDrvRspGetMobiCoreVersionPayload_t versionPayload;
    mcResult_t mcResult = mobiCoreDevice->getMobiCoreVersion(&versionPayload);

    if (mcResult != MC_DRV_OK) {
        LOG_E("Failed to obtain <t-base version info. MCP return code: %u", mcResult);
        failed = true;
    } else {
        LOG_I_RELEASE("Product ID is %s", versionPayload.versionInfo.productId);

        // Check <t-base version info.
        char *msg;
        if (!checkVersionOkMCI(versionPayload.versionInfo.versionMci, &msg)) {
            LOG_E("%s", msg);
            failed = true;
        }
        LOG_I_RELEASE("%s", msg);
        if (!checkVersionOkSO(versionPayload.versionInfo.versionSo, &msg)) {
            LOG_E("%s", msg);
            failed = true;
        }
        LOG_I_RELEASE("%s", msg);
        if (!checkVersionOkMCLF(versionPayload.versionInfo.versionMclf, &msg)) {
            LOG_E("%s", msg);
            failed = true;
        }
        LOG_I_RELEASE("%s", msg);
        if (!checkVersionOkCONTAINER(versionPayload.versionInfo.versionContainer, &msg)) {
            LOG_E("%s", msg);
            failed = true;
        }
        LOG_I_RELEASE("%s", msg);
    }

    if (failed) {
        exit(1);
    }
}

//------------------------------------------------------------------------------
bool MobiCoreDriverDaemon::loadToken(uint8_t *token, uint32_t sosize)
{
    bool ret = false;
    CWsm_ptr pWsm = NULL;
    Connection *conn = NULL;

    do {
        LOG_I("registering L2 in kmod, p=%p, len=%i", token, sosize);

        pWsm = mobiCoreDevice->registerWsmL2((addr_t) (token), sosize, 0);
        if (pWsm == NULL) {
            LOG_E("allocating WSM for Token failed");
            break;
        }

        /* Initialize information data of LOAD_TOKEN command */
        loadTokenData_t loadTokenData;
        loadTokenData.addr = pWsm->physAddr;
        loadTokenData.offs = ((uintptr_t) token) & 0xFFF;
        loadTokenData.len = sosize;

        conn = new Connection();
        uint32_t mcRet = mobiCoreDevice->loadToken(conn, &loadTokenData);

        /* Unregister physical memory from kernel module. This will also destroy
         * the WSM object.
         */
        mobiCoreDevice->unregisterWsmL2(pWsm);
        pWsm = NULL;

        if (mcRet != MC_MCP_RET_OK) {
            LOG_E("LOAD_TOKEN error 0x%x", mcRet);
            break;
        }
        ret = true;

    } while (false);

    delete pWsm;
    delete conn;

    return ret;
}

/** @} */
