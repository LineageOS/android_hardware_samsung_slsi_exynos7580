/*
Copyright  © Trustonic Limited 2013

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  3. Neither the name of the Trustonic Limited nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/** \addtogroup ROOTPA  AIDL Interfaces
 * @{
 */

package com.gd.mobicore.pa.ifc;

import com.gd.mobicore.pa.ifc.CommandResult;


/**
 * This API contains method(s) that are intended to be used for developer
 * trustlet installation. Note that this API may be removed in the future when the
 * developer trustlet installation is handled by a service manager and installation of
 * all trutlets follow the same architecture for RootPA point of view.
 *
 * The service is started by sending intent com.gd.mobicore.pa.service.DEVELOPER_SERVICE in binding.
 * The service is protected by permission com.gd.mobicore.pa.permission.DEVELOPER_PERMISSION with
 * protectionLevel "normal"
 */
interface RootPADeveloperIfc{

    /**
     * This method can be used for installing "developer trustlet" that is not tied to any
     * service provider and/or service manager. It contacts Service Enabler and asks it to
     * perform the tasks, so the device has to be connected to network in order for this to
     * succeed.
     *
     * The service progress is informed with the same Intents as is progress of the
     * @ref RootPAServiceIfc#doProvisioning, however there is an additional
     * Intent com.gd.mobicore.pa.service.INSTALL_TRUSTLET for returning the encrypted
     * trustlet binary (in case of dynamic installation where trustletBinary is given
     * as a parameter).
     *
     * There are constants related to the intents in @ref RootPAProvisioningIntents
     *
     * @param spid service provider id
     * @param uuid uuid of the trustlet in hex, without dashes. Needs to be correct length.
     * @param trustletBinary trustlet binary to be encrypted with and returned back.
     *        If the binary is already encrypted, this array must be empty. The binary
     *        has to be encrypted for transfer. Note that only either trustletBinary
     *        or key can be given. There are sperate instructions on how the binary is to
     *        be encrypted and packaged. Key and trustletBinary are exclusive, only one of
     *        them can be given. This methods uses default values for memoryType (2),
     *        numberOfInstances (1) and flags (0) when trustlet binary is installed.
     * @param key a key that has been used to encrypt the trustlet binary in case when
     *        the trustlet binary is not given as a parameter. This key has to be
     *        encrypted for transfer. There are sperate instructions on how the key is to
     *        be encrypted and packaged. Key and trustletBinary are exclusive, only one of
     *        them can be given.
     * @param minTltVersion minimum version of the trustlet
     * @param tltPukHash this field is not used at the moment, null is fine here.
     *
     * @return indication of successful start of provisioning thread (ROOTPA_OK) or an error code
     */
    CommandResult installTrustletOrKey(in int spid,
                                       in byte[] uuid,
                                       in byte[] trustletBinary,
                                       in byte[] key,
                                       in int minTltVersion,
                                       in byte[] tltPukHash);

    /**
     * This method can be used for installing "developer trustlet" that is not tied to any
     * service provider and/or service manager. It contacts Service Enabler and asks it to
     * perform the tasks, so the device has to be connected to network in order for this to
     * succeed.
     *
     * The service progress is informed with the same Intents as is progress of the
     * @ref RootPAServiceIfc#doProvisioning, however there is an additional
     * Intent com.gd.mobicore.pa.service.INSTALL_TRUSTLET for returning the encrypted
     * trustlet binary.
     *
     * There are constants related to the intents in @ref RootPAProvisioningIntents
     *
     * @param spid service provider id
     * @param uuid uuid of the trustlet in hex, without dashes. Needs to be correct length.
     * @param trustletBinary trustlet binary to be encrypted with and returned back.
     *        If the binary is already encrypted, this array must be empty. The binary
     *        has to be encrypted for transfer. Note that only either trustletBinary
     *        or key can be given. There are sperate instructions on how the binary is to
     *        be encrypted and packaged.
     * @param minTltVersion minimum version of the trustlet
     * @param tltPukHash this field is not used at the moment, null is fine here.
     * @param memoryType memory where the trustlet is to be loaded and executed: 0 - if enough space is available,
              load the Trustlet into the internal memory, otherwise into the external memory, 1 - internal memory,
              2 - external memory
     * @param numberOfInstances indicates how many instances of a trustlet can be installed (run) in parallel
     * @param flags current flags are: 1 - permanent, 2 - service has no WSW control interface,  4 - debuggable
     *
     * @return indication of successful start of provisioning thread (ROOTPA_OK) or an error code
     */
    CommandResult installTrustlet(in int spid,
                                  in byte[] uuid,
                                  in byte[] trustletBinary,
                                  in int minTltVersion,
                                  in byte[] tltPukHash,
                                  in int memoryType,
                                  in int numberOfInstances,
                                  in int flags);

}

/**@}*/
