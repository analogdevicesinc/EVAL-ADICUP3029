/*!
 *****************************************************************************
   @file:    adi_ble_logevent.h
   @brief:   support functions to log Bluetooth low energy events

   @details:
   @version: $Revision$
   @date:    $Date$
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

*/
/*****************************************************************************/

#ifndef ADI_BLE_LOGEVENT_H
#define ADI_BLE_LOGEVENT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    LOGID_CMD_BLEM_INIT                      = 0x100,        /*!< Initialize Bluetooth Low energy module     */
    LOGID_CMD_BLEM_GET_STACK_VER             = 0x101,        /*!< Get core stack version                     */
    LOGID_CMD_BLEM_GET_CONTR_VER             = 0x102,        /*!< Get controller version                     */
    LOGID_CMD_BLEM_HCI_SND_VNDR_CMD          = 0x103,        /*!< Send vendor specific LOGID_command               */

    /* BLE GAP commands */
    LOGID_CMD_BLEGAP_REGISTER_DEVICE         = 0x200,        /*!< Register Bluetooth Low energy device        */
    LOGID_CMD_BLEGAP_SWITCH_ROLE             = 0x201,        /*!< Command to switch the device role           */
    LOGID_CMD_BLEGAP_GET_CURROLE             = 0x202,        /*!< Command to get the current role             */
    LOGID_CMD_BLEGAP_GET_LOCAL_BD_ADDR       = 0x203,        /*!< Command to get local bluetooth address      */
    LOGID_CMD_BLEGAP_SET_MODE                = 0x204,        /*!< Command to set connectivity mode            */
    LOGID_CMD_BLEGAP_SET_MODE_INTERVALS      = 0x205,        /*!< Command to set mode intervals               */
    LOGID_CMD_BLEGAP_SET_MODE_INTERVALS_EX   = 0x206,        /*!< Command to set additional mode setting      */
    LOGID_CMD_BLEGAP_GET_MODE                = 0x207,        /*!< Command to get current discoverability mode */
    LOGID_CMD_BLEGAP_CONNECT                 = 0x208,        /*!< Command to create BLE connection            */
    LOGID_CMD_BLEGAP_CONNECT_INTERVALS       = 0x209,        /*!< Command to create BLE connection with intervals */
    LOGID_CMD_BLEGAP_CONNECT_INTERVALS_EX    = 0x20A,        /*!< Command to create BLE connection with intervals */
    LOGID_CMD_BLEGAP_CANCEL_CONNECTION       = 0x20B,        /*!< Command to cancel connection in progress    */
    LOGID_CMD_BLEGAP_DISCONNECT_CONNECTION   = 0x20C,        /*!< Command to terminate an existing connection */
    LOGID_CMD_BLEGAP_UPDATE_CONN_INTERVAL_EX = 0x20D,        /*!< Command to update connection interval extended */
    LOGID_CMD_BLEGAP_UPDATE_CONN_INTERVAL    = 0x20E,        /*!< Command to update connection interval       */
    LOGID_CMD_BLEGAP_GET_REMOTE_DEV_NAME     = 0x20F,        /*!< Command to get the remote devices name      */
    LOGID_CMD_BLEGAP_GET_CONNECTION_HANDLE   = 0x210,        /*!< Command to get the connection handle        */
    LOGID_CMD_BLEGAP_GET_BD_ADDR             = 0x211,        /*!< Command to get the bluetooth device with the handle */
    LOGID_CMD_BLEGAP_GET_NUM_ACTIVE_CONNS    = 0x212,        /*!< Command to get the number of active connections */
    LOGID_CMD_BLEGAP_START_INQUIRY           = 0x213,        /*!< Command to start inquiry                    */
    LOGID_CMD_BLEGAP_CANCEL_INQUIRY          = 0x214,        /*!< Command to cancel inquiry                   */
    LOGID_CMD_BLEGAP_GET_ADV_DATABLK         = 0x215,        /*!< Command to retrive specific info from inquiry */
    LOGID_CMD_BLEGAP_GET_ADV_DATATYPES       = 0x216,        /*!< Command to parse address data field         */
    LOGID_CMD_BLEGAP_START_OBS_PROC          = 0x217,        /*!< Command to start BLE observation procedure  */
    LOGID_CMD_BLEGAP_START_OBS_PROC_INTERVAL = 0x218,        /*!< Command to start BLE observation procedure with interval   */
    LOGID_CMD_BLEGAP_STOP_OBS_PROC           = 0x219,        /*!< Command to stop observation procedure       */
    LOGID_CMD_BLEGAP_SET_LOCAL_BLE_DEVNAME   = 0x21A,        /*!< Command to set bluetooth device name        */
    LOGID_CMD_BLEGAP_SET_LOCAL_BLE_DEVNAME_EX= 0x21B,        /*!< Command to set bluetooth friendly name      */
    LOGID_CMD_BLEGAP_SET_LOCAL_APPEARANCE    = 0x21C,        /*!< Command to set the appearance               */
    LOGID_CMD_BLEGAP_SET_MANUF_DATA          = 0x21D,        /*!< Command to set manufacture specific data    */
    LOGID_CMD_BLEGAP_SET_TX_PWR_LEVEL        = 0x21E,        /*!< Command to set the TX power level           */
    LOGID_CMD_BLEGAP_SET_SERV_DATA_VALUE     = 0x21F,        /*!< Command to set service data value           */
    LOGID_CMD_BLEGAP_START_BROADCAST         = 0x220,        /*!< Command to start broadcast procedure         */
    LOGID_CMD_BLEGAP_STOP_BROADCAST          = 0x221,        /*!< Command to stop broadcast procedure         */
    LOGID_CMD_BLEGAP_APPLY_BROADCAST_VALUE   = 0x222,        /*!< Command to apply broadcast value            */

    /* Security Manager Opcodes */
    LOGID_CMD_BLESMP_INIT_PAIRING           = 0x300,         /*!< Command to initiate pairing                  */
    LOGID_CMD_BLESMP_SET_PAIRING_POLICY     = 0x301,         /*!< Command to set pairing policy                */
    LOGID_CMD_BLESMP_SET_OOB_DATA           = 0x302,         /*!< Command to set Out of band data              */
    LOGID_CMD_BLESMP_CLEAR_OOB_DATA         = 0x303,         /*!< Command to clear out of bound data           */
    LOGID_CMD_BLESMP_CONFIRM_PASSKEY        = 0x304,         /*!< Command to confirm passkey                   */
    LOGID_CMD_BLESMP_REJECT_PASSKEY         = 0x305,         /*!< Command to reject passkey                    */
    LOGID_CMD_BLESMP_SET_IO_CAPABILITY      = 0x306,         /*!< Command to set I/O capability                */
    LOGID_CMD_BLESMP_SET_MAX_ENCRYPT_SIZE   = 0x307,         /*!< Command to set maximum encryption key size   */
    LOGID_CMD_BLESMP_GET_LINK_SECURITY_INFO = 0x308,         /*!< Command to get link security properties      */
    LOGID_CMD_BLESMP_IS_DEVICE_BONDED       = 0x309,         /*!< Command to check whether remote device is bonded or not */
    LOGID_CMD_BLESMP_UN_BOND                = 0x30A,         /*!< Command to unbond a remote device            */
    LOGID_CMD_BLESMP_SET_RANDOM_ADDRESS     = 0x30B,         /*!< Command to set random address                */

    /* Test commands */
    LOGID_CMD_BLETEST_START_RCVR_TEST       = 0x400,         /*!< Command to start receiver test               */
    LOGID_CMD_BLETEST_START_XMT_TEST        = 0x401,         /*!< Command to start transmitter test            */
    LOGID_CMD_BLETEST_STOP_TEST             = 0x402,         /*!< Command to stop test in progress             */

    /* Findme target profile commands */

    LOGID_CMD_FM_TARGET_REG                 = 0x600,         /*!< Command to register findme target            */
    LOGID_CMD_FM_TARGET_SET_ALERT_LEVEL     = 0x601,         /*!< Command to set the alert level               */

    /* Proximity Reporter profile */
    LOGID_CMD_PROX_REPORTER_REG             = 0x700,         /*!< Command to register proximity reporter       */
    LOGID_CMD_PROX_REPORTER_SET_ALERT_LEVEL = 0x701,         /*!< Command to set the alert level               */
    LOGID_CMD_PROX_REPORTER_SET_IMM_ALERT_LEVEL = 0x702,     /*!< Command to set immediate alert level         */


    /* *****  E V E N T S *** Task events */
    LOGID_BLE_PROFILE_TASK_CREATE            =  0x8010,        /*!< BLE profile task is create event                             */
    LOGID_BLE_PROFILE_TASK_INIT              =  0x8011,        /*!< BLE profile task init event                                  */
    LOGID_BLE_PROFILE_TASK_DESTROY           =  0x8012,        /*!< BLE profile task init event                                  */

    /* GAP events */
    LOGID_GAP_EVENT_INQUIRYRESULT            =  0x8200,        /*!< Event after start inquiry. Indicates remote device is found */
    LOGID_GAP_EVENT_INQUIRYRESULT_EXT        =  0x8201,        /*!< Event that may follow GAP_EVENT_INQUIRYRESULT               */
    LOGID_GAP_EVENT_INQUIRYRESULT_COMPLETE   =  0x8202,        /*!< Event inquiry completed                                     */
    LOGID_GAP_EVENT_MODE_CHANGE              =  0x8203,        /*!< Event indicating the accessibility mode is changed          */
    LOGID_GAP_EVENT_CONNECTED                =  0x8204,        /*!< Event indicating connection is completed                    */
    LOGID_GAP_EVENT_DISCONNECTED             =  0x8205,        /*!< Event indicating connection has been disconnected           */
    LOGID_GAP_EVENT_CONNECTION_UPDATED       =  0x8206,        /*!< Event indicating connection has been updated                */
    LOGID_GAP_EVENT_CONNECTION_CANCLED       =  0x8207,        /*!< Event indicating connection creation has been canceld       */
    LOGID_GAP_EVENT_NAME_INFORMATION         =  0x8208,        /*!< Event indicating name information has been received         */
    LOGID_GAP_EVENT_OBS_MODE_DATA            =  0x8209,        /*!< Event indicating data is observed over air                  */
    LOGID_GAP_EVENT_START_BROADCAST_RSP      =  0x820A,        /*!< Event indicating broadcast procedure has been started       */
    LOGID_GAP_EVENT_STOP_BROADCAST_RSP       =  0x820B,        /*!< Event indicating broadcast procedure has been stopped       */
    LOGID_GAP_EVENT_SET_BROADCAST_VLUE       =  0x820C,        /*!< Event indicating broadcast value has been set               */
    LOGID_GAP_EVENT_UPDATE_CONN_RSP          =  0x820D,        /*!< Event indicating update has been started                    */
    LOGID_GAP_EVENT_LOCAL_NAMEUPDATE         =  0x820E,        /*!< Event indicating local name has been updated                */

    /* Security manager events */
    LOGID_SM_EVENT_PAIRING_REQUEST          = 0x8300,          /*!< Events indicates that remote device has started pairing     */
    LOGID_SM_EVENT_PASSKEY_REQUEST          = 0x8301,          /*!< Events reported when passkey need to be provided            */
    LOGID_SM_EVENT_PASSKEY_DISPLAY          = 0x8302,          /*!< Events reported when passkey need to be displayed           */
    LOGID_SM_EVENT_PAIRING_COMPLETE         = 0x8303,          /*!< Events reported when pairing is completed                   */
    LOGID_SM_EVENT_GEN_RANDOM_ADDR_RSP      = 0x8304,          /*!< Events followed by generate random address call             */

    /* Test events */
    LOGID_TEST_EVENT_START_RCVR_TEST_RSP    = 0x8400,           /*!< Event following the start receiver test                    */
    LOGID_TEST_EVENT_START_XMT_TEST_RSP     = 0x8401,           /*!< Event following the start transmitter test                 */

    /* Findme target and proximity reporter events */
    LOGID_IMMEDIATE_ALERT_EVENT             = 0x600,            /*!< Immediate alert event                                      */

    /* Proximity reporter event */
    LOGID_LINK_LOSS_ALERT_EVENT             = 0x700,            /*!< Link loss event                                            */

} ADI_BLE_LOG_ID;

#if (ADI_CFG_BLE_LOGEVENT == 1)
#include <stdint.h>

void adi_ble_LogEvent(const ADI_BLE_LOG_ID event);
void adi_ble_LogEventData(const ADI_BLE_LOG_ID event,const uint32_t data);

#define ADI_BLE_LOGEVENT(event)             adi_ble_LogEvent(event)
#define ADI_BLE_LOGEVENT_DATA(event,data)   adi_ble_LogEventData(event,data);

static inline uint32_t ADI_BLE_GET_TIMESTAMP()   {
     return 0;
   } // FIXME

#else
#define ADI_BLE_LOGEVENT(event)
#define ADI_BLE_LOGEVENT_DATA(event,data)  
#endif  /* ADI_CFG_BLE_LOGEVENT */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_BLE_LOGEVENT_H */
