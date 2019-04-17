/*!
 *****************************************************************************
   @file:    adi_ble_radio.c

   @brief:   Core interface layer with the BLE radio. Handles the communication
             with radio using the underlying transport layer. Handles the state
             and events and passes up the critical events to the BLE framework
             layer.

   @details:
  -----------------------------------------------------------------------------

Copyright (c) 2016 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/


/** 
 *  @defgroup ble Bluetooth Low-Energy
 */


/** 
 *  @addtogroup radio Radio Module
 *  @ingroup ble
 *  @{
 *
 *  @brief Radio Module
 *  @details The set of functions in this module provide an interface for applications
 *           to interact with the radio. This module provides encoding and decoding of 
 *           of radio packets and calls into the Transport Abstraction Layer for 
 *           physical layer communication. 
 */


/*! \cond PRIVATE */


#include "adi_ble_radio_internal.h"

#include <string.h>
#include <rtos_map/adi_rtos_map.h>
#include <framework/noos/adi_ble_noos.h>


/* Local memory */
static ADI_BLE_RADIO_Device gBLERadio;
static ADI_BLE_RADIO_Device *pBLERadio = &gBLERadio;


/* Local functions */
static        ADI_BLER_RESULT bler_process_cmd       (ADI_BLER_CMD_OPCODE eCmdOpCode, uint8_t nParamLength, void * pRspResult, uint32_t nRspResultLen);
static        ADI_BLER_RESULT bler_parse_packet      (ADI_BLE_EVENT_PACKET * pPacket, uint8_t nValidBytes);
static        ADI_BLER_RESULT bler_parse_event       (ADI_BLE_EVENT_PACKET * pPacket);
static        ADI_BLER_RESULT bler_parse_response    (ADI_BLE_EVENT_PACKET * pPacket);
static        ADI_BLER_EVENT  bler_handle_packet     (ADI_BLE_EVENT_PACKET * pPacket);
static        ADI_BLER_EVENT  bler_handle_event      (ADI_BLE_EVENT_PACKET * pPacket);
static        ADI_BLER_EVENT  bler_handle_response   (ADI_BLE_EVENT_PACKET * pPacket);


/*! \endcond */


/*********************************************************************************
                                    CORE
*********************************************************************************/


/**
 * @brief      Initialize BLE Radio
 *
 * @details    This API is used to initialize the bluetooth radio module.The bluetooth framework layer provides
 *             configuration information such as event callback handler.It is expected that the upper layer shall
 *             initialize the transport layer before calling this function. This API also initializes
 *             the underlying radio by sending required commands to the radio firmware.
 *
 * @param [in] pCallbackFunc : Pointer to the callback function
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_Init(ADI_CALLBACK pCallbackFunc)
{
    ADI_BLE_TRANSPORT_RESULT tResult;
    ADI_BLER_RESULT          bleResult;

    ASSERT(pCallbackFunc != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEM_INIT);
    ADI_BLE_RADIO_CMD_START(CMD_BLEM_INIT);

    /* Initialize the transport layer. Transport layer is statically configured to either SPI2 or UART0 */
    tResult = adi_tal_Init(pCallbackFunc);
    RETURN_ERROR(tResult, ADI_BLE_TRANSPORT_SUCCESS, ADI_BLER_FAILURE);

    /* Initialize BLE radio */
    bleResult = bler_process_cmd(CMD_BLEM_INIT, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Returns the BLE core stack version
 *
 * @details     This API is used to get the core BLE stack version
 *
 * @param [out] pVersion : Pointer to the core stack version
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_GetControllerVersion
 *
 */
ADI_BLER_RESULT adi_radio_GetCoreStackVersion(uint32_t * const pVersion)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pVersion != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEM_GET_STACK_VER);
    ADI_BLE_RADIO_CMD_START(CMD_BLEM_GET_STACK_VER);

    /* Get core stack version */
    bleResult = bler_process_cmd(CMD_BLEM_GET_STACK_VER, 0u, pVersion, 4u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Returns BLE controller manufacturer and version information
 *
 * @details     This API is used to get the BLE device manufacturer and version information
 *
 * @param [out] pControllerVersion : Pointer to the result
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_GetControllerVersion(ADI_BLER_CONT_VERSION * const pControllerVersion)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pControllerVersion != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEM_GET_CONTR_VER);
    ADI_BLE_RADIO_CMD_START(CMD_BLEM_GET_CONTR_VER);

    bleResult = bler_process_cmd(CMD_BLEM_GET_CONTR_VER, 0u, pControllerVersion, 4u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Send vendor specific command
 *
 * @details     This API is used to send the vendor specific command and associated data
 *
 * @param [in]  nVendorOpCode : Vendor specific opcode
 *
 * @param [in]  nParamLen : Length of the payload that will be sent
 *
 * @param [in]  pParams : Pointer to the payload that will be sent. Allocated by user and of length nParamLen.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SendVendorCommand(const uint16_t nVendorOpCode, const uint8_t nParamLen, uint8_t * const pParams)
{
    ADI_BLER_RESULT bleResult;

    /* FIXME: Check nParamLen */
    ASSERT(pParams != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEM_HCI_SND_VNDR_CMD);
    ADI_BLE_RADIO_CMD_START(CMD_BLEM_HCI_SND_VNDR_CMD);

    pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN] = (uint8_t) (nVendorOpCode & 0x00FFu);
    pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u] = (uint8_t) ((nVendorOpCode & 0xFF00u) >> 8u);
    pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 2u] = nParamLen;

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 3u], pParams, nParamLen);

    bleResult = bler_process_cmd(CMD_BLEM_HCI_SND_VNDR_CMD, 3u + nParamLen, NULL, 0u);

    /* FIXME: Not sure if this is robust enough, some vendor commands might have events and some might not, for the SetMacAddress
       one we know that an event is triggered, so for now I will set it to that (currently the only use case) */
    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                                    GAP: GENERAL
*********************************************************************************/


/**
 * @brief       Register Device
 *
 * @details     Register the Bluetooth device in a particular GAP role. This API must be
 *              called after calling #adi_radio_Init and before calling any other
 *              API.
 *
 * @param [in]  eBleRole : Role of the BLE device. This could be any of the following:
 *                          - #ADI_BLE_ROLE_CENTRAL
 *                          - #ADI_BLE_ROLE_PERIPHERAL
 *                          - #ADI_BLE_ROLE_OBSERVER
 *                          - #ADI_BLE_ROLE_BROADCASTER
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_RegisterDevice(const ADI_BLE_ROLE eBleRole)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_REGISTER_DEVICE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_REGISTER_DEVICE);

    pBLERadio->eBleDevRole = eBleRole;

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &eBleRole, 1u);

    bleResult = bler_process_cmd(CMD_BLEGAP_REGISTER_DEVICE, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Switch GAP Role
 *
 * @details     This API is used to switch the BLE GAP role. The role switch can only happen
 *              if the local device:
 *              - is not connected
 *              - is not discoverable
 *              - is not initiating a connection
 *              - is not broadcasting
 *              - is not advertising
 *               
 * @param [in] peDesiredBleRole : Pointer to the desired role to be switched to.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SwitchRole(ADI_BLE_ROLE * const peDesiredBleRole)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(peDesiredBleRole != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_SWITCH_ROLE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_SWITCH_ROLE);

    pBLERadio->eBleDevRole = *peDesiredBleRole;

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], peDesiredBleRole, 1u);

    bleResult = bler_process_cmd(CMD_BLEGAP_SWITCH_ROLE, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get GAP Role
 *
 * @details     This API is used to get the current GAP role of the BLE device.
 *
 * @param [out] pCurrentRole : Pointer to the result holding the current role. Allocated 
 *              by caller.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_GetCurrentRole(ADI_BLE_ROLE * const pCurrentRole)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pCurrentRole != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_CURROLE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_CURROLE);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_CURROLE, 0u, pCurrentRole, sizeof(ADI_BLE_ROLE));

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get Local Address
 *
 * @details     Returns the local bluetooth address.
 *
 * @param [out] pConfigAddr : Pointer to the Bluetooth address of the local device. Allocated
 *                            by caller.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 */
ADI_BLER_RESULT adi_radio_GetLocalBluetoohAddr(ADI_BLER_CONFIG_ADDR * const pConfigAddr)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pConfigAddr != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_LOCAL_BD_ADDR);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_LOCAL_BD_ADDR);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_LOCAL_BD_ADDR, 0u, pConfigAddr, sizeof(ADI_BLER_CONFIG_ADDR));

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief      Set Accessibility Mode
 *
 * @details    Set the accessibility mode of the device by specifying connectivity and discoverability. 
 *
 * @param [in] eGapMode (required) : Indicates the GAP accessibility mode.
 *
 * @param [in] pAdvInterval (optional) : Set the advertisment interval, valid values between [0x0020 : 0x4000]. 
 *                                      Set this to NULL to ignore. Required if pConfigAddr is supplied (!= NULL).
 *
 * @param [in] pConfigAddr (optional) : Pointer to a address configuration structure. Allocated and filled by the caller. 
 *                                      Set to NULL to ignore.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SetMode(ADI_BLE_GAP_MODE eGapMode, uint16_t * const pAdvInterval, ADI_BLER_CONFIG_ADDR * const pConfigAddr)
{
    ADI_BLER_RESULT     bleResult;
    ADI_BLER_CMD_OPCODE eCmdOpCode;
    uint8_t             nParamLen;

    /* Place the first parameter after the header */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &eGapMode, 1u);

    /* IF(Basic set mode, user wants to ignore the other two arguments) */
    if ((pAdvInterval == NULL) && (pConfigAddr == NULL))
    {
        eCmdOpCode = CMD_BLEGAP_SET_MODE;
        nParamLen  = 1u;
    }
    /* ELSE(User either wants set with intervals or set with intervals extended) */
    else
    {
        ASSERT(*pAdvInterval <= 0x4000u);
        ASSERT(*pAdvInterval >= 0x0020u);

        eCmdOpCode = CMD_BLEGAP_SET_MODE_INTERVALS;
        nParamLen  = 3u;
        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u], pAdvInterval, 2u);

      if(pConfigAddr != NULL)
      {
          eCmdOpCode = CMD_BLEGAP_SET_MODE_INTERVALS_EX;
          nParamLen += 7u;
          memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 3u], pConfigAddr->aBD_ADDR, 6u);
          memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 9u], &(pConfigAddr->nAddrType), 1u);
      }
    } /* ENDIF */

    ADI_BLE_LOGEVENT((ADI_BLE_LOG_ID)eCmdOpCode);
    ADI_BLE_RADIO_CMD_START(eCmdOpCode);

    pBLERadio->eBleGapMode = eGapMode;

    bleResult = bler_process_cmd(eCmdOpCode, nParamLen, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE, ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get Accessibility Mode
 *
 * @details     Get the accessibility mode of the device.
 *
 * @param [out] pBleMode : Pointer to the result holding the current mode. Allocated by caller.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *                       
 * @note        When GAP events occur (like connecting or disconnecting) the mode of the device may change automatically. 
 *              It is recommended to call this API after a #GAP_EVENT_MODE_CHANGE event to query the new mode.
 * 
 */
ADI_BLER_RESULT adi_radio_GetMode(ADI_BLE_GAP_MODE * const pBleMode)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pBleMode != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_MODE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_MODE);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_MODE, 0u, pBleMode, sizeof(ADI_BLE_GAP_MODE));

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                              GAP: CENTRAL OR PERIPHERAL
*********************************************************************************/


/**
 * @brief       Disconnect Device
 *
 * @details     This API is used to disconnect local device from an established connection
 *              with a remote device.
 *
 * @param [in]  nConnHandle : Handle for the connection to disconnect.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_CancelCreateConnection
 * 
 * @note        After calling this API, a #GAP_EVENT_DISCONNECTED event will occur.
 * 
 */
ADI_BLER_RESULT adi_radio_Disconnect(const uint16_t nConnHandle)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_DISCONNECT_CONNECTION);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_DISCONNECT_CONNECTION);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);

	bleResult = bler_process_cmd(CMD_BLEGAP_DISCONNECT_CONNECTION, 2u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Update Connection Intveral
 *
 * @details     This API is used to update the current connection interval. It is useful to decrease
 *              the connection interval when the application wants to exchange a lot of data (pairing,
 *              profile link up) but it consumes more power so it is useful to increase the connection
 *              interval to save power.
 *
 * @param [in] nConnHandle   (required) : Handle that identifies the connection to be updated.
 *
 * @param [in] nConnInterval (optional) : Defines a simple connection interval. Required if pConfigConnEx == NULL.
 *                                        Interval = 1.25ms * nConnInterval. Must be in range [0x0006u:0x0C80].
 *
 * @param [in] pConfigConnEx (optional) : Defines a detailed connection interval. Required if nConnInterval == 0u.
 *                                        
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *                       
 * @note        After calling this API, a #GAP_EVENT_CONNECTION_UPDATED event will occur.
 */
ADI_BLER_RESULT adi_radio_UpdateConnectionInterval(const uint16_t nConnHandle, const uint16_t nConnInterval, ADI_BLER_CONFG_CONN_EX * const pConfigConnEx)
{
    ADI_BLER_RESULT     bleResult;
    ADI_BLER_CMD_OPCODE eCmdOpCode;
    uint8_t             nParamLen;

    ASSERT((nConnInterval != 0u) || (pConfigConnEx != NULL));

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);

    if (nConnInterval != 0u)
    {
        ASSERT(nConnInterval <= 0x0C80u);
        ASSERT(nConnInterval >= 0x0006u);

        eCmdOpCode = CMD_BLEGAP_UPDATE_CONN_INTERVAL;
        nParamLen  = 4u;
        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 2u], &nConnInterval, 2u);
    }
    else
    {
        ASSERT(pConfigConnEx->nConnIntervalMin <= 0x0C80u);
        ASSERT(pConfigConnEx->nConnIntervalMin >= 0x0006u);
        ASSERT(pConfigConnEx->nConnIntervalMax <= 0x0C80u);
        ASSERT(pConfigConnEx->nConnIntervalMax >= 0x0006u);
        ASSERT(pConfigConnEx->nConnLatency <= 0x01F3u);
        ASSERT(pConfigConnEx->nConnTimeout <= 0x0C80u);
        ASSERT(pConfigConnEx->nConnTimeout >= 0x000Au);

        eCmdOpCode = CMD_BLEGAP_UPDATE_CONN_INTERVAL_EX;
        nParamLen  = 10u;
        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 2u], pConfigConnEx, 8u);
    }

    ADI_BLE_LOGEVENT((ADI_BLE_LOG_ID)eCmdOpCode);
    ADI_BLE_RADIO_CMD_START(eCmdOpCode);

    bleResult = bler_process_cmd(eCmdOpCode, nParamLen, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get Remote Device Name
 *
 * @details     This API is used to get the name of the remote device the local device is connected to.
 *
 * @param [in]  nConnHandle : Handle that identifies the connection
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @note        After calling this event, a #GAP_EVENT_NAME_INFORMATION will occur. The user should then call
 *              #adi_ble_GetNameBuffer to extract the name.
 */
ADI_BLER_RESULT adi_radio_GetRemoteDeviceName(const uint16_t nConnHandle)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_REMOTE_DEV_NAME);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_REMOTE_DEV_NAME);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_REMOTE_DEV_NAME, 2u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get Connection Handle
 *
 * @details     This API is used to get the handle of the connection currently active with the remote
 *              device specified in pBleAddr.
 *
 * @param [in]  pBleAddr : Pointer to the BLE address used to identify the connected device (6 bytes).
 *
 * @param [out] pConnHandle : Pointer to where the handle will be returned. Allocated by caller.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_GetConnectionHandle(uint8_t * const pBleAddr, uint16_t * const pConnHandle)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pBleAddr != NULL);
    ASSERT(pConnHandle != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_CONNECTION_HANDLE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_CONNECTION_HANDLE);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], pBleAddr, 6u);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_CONNECTION_HANDLE, 6u, pConnHandle, 2u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get Remote Device Address
 *
 * @details     This API is used to get the address of the remote device currently connected with
 *              the handle specified in nConnHandle.
 *
 * @param [in]  nConnHandle : Handle that identifies the connection.
 *
 * @param [out] pBleAddr: Pointer to the BLE address used to identify the connected device (6 bytes).
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_GetConnectionAddr(const uint16_t nConnHandle, uint8_t * const pBleAddr)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pBleAddr != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_BD_ADDR);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_BD_ADDR);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_BD_ADDR, 2u, pBleAddr, 6u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get List of Active Connections
 *
 * @details     This API is used to return the number of active connections and the active connection handles.
 *
 * @param [out] pConnectionList : Pointer to the output that will store the connection information.
 *                                The first byte will be the number of connections, and the subsequent bytes 
 *                                will be the connection handles (in sets of 2 bytes). Allocated by caller.
 *                             
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 */
ADI_BLER_RESULT adi_radio_GetConnectionList(ADI_BLER_CONN_LIST * pConnectionList)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pConnectionList != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_NUM_ACTIVE_CONNS);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_NUM_ACTIVE_CONNS);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_NUM_ACTIVE_CONNS, 0u, pConnectionList, ADI_BLER_CONN_LIST_SIZE);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                                  GAP: CENTRAL
*********************************************************************************/


/**
 * @brief       Connect to Remote Device
 *
 * @details     This API is used to create a bluetooth low energy connection with the remote device identified
 *              with the given address and address type.
 *
 * @param [in]  pConfigAddr (required) : Pointer to an address configuration structure. Allocated and filled by the caller.
 *
 * @param [in]  pConfigConn (optional) : Pointer to a connection configuration structure. Allocated and filled by the caller.
 *                                       Set to NULL to ignore. Required if nConnTimeout is supplied (!= 0).
 * 
 * @param [in]  nConnTimeout (optional) : Timeout parameter. Timeout = 10 ms * nConnTimeout. Valid values 
 *                                        between [0x000A : 0x0C80]. Set to 0 to ignore.   
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *                       
 * @note        After calling this API, a #GAP_EVENT_CONNECTED event should occur. The user can then call #adi_ble_GetConnectionInfo
 *              to query the information about the connection that has just been established.
 * 
 */
ADI_BLER_RESULT adi_radio_Connect(ADI_BLER_CONFIG_ADDR * const pConfigAddr, ADI_BLER_CONFIG_CONN * const pConfigConn, uint16_t nConnTimeout)
{
    ADI_BLER_RESULT     bleResult;
    ADI_BLER_CMD_OPCODE eCmdOpCode;
    uint8_t             nParamLen;

    ASSERT(pConfigAddr != NULL);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], pConfigAddr->aBD_ADDR, 6u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 6u], &(pConfigAddr->nAddrType), 1u);

    /* IF(Basic connect mode, user wants to ignore the other two arguments) */
    if (pConfigConn == NULL)
    {
        eCmdOpCode = CMD_BLEGAP_CONNECT;
        nParamLen  = 7u;
    }
    /* ELSE(User either wants connect with intervals or connect with intervals extended) */
    else
    {
        ASSERT(pConfigConn->nScanInterval <= 0x4000u);
        ASSERT(pConfigConn->nScanInterval >= 0x0004u);
        ASSERT(pConfigConn->nScanWindow <= 0x4000u);
        ASSERT(pConfigConn->nScanWindow >= 0x0004u);
        ASSERT(pConfigConn->nConnInterval <= 0x0C80u);
        ASSERT(pConfigConn->nConnInterval >= 0x0006u);

        eCmdOpCode = CMD_BLEGAP_CONNECT_INTERVALS;
        nParamLen  = 13u;
        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 7u], &(pConfigConn->nScanInterval), 2u);
        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 9u], &(pConfigConn->nScanWindow), 2u);
        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 11u], &(pConfigConn->nConnInterval), 2u);

      if(nConnTimeout != 0)
      {
          ASSERT(nConnTimeout <= 0x0C80);
          ASSERT(nConnTimeout >= 0x000Au);

          eCmdOpCode = CMD_BLEGAP_CONNECT_INTERVALS_EX;
          nParamLen += 2u;
          memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 13u], &nConnTimeout, 2u);
      }
    } /* ENDIF */

    ADI_BLE_LOGEVENT((ADI_BLE_LOG_ID)eCmdOpCode);
    ADI_BLE_RADIO_CMD_START(eCmdOpCode);

    bleResult = bler_process_cmd(eCmdOpCode, nParamLen, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Cancel Connection
 *
 * @details     This API is used to cancel connection creation that is in process. 
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_Disconnect
 *
 * @note        After calling this event, a #CMD_BLEGAP_CANCEL_CONNECTION will occur.
 * 
 */
ADI_BLER_RESULT adi_radio_CancelCreateConnection(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_CANCEL_CONNECTION);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_CANCEL_CONNECTION);

    bleResult = bler_process_cmd(CMD_BLEGAP_CANCEL_CONNECTION, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief         Start Inquiry
 *
 * @details       Start the inquiry prodcedure in order to discover remote devices that are advertising.
 *
 * @param [in]    eInquiryMode : Specifies the inquiry mode, mode could be 
 *                              - #INQUIRY_MODE_GENERAL
 *                              - #INQUIRY_MODE_LIMITED
 *                              
 * @return        ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @note          The inquiry process is entirely event driven. After calling this API, a series of 
 *                #GAP_EVENT_INQUIRYRESULT and #GAP_EVENT_INQUIRYRESULT_EXT (extra data that couldn't 
 *                fit in the previous event) will be received. The user should call #adi_ble_GetInquiryData
 *                to query the inquiry data. The inquiry will conclude with a #GAP_EVENT_INQUIRYRESULT_COMPLETE
 *                event.
 * 
 */
ADI_BLER_RESULT adi_radio_StartInquiry(const ADI_BLE_INQUIRY_MODE eInquiryMode)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_START_INQUIRY);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_START_INQUIRY);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &eInquiryMode, 1u);

    bleResult = bler_process_cmd(CMD_BLEGAP_START_INQUIRY, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE, ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Cancel Inquiry
 *
 * @details     This API is used to cancel an inquiry that is in progress. The user should call this 
 *              after calling #adi_radio_StartInquiry if they wish to cancel the inquiry procedure.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_CancelInquiry(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_CANCEL_INQUIRY);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_CANCEL_INQUIRY);

    bleResult = bler_process_cmd(CMD_BLEGAP_CANCEL_INQUIRY, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                              GAP: CENTRAL OR OBSERVER
*********************************************************************************/


/**
 * @brief       Parse Advertising Data
 *
 * @details     When performing inquiry or observation, the user will receive a series of advertising
 *              packets from the remote device. This function is used to parse that advertising data. A 
 *              bitfield will be returning indicating what types of data (service data, tx power, name, 
 *              etc.) are in the advertising packet. The enumeration #ADI_BLE_GAP_ADV_DATA_TYPE describes
 *              the different types of advertising data that can be received. The value of each enumeration
 *              represents the bit position of the bitfield. The only exception to this is the vendor field
 *              (0xFF) which corresponds to bit 0x10000000.
 *
 * @param [in]  pAdvData : Pointer to the advertising data received.
 *
 * @param [out] pAdvDataBitField : A bitfield describing the advertising data given as input.
 *  
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_ParseAdvData(ADI_BLER_BUFFER * const pAdvData, uint32_t * const pAdvDataBitField)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_ADV_DATATYPES);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_ADV_DATATYPES);

    ASSERT(pAdvData != NULL);
    ASSERT(pAdvData->nDataLen <= 62u); /* FIXME: Confirm that no more than 62 bytes can be received as an advertising payload */
    ASSERT(pAdvDataBitField != NULL);

    /* Copy in the parameters after the header */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &pAdvData->nDataLen, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u], pAdvData->pData, pAdvData->nDataLen);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_ADV_DATATYPES, 1u + pAdvData->nDataLen, pAdvDataBitField, 4u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Filter Advertising Data
 *
 * @details     This function will extract specific fields of the advertising data received during inquiry or observer.
 *              This function should only be called after #adi_radio_ParseAdvData has been called, such that the user 
 *              knows which fields are contained in the advertising data. Then, the user chooses which field they want to
 *              have filtered and pass the advertising data and the enumeration to this function. 
 *
 * @param [in]  pAdvData : The advertising data received during an inquiry
 *
 * @param [in]  eBlockType : The type of data the application wants (e.g. name, service data)
 *
 * @param [out] pFilteredData : The filtered data
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_FilterAdvData(ADI_BLER_BUFFER * const pAdvData, const ADI_BLE_GAP_ADV_DATA_TYPE eBlockType, ADI_BLER_BUFFER * const pFilteredData)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pAdvData != NULL);
    ASSERT(pFilteredData != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_GET_ADV_DATABLK);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_GET_ADV_DATABLK);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &pAdvData->nDataLen, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u], pAdvData->pData, pAdvData->nDataLen);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u + pAdvData->nDataLen], &eBlockType, 1u);

    bleResult = bler_process_cmd(CMD_BLEGAP_GET_ADV_DATABLK, 2u + pAdvData->nDataLen, pFilteredData, pFilteredData->nDataLen);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                                    GAP: OBSERVER
*********************************************************************************/


/**
 * @brief        Start Observation
 *
 * @details      This API is used to start a bluetooth low energy observation procedure in order to get the broadcasted data by other
 *               bluetooth low energy devices in range.
 *
 * @param [in]   bDuplicateFilteringMode (required) : Determine if the observer should enable or disable duplicate filtering capability
 *
 * @param [in]   pScanInfo (optional) : Set the desired scanning interval to scan other devices when observing. Set to NULL to ignore.
 *                                                 Scan Interval = nScanInterval * 0.625ms
 *                                                 Scan Window = nScanWindow * 0.625ms
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @note         Observation is also event driven. After calling this function, a series of #GAP_EVENT_OBS_MODE_DATA events will be 
 *               received. The user can called #adi_ble_GetObserverData to extract the information received during the event.
 *
 */
ADI_BLER_RESULT adi_radio_StartObsvProc(bool bDuplicateFilteringMode, ADI_BLER_CONFIG_SCAN * const pScanInfo)
{
    ADI_BLER_RESULT     bleResult;
    ADI_BLER_CMD_OPCODE eCmdOpCode;
    uint8_t             nParamLen;

    if (bDuplicateFilteringMode == true)
    {

        pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN] = 1u;
    }
    else
    {
        pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN] = 0u;
    }

    if (pScanInfo == NULL)
    {
        eCmdOpCode = CMD_BLEGAP_START_OBS_PROC;
        nParamLen  = 1u;
    }
    else
    {
        eCmdOpCode = CMD_BLEGAP_START_OBS_PROC_INTERVAL;
        nParamLen  = 5u;

        ASSERT(pScanInfo->nScanInterval >= 0x0004u);
        ASSERT(pScanInfo->nScanInterval <= 0x4000u);
        ASSERT(pScanInfo->nScanWindow   >= 0x0004u);
        ASSERT(pScanInfo->nScanWindow   <= 0x4000u);

        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u], &pScanInfo->nScanInterval, 2u);
        memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 3u], &pScanInfo->nScanWindow,   2u);
    }

    ADI_BLE_LOGEVENT((ADI_BLE_LOG_ID)eCmdOpCode);

    bleResult = bler_process_cmd(eCmdOpCode, nParamLen, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    return (bleResult);
}


/**
 * @brief       Stop Observation
 *
 * @details     This API is used to stop the observation procedure.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_StopObsvProc(void)
{
    ADI_BLER_RESULT    bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_STOP_OBS_PROC);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_STOP_OBS_PROC);

    bleResult = bler_process_cmd(CMD_BLEGAP_STOP_OBS_PROC, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                                  GAP: BROADCASTER
*********************************************************************************/


/**
 * @brief       Set Local Name
 *
 * @details     This API is used to set the local name. This is the name that remote devices will
 *              see when they receive advertising packets.
 *
 * @param [in] nNameLen         (required) The length of the bluetooth device friendly name
 * @param [in] pName            (required)  Pointer to the desired local device friendly name
 * @param [in] nIsWriteable     (optional) If set to 1, remote device can overwrite the name. 
 *                                         If set to 0, the name is only readable by a remote device.
 *                                         Set to 3 to ignore.
 * @param [in] nMaxWriteableLen (optional) Specifies the maximum writable length in the isWritable is set to 1.
 *                                         Will be ignored if nIsWriteable is set to 3. 
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SetLocalBluetoothDevName(uint8_t * const pName, uint8_t nNameLen, uint8_t nIsWriteable, uint16_t nMaxWriteableLen)
{
    ADI_BLER_RESULT bleResult;
    uint8_t paramLen = 0u;
    ADI_BLER_CMD_OPCODE opcode;

    ASSERT(nNameLen <= 53u); /* FIXME: confirm the max size should be 246. 64-6-4-1 = 53 */
    ASSERT(nMaxWriteableLen <= 53u);
    ASSERT(pName != NULL);

    /* Copy in the parameter after the header. pNameLen and pName are copied in regardless of the writability */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nNameLen, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u], pName, nNameLen);

    /* If the writable information was not provided use the basic command */
    if(nIsWriteable == 3u)
    {
          opcode = CMD_BLEGAP_SET_LOCAL_BLE_DEVNAME;
          paramLen = 1u + nNameLen;
    }
    /* If the writable information was provided then use the extended command */
    else
    {
          opcode = CMD_BLEGAP_SET_LOCAL_BLE_DEVNAME_EX;
          paramLen = 4u + nNameLen;
          memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + (1u + nNameLen)], &nIsWriteable, 1u);
          memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + (2u + nNameLen)], &nMaxWriteableLen, 2u);
    }

    ADI_BLE_LOGEVENT((ADI_BLE_LOG_ID)opcode);
    ADI_BLE_RADIO_CMD_START(opcode);

    bleResult = bler_process_cmd(opcode, paramLen, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Set Local Appearance
 *
 * @details     This API is used to set the local device appearance. This appearance is populated
 *              when discoverable if the device is peripheral or may be read by the remote device
 *              when connected. Setting the appearance will improve user experience. 
 *
 * @param [in]  eAppearance : GAP appearance value
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SetLocalAppearance(const ADI_BLE_GAP_APPEARANCE eAppearance)
{
    ADI_BLER_RESULT bleResult;
    uint16_t nAppearance = (uint16_t) eAppearance;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_SET_LOCAL_APPEARANCE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_SET_LOCAL_APPEARANCE);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nAppearance, 2u);

    bleResult = bler_process_cmd(CMD_BLEGAP_SET_LOCAL_APPEARANCE, 2u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Set Manufacturer Specific Data
 *
 * @details     This API is used to set manufacturer specific data and its value
 *              that is dispatched when advertising.
 *
 * @param [in]  pValue : Pointer to the service data value
 *
 * @param [in]  nValueLen : Pointer to the number of valid bytes in pValue (less than or equal to 26)
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SetMfgSpecificData(uint8_t * const pValue, const uint8_t nValueLen)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_SET_MANUF_DATA);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_SET_MANUF_DATA);

    ASSERT(pValue != NULL);
    ASSERT(nValueLen <= 26u);

    /* Copy in the parameter after the header.  */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nValueLen, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1u], pValue, nValueLen);

    bleResult = bler_process_cmd(CMD_BLEGAP_SET_MANUF_DATA, 1u + nValueLen, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief      Set Transmit Power
 *
 * @details    This API is used to set the transmit power level that will be used during
 *             advertising.
 *
 * @param [in] nTxPowerLevel : Transmit power level in dB ranging from -127 to 127
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SetTxPowerLevel(const int8_t nTxPowerLevel)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_SET_TX_PWR_LEVEL);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_SET_TX_PWR_LEVEL);

    ASSERT(nTxPowerLevel >= -127);

    /* Copy in the parameter after the header. */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nTxPowerLevel, 1u);

    bleResult = bler_process_cmd(CMD_BLEGAP_SET_TX_PWR_LEVEL, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief      Set Service Data
 *
 * @details    This API is used to set service advertising data. A subset of the services that the
 *             device supports will be advertised when broadcasting. This API allows the user to send
 *             particular data allong with those service UUIDs.
 *
 * @param [in] nUUID : Pointer to 16-bit service UUID for which data is broadcasted 
 *
 * @param [in] pValue : Pointer to the service data value
 *
 * @param [in] nValueLen : Number of valid bytes in pValue (less than or equal to 24)
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SetServiceDataValue(const uint16_t nUUID, uint8_t * const pValue, const uint8_t nValueLen)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_SET_SERV_DATA_VALUE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_SET_SERV_DATA_VALUE);

    ASSERT(pValue != NULL);
    ASSERT(nValueLen <= 24);

    /* Copy in the parameter after the header. */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nUUID, 2u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 2u], &nValueLen, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 3u], pValue, nValueLen);

    bleResult = bler_process_cmd(CMD_BLEGAP_SET_SERV_DATA_VALUE, 3u + nValueLen, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief      Start Broadcasting
 *
 * @details    This API is used to start the broadcast procedure.
 *
 * @param [in] nBroadcastInterval :  Desired interval between broadcast packets in radio slot. One radio slot is 0.625ms
 *                                   allowed range for broadcast interval is 0x00A0 to 0x4000 ( 100ms -- 10240ms)
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_StartBroadcastProc(const uint16_t nBroadcastInterval)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_START_BROADCAST);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_START_BROADCAST);

    ASSERT(nBroadcastInterval <= 0x4000u);
    ASSERT(nBroadcastInterval >= 0x00A0u);

    /* Copy in the parameter after the header. */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nBroadcastInterval, 2u);

    bleResult = bler_process_cmd(CMD_BLEGAP_START_BROADCAST, 2u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Stop Broadcasting
 *
 * @details     This API is used to stop the broadcast procedure.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_StopBroadcastProc(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_STOP_BROADCAST);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_STOP_BROADCAST);

    bleResult = bler_process_cmd(CMD_BLEGAP_STOP_BROADCAST, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Apply Broadcast Values
 *
 * @details     Apply the broadcast data configured in the other broadcast SET APIs:
 *                  - #adi_radio_SetLocalBluetoothDevName
 *                  - #adi_radio_SetLocalAppearance
 *                  - #adi_radio_SetMfgSpecificData
 *                  - #adi_radio_SetTxPowerLevel
 *                  - #adi_radio_SetServiceDataValue
 *                  
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @note        In theory, this function must be called prior to #adi_radio_StartBroadcastProc 
 *              in order to apply the configurations shown above. In practice, it was found that
 *              some broadcast settings will take effect without calling this API.
 * 
 */
ADI_BLER_RESULT adi_radio_ApplyBroadcastValue(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLEGAP_APPLY_BROADCAST_VALUE);
    ADI_BLE_RADIO_CMD_START(CMD_BLEGAP_APPLY_BROADCAST_VALUE);

    bleResult = bler_process_cmd(CMD_BLEGAP_APPLY_BROADCAST_VALUE, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                                  FINDME
*********************************************************************************/


/**
 * @brief       Register Findme Target
 *
 * @details     Registers the findme target with the radio. This enables the findme target
 *              service at the radio end. Applications can set the alert levels after registering
 *              the service.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_Register_FindmeTarget(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_FM_TARGET_REG);
    ADI_BLE_RADIO_CMD_START(CMD_FM_TARGET_REG);

    bleResult = bler_process_cmd(CMD_FM_TARGET_REG, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Set findme target alert level
 *
 * @details     Sets the findme target alert level. Alert level is described by ADI_BLE_FM_ALERT_LEVEL
 *
 * @param [in] eAlertLevel :  Specifies the local alert level of the findme target.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_FM_SetAlertLevel(const ADI_BLE_ALERT_LEVEL eAlertLevel)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_FM_TARGET_SET_ALERT_LEVEL);
    ADI_BLE_RADIO_CMD_START(CMD_FM_TARGET_SET_ALERT_LEVEL);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN],(void*)&eAlertLevel, 1u);

    bleResult = bler_process_cmd(CMD_FM_TARGET_SET_ALERT_LEVEL, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                                PROXIMITY REPORTER
*********************************************************************************/


/**
 * @brief       Register Proximity Reporter
 *
 * @details     Register proximity reporter with the radio. Once the service is registered with radio
 *              the clients can avail its services.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_Register_ProximityReporter(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_PROX_REPORTER_REG);
    ADI_BLE_RADIO_CMD_START(CMD_PROX_REPORTER_REG);

    bleResult = bler_process_cmd(CMD_PROX_REPORTER_REG, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Set Proximity alert level
 *
 * @details     Sets Proximity profile alert level. Alert level is described by ADI_BLE_ALERT_LEVEL
 *
 * @param [in] eLinkLossAlertLevel :  Specifies the local alert level of the proximity profile
 *
 * @param [in] bLinkLoss :  If true indicates link loss alert level or else immediate alert level
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon successful ble response
 *                       - #ADI_BLER_FAILURE is returned upon failure ble response
 *                       - #ADI_BLER_FAILURE is returned when the event processing failed
 *
 */
ADI_BLER_RESULT adi_radio_Proximity_SetAlertLevel(const ADI_BLE_ALERT_LEVEL eLinkLossAlertLevel, const bool bLinkLoss)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_PROX_REPORTER_SET_ALERT_LEVEL);
    ADI_BLE_RADIO_CMD_START(CMD_PROX_REPORTER_SET_ALERT_LEVEL);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN],&eLinkLossAlertLevel, 1u);

    bleResult = bler_process_cmd(((bLinkLoss == true) ? CMD_PROX_REPORTER_SET_ALERT_LEVEL : CMD_PROX_REPORTER_SET_IMM_ALERT_LEVEL), 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                               DATA EXCHANGE
*********************************************************************************/


/**
 * @brief       Register data exchange server
 *
 * @details     Register data exchange server profile
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_Register_DataExchangeServer(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_PROX_REPORTER_REG);
    ADI_BLE_RADIO_CMD_START(CMD_PROX_REPORTER_REG);

    bleResult = bler_process_cmd(CMD_DATA_EXCHANGE_SERV_REG, 0u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Send data through exchange server
 *
 * @details     send data through data exchange server 
 *
 * @param [in] connHandle :  Specifies the connection handle
 * @param [in] txDataLen  :  To be transmitted data length
 * @param [in] ptxData    :  Pointer to the data
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_DE_SendData(const uint16_t connHandle, uint8_t txDataLen, uint8_t *ptxData)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(txDataLen <= 20);

    ADI_BLE_LOGEVENT(LOGID_CMD_PROX_REPORTER_SET_ALERT_LEVEL);
    ADI_BLE_RADIO_CMD_START(CMD_PROX_REPORTER_SET_ALERT_LEVEL);

    /* write connection handle */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN],&connHandle, 2u);

    /* write the data length */
    pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN+2] = txDataLen;

    /* write data */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN+3],ptxData, txDataLen);

    bleResult = bler_process_cmd(CMD_DATA_EXCHANGE_SEND_DATA, txDataLen+3, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
		bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/********************************************************************************
                                    TEST
 ********************************************************************************/


/**
 * @brief       Start Receiver Test
 *
 * @details     Starts the receiver test sequence
 *
 * @param [in] rxFrequency :  Specifies the rx frequency. Frequency = 2407 + (k * 2Mhz). The value of k
 *                            should be less than 0x27
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_StartRecvTest(const uint8_t rxFrequency)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(rxFrequency < 0x27);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLETEST_START_RCVR_TEST);
    ADI_BLE_RADIO_CMD_START(CMD_BLETEST_START_RCVR_TEST);

    /* write rx frequency */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN],&rxFrequency, 1u);

    bleResult = bler_process_cmd(CMD_BLETEST_START_RCVR_TEST,1, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Stop any Test
 *
 * @details     Stops any test that is already scheduled
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_StopTest(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLETEST_STOP_TEST);
    ADI_BLE_RADIO_CMD_START(CMD_BLETEST_STOP_TEST);

    bleResult = bler_process_cmd(CMD_BLETEST_STOP_TEST,1, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Start Transmitter Test
 *
 * @details     Starts the transmitter test sequence
 *
 * @param [in] txFrequency :  Specifies the tx frequency. Frequency = 2407 + (k * 2Mhz). The value of k
 *                            should be less than 0x27
 * @param [in] payloadLen      : specifies the length of the payload data between 0 to 37
 *
 * @param [in] payloadPattern  : Payload pattern
 *                               0x00 - Pseudo random bit sequence 9
 *                               0x01 - Pattern of alternating bits '11110000'
 *                               0x02 - Pattern of alternating bits '10101010'
 *                               0x03 - Pseudo random bit sequence 15
 *                               0x04 - Pattern of all 1's
 *                               0x05 - Pattern of all 0's
 *                               0x06 - Pattern of alternating bits '00001111'
 *                               0x07 - Pattern of alternating bits '01010101'
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_StartTransmitTest(const uint8_t txFrequency,uint8_t payloadLen, uint8_t payloadPattern)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLETEST_START_XMT_TEST);
    ADI_BLE_RADIO_CMD_START(CMD_BLETEST_START_XMT_TEST);

    /* write rx frequency */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN],&txFrequency, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN+1],&payloadLen, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN+2],&payloadPattern, 1u);

    bleResult = bler_process_cmd(CMD_BLETEST_START_XMT_TEST,3, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/*********************************************************************************
                                      SMP
*********************************************************************************/


/**
 * @brief      Initiate a pairing procedure
 *              
 * @details    Initiate a pairing procedure with the remote device connected. The local device will 
 *             become the "initiator" and the remote device will be the "responder"
 * 
 * @param [in] nConnHandle : Handle that identifies the connection 
 * @param [in] nBond : Set to 1 to request the link to be bonded and the security information saved
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_SetPairingPolicy
 * @sa          #adi_radio_RejectPasskey
 * @sa          #adi_radio_ConfirmPasskey
 *
 */
ADI_BLER_RESULT adi_radio_InitiatePairing(const uint16_t nConnHandle, const uint8_t nBond)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_INIT_PAIRING);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_INIT_PAIRING);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 2], &nBond, 1u);

    bleResult = bler_process_cmd(CMD_BLESMP_INIT_PAIRING, 3u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Set a pairing policy
 * 
 * @details     Set a pairing policy to be applied when receiving a pairing request event.
 *              The default policy is to always accept pairing and always accept bonding
 *
 * @param [in] nAccept : 0 means the pairing request is rejected by the application.
 *                       Non-zero means the pairing request is accepted by the application
 * @param [in] nBond : 1 means link will be bonded. This is only valid if pAccept is non-zero
 *                     0 means link will not be bonded
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_InitiatePairing
 * @sa          #adi_radio_RejectPasskey
 * @sa          #adi_radio_ConfirmPasskey
 * 
 */
ADI_BLER_RESULT adi_radio_SetPairingPolicy(const uint8_t nAccept, const uint8_t nBond)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_SET_PAIRING_POLICY);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_SET_PAIRING_POLICY);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nAccept, 1u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 1], &nBond, 1u);

    bleResult = bler_process_cmd(CMD_BLESMP_SET_PAIRING_POLICY, 2u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Function to confirm the passkey
 *
 * @details     Function to confirm the passkey following a passkey request event or a passkey display event
 *
 * @param [in] nConnHandle : Handle that identifies the connection
 * @param [in] pPasskey : The 6 byte passkey to provide or confirm. Each character in the passkey must be ['0' - '9']
 *                        which is the same as 0x30-0x39. 
 * 
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_RejectPasskey
 *
 */
ADI_BLER_RESULT adi_radio_ConfirmPasskey(const uint16_t nConnHandle, uint8_t * const pPasskey)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_CONFIRM_PASSKEY);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_CONFIRM_PASSKEY);

    ASSERT(pPasskey != NULL);

    for(uint8_t i = 0; i < 6; i++)
    {
       ASSERT(pPasskey[i] >= 0x30u);
       ASSERT(pPasskey[i] <= 0x39u);
    }

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 2u], pPasskey, 6u);

    bleResult = bler_process_cmd(CMD_BLESMP_CONFIRM_PASSKEY, 8u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Function to reject the passkey
 * 
 * @details     Function to reject the passkey following a passkey request event or during the passkey display event
 *
 * @param [in] nConnHandle : Handle that identifies the connection
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 * 
 * @sa          #adi_radio_ConfirmPasskey
 *
 */
ADI_BLER_RESULT adi_radio_RejectPasskey(uint16_t nConnHandle)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_REJECT_PASSKEY);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_REJECT_PASSKEY);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);

    bleResult = bler_process_cmd(CMD_BLESMP_REJECT_PASSKEY, 2u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}

/**
 * @brief       Set out of band data
 *
 * @details     Set some out of band data in order to use it as pairing information. When
 *              the application needs to use out of band data it will need to call this 
 *              function before any call to initiate pairing event as initiator or 
 *              before/during the pairing event request if acting as a pairing responder.
 *              Pairing will fail if the OOB data is set and the remote device does not have
 *              any OOB data set and vice-versa. This Out Of band data will be used for every
 *              pairing until the call to BLESMP_ClearOobData() to clear the Out Of Band Data 
 *              or BLESMP_SetOobData() to set new Out Of Band data. If no Out of band data is 
 *              registered, the normal simple pairing behavior is used.
 *
 * @param [in] pOobData : Pointer to 16 bytes of out of band data
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_ClearOobData
 *
 * @note:       The Out Of Band data length SHALL be 16 bytes, the application is responsible to fill
 *              correctly the oobData.
 *
 */
ADI_BLER_RESULT adi_radio_SetOobData(uint8_t * const pOobData)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_SET_OOB_DATA);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_SET_OOB_DATA);

    ASSERT(pOobData != NULL);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], pOobData, 16u); 
    
    bleResult = bler_process_cmd(CMD_BLESMP_SET_OOB_DATA, 16u, NULL, 0u);    

    if(bleResult == ADI_BLER_SUCCESS){
        bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();  
    return (bleResult);
}


/**
 * @brief       Clear the previsouly set out of band data
 *
 * @details     Clear the previously set out of band data in order to not use it anymore
 *              as pairing information
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_SetOobData
 */
ADI_BLER_RESULT adi_radio_ClearOobData(void)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_CLEAR_OOB_DATA);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_CLEAR_OOB_DATA);
    
    bleResult = bler_process_cmd(CMD_BLESMP_CLEAR_OOB_DATA, 0u, NULL, 0u);
    
    if(bleResult == ADI_BLER_SUCCESS){
        bleResult |= ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();  
    return (bleResult);  
}

/**
 * @brief       Set I/O capability of the local device 
 *
 * @details     Dynamically sets the I/O capability of the device
 *
 * @param [in] eIOCapability : IO capability to set for the local device
 *                             - #ADI_BLER_IO_CAP_DISPLAY_ONLY     = 0x00
 *                             - #ADI_BLER_IO_CAP_DISPLAY_YES_NO   = 0x01
 *                             - #ADI_BLER_IO_CAP_KEYBOARD_ONLY    = 0x02 
 *                             - #ADI_BLER_IO_CAP_NO_IN_NO_OUT     = 0x03
 *                             - #ADI_BLER_IO_CAP_KEYBOARD_DISPLAY = 0x04
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 */
ADI_BLER_RESULT adi_radio_SetIOCapability(const ADI_BLER_IO_CAP eIOCapability)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_SET_IO_CAPABILITY);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_SET_IO_CAPABILITY);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &eIOCapability, 1u);

    bleResult = bler_process_cmd(CMD_BLESMP_SET_IO_CAPABILITY, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}

/**
 * @brief       Set the maximum encryption key size
 *
 * @details     Set the maximum encryption key size supported by the local device. In the case
 *              of dynamic encryption key size mode the default value for the device maximum 
 *              encryption key size is set to 16
 *
 * @param [in] nKeySize : Maximum encryption key size set for the local device.
 *                        Must be between 7 and 16 bytes
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_SetMaxEncryptionKeySize(const uint8_t nKeySize)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_SET_MAX_ENCRYPT_SIZE);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_SET_MAX_ENCRYPT_SIZE);

    ASSERT(nKeySize >= 7u);
    ASSERT(nKeySize <= 16u);

    /* Copy in the parameter after the header */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nKeySize, 1u);

    bleResult = bler_process_cmd(CMD_BLESMP_SET_MAX_ENCRYPT_SIZE, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Get security properties
 *
 * @details     Get security properties for the given link. This is useful to know if the link is encrypted, 
 *              bonded or authenticated
 *
 * @param [in]  nConnHandle : The handle that identifies the connection
 * @param [out] pProperties : A pointer to the result holding the link security properties bitfield
 *                              0x00 = no security
 *                              0x10 = authenticated
 *                              0x20 = encrypted
 *                              0x40 = bonded
 * 
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 */
ADI_BLER_RESULT adi_radio_GetLinkSecurityProperties(const uint16_t nConnHandle, uint8_t * const pProperties)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_GET_LINK_SECURITY_INFO);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_GET_LINK_SECURITY_INFO);

    ASSERT(pProperties != NULL);

    /* Copy in the parameter after the header */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &nConnHandle, 2u);

    bleResult = bler_process_cmd(CMD_BLESMP_GET_LINK_SECURITY_INFO, 2u, pProperties, 1u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Find out if device is bonded or not in a given role
 * 
 * @details     Function used to know if the remote device identified by the given address is 
 *              bonded or not when the local device acts in the given role
 *
 * @param [in] pAddr :           The bluetooth device address to check bonding state
 * @param [in] eBleConnRole :    The local device role
 * @param [out] pBonded :        A pointer to the result holding 1 if the device is bonded, 0 if it is not, or 2
 *                               if there was an error.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa          #adi_radio_UnBond
 */
ADI_BLER_RESULT adi_radio_IsDeviceBonded(uint8_t * const pAddr, const ADI_BLE_CONN_ROLE eBleConnRole, uint8_t * const pBonded)
{
    ADI_BLER_RESULT bleResult;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_IS_DEVICE_BONDED);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_IS_DEVICE_BONDED);

    ASSERT(pAddr != NULL);
    ASSERT(pBonded != NULL);

    /* Copy in the parameter after the header */
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], pAddr, 6u);
    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN + 6u], &eBleConnRole, 1u);

    bleResult = bler_process_cmd(CMD_BLESMP_IS_DEVICE_BONDED, 7u, pBonded, 1u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS | ADI_EVENT_FLAG_RESP_FAILURE, ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Unbond a remote device
 *
 * @details     Unbond a remote device. This will remove all persistent information saved for this device
 *
 * @param [in] pAddr : A pointer to the remote bluetooth device address. This should be 6 bytes
 * 
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @sa  adi_radio_IsDeviceBonded 
 */
ADI_BLER_RESULT adi_radio_UnBond(uint8_t * const pAddr)
{
    ADI_BLER_RESULT bleResult;

    ASSERT(pAddr != NULL);

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_UN_BOND);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_UN_BOND);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], pAddr, 6u);

    bleResult = bler_process_cmd(CMD_BLESMP_UN_BOND, 6u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_RESP_SUCCESS | ADI_EVENT_FLAG_RESP_FAILURE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


/**
 * @brief       Generate a new private address
 *
 * @details     This function is used by the application to generate a new private address in
 *              order to use as the local device private address in an advertising process, 
 *              a scanning process or a connection process
 *
 * @param [in] eAddrType : (required)  The type of private address that the application wants to generate and set
 *                                      - #RANDOM_STATIC must be regenerated and changed at each device power up, 
 *                                        it is not resolvable, which means that when the device address is changing 
 *                                        the remote bonded devices are no longer considered as bonded, persistent memory is cleared. 
 *                                        It is targeted for a device that does not restart/power up
 *                                      - #RANDOM_PRIVEATE_NON_RESOLVABLE must be regenerated and changed every 15
 *                                        minutes by the application. It is also not resolvable, which means that when the device 
 *                                        address is changing the remote bonded devices are no longer considered as bonded, and persitent
 *                                        memory is cleared. It is better for privacy, but not for persistent operations. It is targeted 
 *                                        for a device that does not bond with remote devices.
 *                                      - #RANDOM_PRIVATE_RESOLVABLE must be regenerated and changed every 15 minutes by the application.
 *                                        Is resolvable, which means that when the device address is changing the remote bonded devices are
 *                                        able to "resolve the new address" and then recognize the device. It is better for privacy, but costs
 *                                        some time and power to resolve the address.
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned upon success
 *                       - #ADI_BLER_FAILURE is returned upon failure
 *
 * @note : The current STACK implementation supports only RANDOM STATIC for instance
*/
ADI_BLER_RESULT adi_radio_GenerateAndSetRandomAddr(const ADI_BLE_RANDOM_ADDR_TYPE eAddrType)
{
    ADI_BLER_RESULT bleResult = ADI_BLER_SUCCESS;

    ADI_BLE_LOGEVENT(LOGID_CMD_BLESMP_SET_RANDOM_ADDRESS);
    ADI_BLE_RADIO_CMD_START(CMD_BLESMP_SET_RANDOM_ADDRESS);

    memcpy(&pBLERadio->cmdPkt[ADI_RADIO_CMD_HEADER_LEN], &eAddrType, 1u);

    bleResult = bler_process_cmd(CMD_BLESMP_SET_RANDOM_ADDRESS, 1u, NULL, 0u);

    if(bleResult == ADI_BLER_SUCCESS){
    	bleResult = ADI_BLE_WAIT_FOR_COMPLETION(ADI_EVENT_FLAG_EVENT_COMPLETE,ADI_BLER_CMD_TIMEOUT);
    }

    ADI_BLE_RADIO_CMD_END();
    return (bleResult);
}


 /*! \cond PRIVATE */


/*********************************************************************************
                            COMMAND LOGIC
*********************************************************************************/


static ADI_BLER_RESULT bler_process_cmd(ADI_BLER_CMD_OPCODE eCmdOpCode, uint8_t nParamLength, void * pRspResult, uint32_t nRspResultLen)
{
    ADI_BLE_TRANSPORT_RESULT tResult;
    uint8_t                  nCmdLen;

    ADI_BLE_LOCK_RADIO(ADI_BLER_LOCK_RADIO_TIMEOUT);

    /* Retain state from command to response */
    pBLERadio->curCmdOpCode = eCmdOpCode;

    /* Command is the header and the paramter concatenated */
    nCmdLen = ADI_RADIO_CMD_HEADER_LEN + nParamLength;

    /* Fill the commmand packet header */
    SET_CMDPKT_OPCODE    (pBLERadio->cmdPkt);
    SET_CMDPKT_VENDORCODE(pBLERadio->cmdPkt);
    SET_CMDPKT_LEN       (pBLERadio->cmdPkt, 2u + nParamLength);
    SET_CMDPKT_CMDCODE   (pBLERadio->cmdPkt, eCmdOpCode);

    /* If the response will return data, save where it needs to go */
    if ((pRspResult != NULL) && (nRspResultLen != 0u))
    {
        pBLERadio->responseData.pData    = pRspResult;
        pBLERadio->responseData.nDataLen = nRspResultLen;
    }

    /* Send the command */
    tResult = adi_tal_Write(pBLERadio->cmdPkt, nCmdLen);

    ADI_BLE_UNLOCK_RADIO();

    return ((tResult == ADI_BLE_TRANSPORT_SUCCESS) ? ADI_BLER_SUCCESS : ADI_BLER_FAILURE);
}


/*********************************************************************************
                             EVENT LOGIC
*********************************************************************************/


/**
 *  @brief      Parse Packet
 *
 *  @details    Parse the first three bytes of eventPkt, this contains a constant, the packet type
 *              (event or response), and a length. Then call the specific parsing function.
 *
 * @param [out] pPacket : The parsed packet
 *
 * @param [in]  nValidBytes : Number of valid bytes in eventPkt
 *
 * @return      ADI_BLER_RESULT            
 *                       - #ADI_BLER_SUCCESS is returned if parsing worked
 *                       - #ADI_BLER_FAILURE is returned if parsing failed
 *  
 */
static ADI_BLER_RESULT bler_parse_packet(ADI_BLE_EVENT_PACKET * pPacket, uint8_t nValidBytes)
{
    ASSERT(pPacket != NULL);

    /* There must be at least 3 bytes for initial parsing to occur */
    if (nValidBytes < 3u)
    {
        return ADI_BLER_FAILURE;
    }

    /* Verify the value of the first byte */
    if (pBLERadio->eventPkt[0u] != HCI_PKT_TYPE_EVENT)
    {
        return ADI_BLER_FAILURE;
    }

    /* Initial parsing */
    pPacket->nHciEventCode = pBLERadio->eventPkt[1u];
    pPacket->nParamLen     = pBLERadio->eventPkt[2u];

    /* Verify the value of the third byte */
    if (pPacket->nParamLen != (nValidBytes - 3u))
    {
        return ADI_BLER_FAILURE;
    }

    /* Verify the value of the second byte, and parse accordingly */
    if (pPacket->nHciEventCode == ADI_BLE_EVENT_RESP_CODE)
    {
        return (bler_parse_response(pPacket));
    }
    else if (pPacket->nHciEventCode == ADI_BLE_EVENT_EVT_CODE)
    {
        return (bler_parse_event(pPacket));
    }
    else
    {
       return ADI_BLER_FAILURE;
    }
}


static ADI_BLER_RESULT bler_parse_response(ADI_BLE_EVENT_PACKET * pPacket)
{
    /* Check constants */
    RETURN_ERROR(pBLERadio->eventPkt[4u], ADI_VNDR_CMDOPCODE_BYTE0, ADI_BLER_FAILURE);
    RETURN_ERROR(pBLERadio->eventPkt[5u], ADI_VNDR_CMDOPCODE_BYTE1, ADI_BLER_FAILURE);

    /* Parse */
    pPacket->nAciEventCode = (((uint16_t) pBLERadio->eventPkt[7u]) << 8u) | ((uint16_t) pBLERadio->eventPkt[6u]);
    pPacket->nStatus       = pBLERadio->eventPkt[8u];
    pPacket->pParam        = &pBLERadio->eventPkt[9u];

    /* Update parameter size to not include the header */
    if (pPacket->nParamLen < 6u)
    {
        return ADI_BLER_FAILURE;
    }
    else
    {
        pPacket->nParamLen -= 6u;
    }

    return ADI_BLER_SUCCESS;
}


static ADI_BLER_RESULT bler_parse_event(ADI_BLE_EVENT_PACKET * pPacket)
{
    /* Check constants */
    RETURN_ERROR(pBLERadio->eventPkt[3u], ADI_BLE_ACI_EVT_CODE, ADI_BLER_FAILURE);

    /* Parse */
    pPacket->nAciEventCode = (((uint16_t) pBLERadio->eventPkt[5u]) << 8u) | ((uint16_t) pBLERadio->eventPkt[4u]);
    pPacket->nStatus       = pBLERadio->eventPkt[6u];
    pPacket->pParam        = &pBLERadio->eventPkt[7u];

    /* Update parameter size to not include the header */
    if (pPacket->nParamLen < 4u)
    {
        return ADI_BLER_FAILURE;
    }
    else
    {
        pPacket->nParamLen -= 4u;
    }

    return ADI_BLER_SUCCESS;
}


static ADI_BLER_EVENT bler_handle_packet(ADI_BLE_EVENT_PACKET * pPacket)
{
    /* IF(Response packet) */
    if (pPacket->nHciEventCode == ADI_BLE_EVENT_RESP_CODE)
    {
        return (bler_handle_response(pPacket));
    }
    /* ELSE(Event packet) */
    else
    {
        return (bler_handle_event(pPacket));
    }
}


static ADI_BLER_EVENT bler_handle_response(ADI_BLE_EVENT_PACKET * pPacket)
{
    ADI_BLER_CONT_VERSION * pControllerVersion;
    ADI_BLER_CONFIG_ADDR  * pBleAddr;
    ADI_BLER_CONN_LIST    * pConnectionList;
    ADI_BLER_BUFFER       * pBuffer;
    uint8_t               * pBonded;
    uint8_t               * pLinkProps;

    /* Verify that the most recent command matches the received response, otherwise pBLERadio->responseData is invalid */
    if (adi_ble_GetCurCmd() == pPacket->nAciEventCode)
    {

        /* If the response has multiple fields, need to set each field individually - otherwise just memcpy once */
        switch(pPacket->nAciEventCode)
        {
            case CMD_BLEM_GET_CONTR_VER:

                if ((ADI_BLER_CONT_VERSION_SIZE == pPacket->nParamLen) && (ADI_BLER_CONT_VERSION_SIZE == pBLERadio->responseData.nDataLen))
                {

                    pControllerVersion = (ADI_BLER_CONT_VERSION *) pBLERadio->responseData.pData;

                    pControllerVersion->nManufacturerId = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
                    pControllerVersion->nLmpSubVersion  = (((uint16_t) pPacket->pParam[3u]) << 8u) | ((uint16_t) pPacket->pParam[2u]);
                }
                else
                {
                    pPacket->nStatus = ADI_RESPONSE_RADIO_ERROR;
                }

                break;

            case CMD_BLEGAP_GET_LOCAL_BD_ADDR:

                if ((ADI_BLER_CONFIG_ADDR_SIZE == pPacket->nParamLen) && (ADI_BLER_CONFIG_ADDR_SIZE == pBLERadio->responseData.nDataLen))
                {
                    pBleAddr = (ADI_BLER_CONFIG_ADDR *) pBLERadio->responseData.pData;

                    memcpy(pBleAddr->aBD_ADDR, pPacket->pParam, 6u);
                    pBleAddr->nAddrType = (uint8_t) pPacket->pParam[6u];
                }
                else
                {
                    pPacket->nStatus = ADI_RESPONSE_RADIO_ERROR;
                }

                break;

            case CMD_BLEGAP_GET_ADV_DATABLK:
                /* The response data does not count the length, so we subtract one to get the actual payload within pPacket */
                if((pPacket->nParamLen - 1u) <= pBLERadio->responseData.nDataLen)
                {
                    pBuffer = (ADI_BLER_BUFFER *) pBLERadio->responseData.pData;

                    pBuffer->nDataLen = pPacket->pParam[0u];
                    memcpy(pBuffer->pData, &pPacket->pParam[1u], pPacket->pParam[0u]);
                }
                else
                {
                    pPacket->nStatus = ADI_RESPONSE_RADIO_ERROR;
                }

                break;

            case CMD_BLEGAP_GET_NUM_ACTIVE_CONNS:

                if((1u <= pPacket->nParamLen) && (ADI_BLER_CONN_LIST_SIZE == pBLERadio->responseData.nDataLen))
                {
                    pConnectionList = (ADI_BLER_CONN_LIST *) pBLERadio->responseData.pData;

                    pConnectionList->nConnectionListLen = pPacket->pParam[0];
                    memcpy(pConnectionList->aConnectionList, &(pPacket->pParam[1]), 2u*pConnectionList->nConnectionListLen);
                }
                else
                {
                    pPacket->nStatus = ADI_RESPONSE_RADIO_ERROR;
                }

                break;

          case CMD_BLESMP_GET_LINK_SECURITY_INFO:
          if((1u <= pPacket->nParamLen) && (1u == pBLERadio->responseData.nDataLen))
                {
                   pLinkProps = (uint8_t *) pBLERadio->responseData.pData;
                   *pLinkProps = pPacket->pParam[0];
                }
                else
                {
                    pPacket->nStatus = ADI_RESPONSE_RADIO_ERROR;
                }

                break;

          case CMD_BLESMP_IS_DEVICE_BONDED:

                  pBonded = (uint8_t *) pBLERadio->responseData.pData;
                  if(pPacket->nStatus == ADI_RESPONSE_FAILED)
                  {
                    *pBonded = 0u;
                  }
                  else if(pPacket->nStatus == ADI_RESPONSE_SUCCESS)
                  {
                    *pBonded = 1u;
                  }

                  break;

            default:

                if (pBLERadio->responseData.nDataLen >= pPacket->nParamLen)
                {
                    memcpy(pBLERadio->responseData.pData, pPacket->pParam, pPacket->nParamLen);
                }
                else
                {
                    pPacket->nStatus = ADI_RESPONSE_RADIO_ERROR;
                }

                break;
        }
    }
    else
    {
        pPacket->nStatus = ADI_RESPONSE_RADIO_ERROR;
    }

    /* Check the status */
    switch(pPacket->nStatus)
    {
        case 0x00u:                    return BLE_RESPONSE_SUCCESS_NOEVENT;                break;
        case 0x01u:                    return BLE_RESPONSE_FAILURE;                        break;
        case 0x02u:                    return BLE_RESPONSE_SUCCESS_EVENT;                  break;
        case 0x03u:                    return BLE_RESPONSE_BUSY;                           break;
        case 0x04u:                    return BLE_RESPONSE_INSUFFICENT_AUTHENTICATION;     break;
        case 0x05u:                    return BLE_RESPONSE_INSUFFICENT_AUTHORIZATION;      break;
        case 0x06u:                    return BLE_RESPONSE_INSUFFICENT_ENCRYPTION;         break;
        case 0x07u:                    return BLE_RESPONSE_INSUFFICENT_ENCRYPT_KEY_SIZE;   break;
        case 0x10u:                    return BLE_RESPONSE_INVALID_PARAMS;                 break;
        case 0x11u:                    return BLE_RESPONSE_NO_RESOURCES;                   break;
        case 0x12u:                    return BLE_RESPONSE_NOT_SUPPORTED;                  break;
        default:                       return BLE_RADIO_ERROR_PROCESSING;                  break;
    }
}


static ADI_BLER_EVENT bler_handle_event(ADI_BLE_EVENT_PACKET * pPacket)
{
    /* Handle each event individually */
    switch(pPacket->nAciEventCode)
    {
        case CORE_HARDWARE_ERROR:

            if (ADI_BLER_HW_ERROR_DATA_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->nHardwareError = pPacket->pParam[0];

            break;

        case CORE_VENDOR_COMMAND_COMPLETE:

            if (ADI_BLER_VENDOR_DATA_MIN_SIZE > pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sVendorData.nVendorOpCode = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            pBLERadio->sVendorData.nDataLen      = pPacket->pParam[2u];
            pBLERadio->sVendorData.pData         = &pPacket->pParam[3u];

            break;

        case GAP_EVENT_INQUIRYRESULT:
        case GAP_EVENT_INQUIRYRESULT_EXT:

            if (ADI_BLER_INQUIRY_DATA_MIN_SIZE > pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sInquiryData.sRemoteAddress.nAddrType = pPacket->pParam[0u];
            memcpy(pBLERadio->sInquiryData.sRemoteAddress.aBD_ADDR, &pPacket->pParam[1u], 6u);
            pBLERadio->sInquiryData.eRemoteMode = (ADI_BLE_GAP_MODE) pPacket->pParam[7u];
            pBLERadio->sInquiryData.nRssi = (int8_t) pPacket->pParam[8u];
            pBLERadio->sInquiryData.sBuffer.nDataLen = pPacket->pParam[9u];
            pBLERadio->sInquiryData.sBuffer.pData = &pPacket->pParam[10u];

            break;

        case GAP_EVENT_CONNECTED:
        case GAP_EVENT_DISCONNECTED:
        case GAP_EVENT_CONNECTION_UPDATED:

            if (ADI_BLER_CONN_INFO_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->eBleConnInfo.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            pBLERadio->eBleConnInfo.eCurrentRole = (ADI_BLE_CONN_ROLE) pPacket->pParam[2u];
            pBLERadio->eBleConnInfo.eConnAddr.nAddrType = pPacket->pParam[3u];
            memcpy(pBLERadio->eBleConnInfo.eConnAddr.aBD_ADDR, &pPacket->pParam[4u], 6u);
            pBLERadio->eBleConnInfo.nDisConnReason = pPacket->pParam[10u];
            pBLERadio->eBleConnInfo.nConnInterval = (((uint16_t) pPacket->pParam[12u]) << 8u) | ((uint16_t) pPacket->pParam[11u]);
            pBLERadio->eBleConnInfo.nConnLatency = (((uint16_t) pPacket->pParam[14u]) << 8u) | ((uint16_t) pPacket->pParam[13u]);
            pBLERadio->eBleConnInfo.nConnTimeoutParam = (((uint16_t) pPacket->pParam[16u]) << 8u) | ((uint16_t) pPacket->pParam[15u]);

            break;

        case GAP_EVENT_NAME_INFORMATION:
        case GAP_EVENT_LOCAL_NAMEUPDATE:

            if (ADI_BLER_NAME_DATA_MIN_SIZE > pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sNameBuffer.nDataLen = pPacket->nParamLen;
            pBLERadio->sNameBuffer.pData    = pPacket->pParam;

            break;

        case GAP_EVENT_OBS_MODE_DATA:

            if (ADI_BLER_OBSERVER_DATA_MIN_SIZE > pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sObserverData.sRemoteAddress.nAddrType = pPacket->pParam[0u];
            memcpy(pBLERadio->sObserverData.sRemoteAddress.aBD_ADDR, &pPacket->pParam[1u], 6u);
            pBLERadio->sObserverData.nRssi = (int8_t) pPacket->pParam[7u];
            pBLERadio->sObserverData.sBuffer.nDataLen = pPacket->pParam[8u];
            pBLERadio->sObserverData.sBuffer.pData = &pPacket->pParam[9u];

            break;

        case SM_EVENT_PAIRING_REQUEST:

            if (ADI_BLER_PAIR_REQUEST_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sPairInfo.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            pBLERadio->sPairInfo.sm_event_data.RemotePairCapabilities = pPacket->pParam[2u];

            break;

        case SM_EVENT_PASSKEY_REQUEST:

            if (ADI_BLER_PASSKEY_REQUEST_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sPairInfo.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);

            break;

        case SM_EVENT_PASSKEY_DISPLAY:

            if (ADI_BLER_PASSKEY_DISPLAY_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sPairInfo.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            memcpy(&pBLERadio->sPairInfo.sm_event_data.DisplayPasskey[0], &pPacket->pParam[2u], 6u);

            break;

        case SM_EVENT_PAIRING_COMPLETE:

            if (ADI_BLER_PAIR_COMPLETE_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sPairInfo.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            pBLERadio->sPairInfo.sm_event_data.PairFailure = pPacket->pParam[2u];

            break;

        case SM_EVENT_GEN_RANDOM_ADDR_RSP:

            if (ADI_BLER_GEN_RANDOM_ADDR_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            memcpy(pBLERadio->sRandomAddress.aBD_ADDR, pPacket->pParam, 6u);

            break;

        case IMMEDIATE_ALERT_EVENT:

            /* Cache the data in the radio structure if we received the right amount of data */
            if (ADI_BLER_IMM_ALERT_DATA_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sImmAlertLevel.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            pBLERadio->sImmAlertLevel.eAlertLevel = (ADI_BLE_ALERT_LEVEL) pPacket->pParam[2u];

            break;

        case LINKLOSS_ALERT_EVENT:

            /* Cache the data in the radio structure if we received the right amount of data */
            if (ADI_BLER_LINKLOSS_DATA_SIZE != pPacket->nParamLen)
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }

            pBLERadio->sLinkLossLevel.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            memcpy(pBLERadio->sLinkLossLevel.aBD_ADDR, &pPacket->pParam[2u], 6u);
            pBLERadio->sLinkLossLevel.eAlertLevel = (ADI_BLE_ALERT_LEVEL) pPacket->pParam[8u];

            break;

	case DATA_EXCHANGE_RX_EVENT:

            /* Cache the data in the radio structure if we received the right amount of data */
            if ((ADI_DATAEX_MAX_RXPKT_LEN < pBLERadio->eventPkt[9]) || (ADI_BLER_RX_DATA_MIN_SIZE > pPacket->nParamLen))
            {
                return BLE_RADIO_ERROR_PROCESSING;
            }
              
            pBLERadio->rxDataPkt.nConnHandle = (((uint16_t) pPacket->pParam[1u]) << 8u) | ((uint16_t) pPacket->pParam[0u]);
            pBLERadio->rxDataPkt.uPktLen = pBLERadio->eventPkt[9];
            memcpy(&pBLERadio->rxDataPkt.rxPkt[0],&pBLERadio->eventPkt[10],pBLERadio->eventPkt[9]);

	    break;

        default:

        break;
    }

    /* Check the event status...hardware errors always return failure, so return the event code instead for that case */
    if ((pPacket->nStatus != ADI_RESPONSE_SUCCESS) && (pPacket->nAciEventCode != CORE_HARDWARE_ERROR))
    {
        return BLE_RADIO_ERROR_PROCESSING;
    }
    else
    {
        return (ADI_BLER_EVENT) pPacket->nAciEventCode;
    }
}


/*! \endcond */


/**
 * @brief       Application Event Interface
 *
 * @details     This function retrieves, parses, and handles an event received. The 
 *              application is notified of an event through the callback registered
 *              in #adi_radio_Init. After getting this notification, the application
 *              should call this function as soon as possible. 
 *
 * @return      The event received.
 *
 * @note        This function should never return #BLE_RADIO_EVENT_DISPATCH since that is
 *              used to indicate no event in particular.
 *
 */
ADI_BLER_EVENT adi_radio_GetEvent(void)
{
    ADI_BLE_TRANSPORT_RESULT eTransportResult;
    ADI_BLE_EVENT_PACKET     sParsedEventPacket;
    ADI_BLER_RESULT          eResult;
    uint8_t                  nBytes;

    ADI_BLE_LOCK_RADIO(ADI_BLER_LOCK_RADIO_TIMEOUT);

    /* Clear the buffer that holds the event */
    memset(pBLERadio->eventPkt, 0u, ADI_RADIO_RES_HEADER_LEN);

    /* Read event packet */
    eTransportResult = adi_tal_Read(pBLERadio->eventPkt, ADI_CMDPKT_SIZE, &nBytes);

    ADI_BLE_UNLOCK_RADIO();

    /* If the transport layer failed, do not parse */
    if ((eTransportResult != ADI_BLE_TRANSPORT_SUCCESS) || (nBytes == 0u))
    {
        return BLE_RADIO_ERROR_READING;
    }

    /* Parse event packet */
    eResult = bler_parse_packet(&sParsedEventPacket, nBytes);

    /* If parsing failed, do not handle */
    if (eResult != ADI_BLER_SUCCESS)
    {
        return BLE_RADIO_ERROR_PARSING;
    }

    /* Handle event packet */
    return bler_handle_packet(&sParsedEventPacket);
}


/*********************************************************************************
                             GET EVENT DATA
*********************************************************************************/


/**
 * @brief       Get Observer Data
 *
 * @details     This funtion is used to get the most recent observer packet.
 *
 * @param [out] pObserverData : Pointer to the result. Allocated by caller.     
 *
 * @note         It is recommended that the application calls this function after the 
 *               following events are received in #adi_radio_GetEvent:
 *                  - #GAP_EVENT_OBS_MODE_DATA
 *
 */
void adi_ble_GetObserverData(ADI_BLER_OBSERVER_DATA * pObserverData)
{
    ASSERT(pObserverData != NULL);

    memcpy(pObserverData, &pBLERadio->sObserverData, sizeof(ADI_BLER_OBSERVER_DATA));
}


/**
 * @brief       Get Inquiry Data
 *
 * @details     This funtion is used to get the most recent inquiry packet.
 *
 * @param [out] pInquiryData : Pointer to the result. Allocated by caller.           
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *                  - #GAP_EVENT_INQUIRYRESULT
 *                  - #GAP_EVENT_INQUIRYRESULT_EXT
 *
 */
void adi_ble_GetInquiryData(ADI_BLER_INQUIRY_DATA * pInquiryData)
{
    ASSERT(pInquiryData != NULL);

    memcpy(pInquiryData, &pBLERadio->sInquiryData, sizeof(ADI_BLER_INQUIRY_DATA));
}


/**
 * @brief       Get Random Address
 *
 * @details     This funtions is used to get the randomly generated address.
 *
 * @param [out] pRandomAddress : Pointer to the result. Allocated by caller. 
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *                  - #SM_EVENT_GEN_RANDOM_ADDR_RSP
 */
void adi_ble_GetRandomAddress(ADI_BLER_CONFIG_ADDR * pRandomAddress)
{
    ASSERT(pRandomAddress != NULL);

    memcpy(pRandomAddress, &pBLERadio->sRandomAddress, sizeof(ADI_BLER_CONFIG_ADDR));
}


/**
 * @brief       Get Vendor Data
 *
 * @details     This funtions is used to get the vendor specific data.
 *
 * @param [out] pVendorData : Pointer to the result. Allocated by caller. 
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *                  - #CORE_VENDOR_COMMAND_COMPLETE
 *
 */
void adi_ble_GetVendorData(ADI_BLE_VENDOR_DATA * pVendorData)
{
    ASSERT(pVendorData != NULL);

    memcpy(pVendorData, &pBLERadio->sVendorData, sizeof(ADI_BLE_VENDOR_DATA));
}


/**
 * @brief       Get Pairing Information
 *
 * @details     This funtions is used to get the pairing information received in an SMP event.
 *
 * @param [out] pPairInfo : Pointer to the result. Allocated by caller. 
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *                  - #SM_EVENT_PAIRING_REQUEST 
 *                  - #SM_EVENT_PASSKEY_REQUEST 
 *                  - #SM_EVENT_PASSKEY_DISPLAY 
 *                  - #SM_EVENT_PAIRING_COMPLETE
 *
 */
void adi_ble_GetPairingInfo(ADI_BLER_PAIR_INFO * pPairInfo)
{
    ASSERT(pPairInfo != NULL);

    memcpy(pPairInfo, &pBLERadio->sPairInfo, sizeof(ADI_BLER_PAIR_INFO));
}


/**
 * @brief       Get Connection Information
 *
 * @details     This funtions is used to get the information about the current connection.
 *
 * @param [out] pConnectionInfo : Pointer to the result. Allocated by caller. 
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *                  - #GAP_EVENT_CONNECTED
 *                  - #GAP_EVENT_DISCONNECTED
 *                  - #GAP_EVENT_CONNECTION_UPDATED
 *
 */
void adi_ble_GetConnectionInfo(ADI_BLER_CONN_INFO * pConnectionInfo)
{
    ASSERT(pConnectionInfo != NULL);

    memcpy(pConnectionInfo, &pBLERadio->eBleConnInfo, sizeof(ADI_BLER_CONN_INFO));
}


/**
 * @brief      Get Alert Level for the immedaite alert service
 *
 * @details    This funtions is used to get the most recent alert level for the immediate alert service.
 *
 * @return     The current alert level.
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *              - #IMMEDIATE_ALERT_EVENT
 */
ADI_BLE_ALERT_LEVEL adi_ble_GetImmediateAlertLevel(void)
{
    return (pBLERadio->sImmAlertLevel.eAlertLevel);
}

/**
 * @brief      Get Alert Level for the link loss service
 *
 * @details    This funtions is used to get the most recent alert level.
 *
 * @return     The current alert level.
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *              - #LINKLOSS_ALERT_EVENT
 */
ADI_BLE_ALERT_LEVEL adi_ble_GetLinkLossAlertLevel(void)
{
    return (pBLERadio->sLinkLossLevel.eAlertLevel);
}

/**
 * @brief       Get Name Buffer
 *
 * @details     This funtions is used to get the name information buffer.
 *
 * @param [out] pNameBuffer : Pointer to the result. Allocated by caller. 
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *                  - #GAP_EVENT_NAME_INFORMATION
 *                  - #GAP_EVENT_LOCAL_NAMEUPDATE
 *
 */
void adi_ble_GetNameBuffer(ADI_BLER_BUFFER * pNameBuffer)
{
    ASSERT(pNameBuffer != NULL);

    memcpy(pNameBuffer, &pBLERadio->sNameBuffer, sizeof(ADI_BLER_BUFFER));
}

/**
 * @brief       Get Authentication Payload Timeout
 *
 * @details     This funtions is used to get the authentication payload timeout
 *
 * @param [out] pTimeout : Pointer to the result. Allocated by caller. 
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *                  - #GAP_EVENT_NAME_INFORMATION
 *                  - #GAP_EVENT_LOCAL_NAMEUPDATE
 *
 */
void adi_ble_GetAuthTimeout(uint32_t * pTimeout)
{
    ASSERT(pTimeout != NULL);

    *pTimeout = pBLERadio->nAuthenticationTimeout;
}

/**
 * @brief      Get Hardware Errors
 *
 * @details    This funtions is used to get the most recent hardware error.
 *
 * @return     Hardware error code.
 *
 * @note        It is recommended that the application calls this function after the 
 *              following events are received in #adi_radio_GetEvent:
 *              - #CORE_HARDWARE_ERROR
 */
uint8_t adi_ble_GetHardwareErrors(void)
{
    return (pBLERadio->nHardwareError);
}


/**
 * @brief       Get Data Exchange received data
 *
 * @details     This function returns the latest Received data for the Data Exchange RX channel.
 *
 * @param [out] pRxData : Pointer to the data to be received  
 *
 *
 */
void adi_ble_GetDataExchangeRxData(ADI_DATAEX_RX_DATA * pRxData)
{
    ASSERT(pRxData != NULL);
    memcpy(pRxData, &pBLERadio->rxDataPkt, sizeof(ADI_DATAEX_RX_DATA));
}


/*********************************************************************************
                             HIGHER ORDER
*********************************************************************************/


/**
 * @brief       Get Most Recent Command
 *
 * @details     Get the most recent command sent to the radio.
 *
 * @return      ADI_BLER_CMD_OPCODE
 *
 */
ADI_BLER_CMD_OPCODE adi_ble_GetCurCmd(void)
{
    return(pBLERadio->curCmdOpCode);
}


/*@}*/
