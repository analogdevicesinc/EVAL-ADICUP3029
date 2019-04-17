/*!
 *****************************************************************************
  @file:    adi_ble_radio.h

  @brief:   This header file contains the radio specific interface command opcodes,
            status codes and radio interface specific functions for BLE radio.
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
 *  @addtogroup radio Radio Module
 *  @{
 */


#ifndef ADI_BLE_RADIO_H
#define ADI_BLE_RADIO_H


/*! \cond PRIVATE */


#include <stdint.h>
#include <stdbool.h>
#include <adi_callback.h>
#include <adi_ble_config.h>


/*! \endcond */
  
/*! Size of the minimum payload received in a #CORE_VENDOR_COMMAND_COMPLETE */
#define ADI_BLER_VENDOR_DATA_MIN_SIZE   (3u)

/*! Size of the payload received in a #CORE_HARDWARE_ERROR */
#define ADI_BLER_HW_ERROR_DATA_SIZE     (1u)

/*! Size of the payload received in a #GAP_EVENT_LOCAL_NAMEUPDATE */
#define ADI_BLER_NAME_DATA_MIN_SIZE     (1u)

/*! Max time (ms) a command will wait for a response or event before a timeout occurs */
#define ADI_BLER_CMD_TIMEOUT            (1000u)

/*! Max time (ms) the radio can be locked for before a timeout occurs */ 
#define ADI_BLER_LOCK_RADIO_TIMEOUT     (1000u)

/*! Max data exchange data length */
#define ADI_DATAEX_MAX_RXPKT_LEN        (20u)

/*! Size of #ADI_BLER_CONFIG_ADDR */
#define ADI_BLER_CONFIG_ADDR_SIZE       (7u)

/*! Maximum size of #ADI_BLER_CONN_LIST which is returned in #adi_radio_GetConnectionList */
#define ADI_BLER_CONN_LIST_SIZE         (1u + (2u*ADI_BLE_MAX_CONNECTION))

/*! Size of #ADI_BLER_CONT_VERSION */
#define ADI_BLER_CONT_VERSION_SIZE      (4u)

/*! Size of #ADI_BLER_CONN_INFO */
#define ADI_BLER_CONN_INFO_SIZE         (17u)

/*! Size of data in #SM_EVENT_PAIRING_REQUEST event */
#define ADI_BLER_PAIR_REQUEST_SIZE      (3u)

/*! Size of data in #SM_EVENT_PAIRING_COMPLETE event */
#define ADI_BLER_PAIR_COMPLETE_SIZE     (3u)

/*! Size of data in #SM_EVENT_PASSKEY_REQUEST event */
#define ADI_BLER_PASSKEY_REQUEST_SIZE   (2u)

/*! Size of data in #SM_EVENT_PASSKEY_DISPLAY event */
#define ADI_BLER_PASSKEY_DISPLAY_SIZE   (8u)

/*! Size of data in #SM_EVENT_GEN_RANDOM_ADDR_RSP event */
#define ADI_BLER_GEN_RANDOM_ADDR_SIZE   (6u)

/*! Minimum length of inquiry data packet received in a #GAP_EVENT_OBS_MODE_DATA event */
#define ADI_BLER_OBSERVER_DATA_MIN_SIZE (9u)

/*! Minimum length of inquiry data packet received in a #GAP_EVENT_INQUIRYRESULT or #GAP_EVENT_INQUIRYRESULT_EXT event */
#define ADI_BLER_INQUIRY_DATA_MIN_SIZE  (10u)

/*! Size of the payload received in a #IMMEDIATE_ALERT_EVENT */
#define ADI_BLER_IMM_ALERT_DATA_SIZE    (3u)

/*! Size of the payload received in a #LINKLOSS_ALERT_EVENT */
#define ADI_BLER_LINKLOSS_DATA_SIZE     (9u)

/*! Size of the minimum payload received in a #DATA_EXCHANGE_RX_EVENT */
#define ADI_BLER_RX_DATA_MIN_SIZE       (3u)

/*! Lock the radio as it is not thread safe */
#define ADI_BLE_LOCK_RADIO(timeout)  

/*! Unlock the radio */
#define ADI_BLE_UNLOCK_RADIO()      

/*! Wait for the current command to complete */
#define ADI_BLE_WAIT_FOR_COMPLETION(eventbits,timeout) adi_ble_WaitForResponseWithTimeout(eventbits, timeout);  

/*! Start the radio command */
#define ADI_BLE_RADIO_CMD_START(x) 

/*! Clean up the radio command */
#define ADI_BLE_RADIO_CMD_END() 

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \enum ADI_BLER_CMD_OPCODE 
 * \brief specifies command opcodes for radio firmware
 */
typedef enum
{
    /* Core commands */
    CMD_BLEM_INIT                         = 0x100,        /*!< Initialize Bluetooth Low energy module                     */
    CMD_BLEM_GET_STACK_VER                = 0x101,        /*!< Get core stack version                                     */
    CMD_BLEM_GET_CONTR_VER                = 0x102,        /*!< Get controller version                                     */
    CMD_BLEM_HCI_SND_VNDR_CMD             = 0x103,        /*!< Send vendor specific command                               */

    /* GAP commands */
    CMD_BLEGAP_REGISTER_DEVICE            = 0x200,        /*!< Register Bluetooth Low energy device                       */
    CMD_BLEGAP_SWITCH_ROLE                = 0x201,        /*!< Command to switch the device role                          */
    CMD_BLEGAP_GET_CURROLE                = 0x202,        /*!< Command to get the current role                            */
    CMD_BLEGAP_GET_LOCAL_BD_ADDR          = 0x203,        /*!< Command to get local bluetooth address                     */
    CMD_BLEGAP_SET_MODE                   = 0x204,        /*!< Command to set connectivity mode                           */
    CMD_BLEGAP_SET_MODE_INTERVALS         = 0x205,        /*!< Command to set mode intervals                              */
    CMD_BLEGAP_SET_MODE_INTERVALS_EX      = 0x206,        /*!< Command to set additional mode setting                     */
    CMD_BLEGAP_GET_MODE                   = 0x207,        /*!< Command to get current discoverability mode                */
    CMD_BLEGAP_CONNECT                    = 0x208,        /*!< Command to create BLE connection                           */
    CMD_BLEGAP_CONNECT_INTERVALS          = 0x209,        /*!< Command to create BLE connection with intervals            */
    CMD_BLEGAP_CONNECT_INTERVALS_EX       = 0x20A,        /*!< Command to create BLE connection with intervals            */
    CMD_BLEGAP_CANCEL_CONNECTION          = 0x20B,        /*!< Command to cancel connection in progress                   */
    CMD_BLEGAP_DISCONNECT_CONNECTION      = 0x20C,        /*!< Command to terminate an existing connection                */
    CMD_BLEGAP_UPDATE_CONN_INTERVAL_EX    = 0x20D,        /*!< Command to update connection interval extended             */
    CMD_BLEGAP_UPDATE_CONN_INTERVAL       = 0x20E,        /*!< Command to update connection interval                      */
    CMD_BLEGAP_GET_REMOTE_DEV_NAME        = 0x20F,        /*!< Command to get the remote devices name                     */
    CMD_BLEGAP_GET_CONNECTION_HANDLE      = 0x210,        /*!< Command to get the connection handle                       */
    CMD_BLEGAP_GET_BD_ADDR                = 0x211,        /*!< Command to get the bluetooth device with the handle        */
    CMD_BLEGAP_GET_NUM_ACTIVE_CONNS       = 0x212,        /*!< Command to get the number of active connections            */
    CMD_BLEGAP_START_INQUIRY              = 0x213,        /*!< Command to start inquiry                                   */
    CMD_BLEGAP_CANCEL_INQUIRY             = 0x214,        /*!< Command to cancel inquiry                                  */
    CMD_BLEGAP_GET_ADV_DATABLK            = 0x215,        /*!< Command to retrive specific info from inquiry              */
    CMD_BLEGAP_GET_ADV_DATATYPES          = 0x216,        /*!< Command to parse address data field                        */
    CMD_BLEGAP_START_OBS_PROC             = 0x217,        /*!< Command to start BLE observation procedure                 */
    CMD_BLEGAP_START_OBS_PROC_INTERVAL    = 0x218,        /*!< Command to start BLE observation procedure with interval   */
    CMD_BLEGAP_STOP_OBS_PROC              = 0x219,        /*!< Command to stop observation procedure                      */
    CMD_BLEGAP_SET_LOCAL_BLE_DEVNAME      = 0x21A,        /*!< Command to set bluetooth device name                       */
    CMD_BLEGAP_SET_LOCAL_BLE_DEVNAME_EX   = 0x21B,        /*!< Command to set bluetooth friendly name                     */
    CMD_BLEGAP_SET_LOCAL_APPEARANCE       = 0x21C,        /*!< Command to set the appearance                              */
    CMD_BLEGAP_SET_MANUF_DATA             = 0x21D,        /*!< Command to set manufacture specific data                   */
    CMD_BLEGAP_SET_TX_PWR_LEVEL           = 0x21E,        /*!< Command to set the TX power level                          */
    CMD_BLEGAP_SET_SERV_DATA_VALUE        = 0x21F,        /*!< Command to set service data value                          */
    CMD_BLEGAP_START_BROADCAST            = 0x220,        /*!< Command to start broadcast procedure                       */
    CMD_BLEGAP_STOP_BROADCAST             = 0x221,        /*!< Command to stop broadcast procedure                        */
    CMD_BLEGAP_APPLY_BROADCAST_VALUE      = 0x222,        /*!< Command to apply broadcast value                           */

    /* Security Manager Opcodes */
    CMD_BLESMP_INIT_PAIRING               = 0x300,         /*!< Command to initiate pairing                               */
    CMD_BLESMP_SET_PAIRING_POLICY         = 0x301,         /*!< Command to set pairing policy                             */
    CMD_BLESMP_SET_OOB_DATA               = 0x302,         /*!< Command to set Out of band data                           */
    CMD_BLESMP_CLEAR_OOB_DATA             = 0x303,         /*!< Command to clear out of bound data                        */
    CMD_BLESMP_CONFIRM_PASSKEY            = 0x304,         /*!< Command to confirm passkey                                */
    CMD_BLESMP_REJECT_PASSKEY             = 0x305,         /*!< Command to reject passkey                                 */
    CMD_BLESMP_SET_IO_CAPABILITY          = 0x306,         /*!< Command to set I/O capability                             */
    CMD_BLESMP_SET_MAX_ENCRYPT_SIZE       = 0x307,         /*!< Command to set maximum encryption key size                */
    CMD_BLESMP_GET_LINK_SECURITY_INFO     = 0x308,         /*!< Command to get link security properties                   */
    CMD_BLESMP_IS_DEVICE_BONDED           = 0x309,         /*!< Command to check whether remote device is bonded or not   */
    CMD_BLESMP_UN_BOND                    = 0x30A,         /*!< Command to unbond a remote device                         */
    CMD_BLESMP_SET_RANDOM_ADDRESS         = 0x30B,         /*!< Command to set random address                             */

    /* Test commands */
    CMD_BLETEST_START_RCVR_TEST           = 0x400,         /*!< Command to start receiver test                            */
    CMD_BLETEST_START_XMT_TEST            = 0x401,         /*!< Command to start transmitter test                         */
    CMD_BLETEST_STOP_TEST                 = 0x402,         /*!< Command to stop test in progress                          */

    /* Find me target profile */
    CMD_FM_TARGET_REG                     = 0x600,         /*!< Command to register findme target                         */
    CMD_FM_TARGET_SET_ALERT_LEVEL         = 0x601,         /*!< Command to set the alert level                            */

    /* Proximity Reporter profile */
    CMD_PROX_REPORTER_REG                 = 0x700,         /*!< Command to register proximity reporter                    */
    CMD_PROX_REPORTER_SET_ALERT_LEVEL     = 0x701,         /*!< Command to set the alert level                            */
    CMD_PROX_REPORTER_SET_IMM_ALERT_LEVEL = 0x702,         /*!< Command to set immediate alert level                      */

    /* Data exchange server */
    CMD_DATA_EXCHANGE_SERV_REG            = 0x500,         /*!< Command to register data exchange server                  */
    CMD_DATA_EXCHANGE_SEND_DATA           = 0x501,         /*!< Command to send data via data exchange server             */

} ADI_BLER_CMD_OPCODE;


/**
 * \enum ADI_BLER_EVENT
 * \brief All software driven event codes.
 */
typedef enum
{
    /************************************ SOFTWARE DRIVEN ***********************************/

    /* Event dispatch */
    BLE_RADIO_EVENT_DISPATCH   = 0x000u,       /*!< Used to indicate no event in particular         */

    /* Software Error codes returned by adi_radio_GetEvent */
    BLE_RADIO_ERROR_READING    = 0x001u,       /*!< Could not read anything from the radio          */
    BLE_RADIO_ERROR_PARSING    = 0x002u,       /*!< Could not parse the packet from the radio       */
    BLE_RADIO_ERROR_PROCESSING = 0x003u,       /*!< Could not process the packet from the radio     */
    BLE_RADIO_ERROR_TIMEOUT    = 0x004u,       /*!< Timed out waiting for the packet from the radio */

    /* High order events */
    BLE_PROFILE_TASK_CREATE    =  0x005,       /*!< BLE profile task is create event                */
    BLE_PROFILE_TASK_INIT      =  0x006,       /*!< BLE profile task init event                     */
    BLE_PROFILE_TASK_DESTROY   =  0x007,       /*!< BLE profile task init event                     */

    /****************************** HARDWARE DRIVEN (RESPONSES) *****************************/

    /* Command complete (response) events */
    BLE_RESPONSE_SUCCESS_NOEVENT                =  0x008u,  /*!< Response event was processed successfully, no event is expected                      */
    BLE_RESPONSE_SUCCESS_EVENT                  =  0x009u,  /*!< Response event was processed successfully, another event is expected                 */
    BLE_RESPONSE_FAILURE                        =  0x00Au,  /*!< Response event was processed unsuccessfully or returned false                        */
    BLE_RESPONSE_BUSY                           =  0x00Bu,  /*!< Response event was processed unsuccessfully because another operation is in progress */
    BLE_RESPONSE_INSUFFICENT_AUTHENTICATION     =  0x00Cu,  /*!< Response event was processed unsuccessfully due to insufficent authentication        */
    BLE_RESPONSE_INSUFFICENT_AUTHORIZATION      =  0x00Du,  /*!< Response event was processed unsuccessfully due to insufficent authorization         */
    BLE_RESPONSE_INSUFFICENT_ENCRYPTION         =  0x00Eu,  /*!< Response event was processed unsuccessfully due to insufficent encryption            */
    BLE_RESPONSE_INSUFFICENT_ENCRYPT_KEY_SIZE   =  0x00Fu,  /*!< Response event was processed unsuccessfully due to insufficent encryption key size   */
    BLE_RESPONSE_INVALID_PARAMS                 =  0x010u,  /*!< Response event was processed unsuccessfully due to an invalid parameter.             */
    BLE_RESPONSE_NO_RESOURCES                   =  0x012u,  /*!< Response event was processed unsuccessfully due to no resources                      */
    BLE_RESPONSE_NOT_SUPPORTED                  =  0x013u,  /*!< Response event was processed unsuccessfully due to the command not being supported   */

    /******************************* HARDWARE DRIVEN (EVENTS) *******************************/

    /* Core events */
    CORE_INITIALIZATION_RSP            =  0x100,        /*!< Event following init command to indicate if the stack was initialized */
    CORE_HARDWARE_ERROR                =  0x101,        /*!< Hardware error received from radio                                    */
    CORE_VENDOR_COMMAND_COMPLETE       =  0x102,        /*!< Vendor command complete, may follow a vendor command                  */
    CORE_VENDOR_COMMAND_STATUS         =  0x103,        /*!< Vendor command status, may follow a vendor command                    */
    CORE_VENDOR_COMMAND_EVENT          =  0x104,        /*!< Vendor command event                                                  */

    /* GAP Events */
    GAP_EVENT_INQUIRYRESULT            =  0x200,        /*!< Event after start inquiry. Indicates remote device is found           */
    GAP_EVENT_INQUIRYRESULT_EXT        =  0x201,        /*!< Event that may follow GAP_EVENT_INQUIRYRESULT                         */
    GAP_EVENT_INQUIRYRESULT_COMPLETE   =  0x202,        /*!< Event inquiry completed                                               */
    GAP_EVENT_MODE_CHANGE              =  0x203,        /*!< Event indicating the accessibility mode is changed                    */
    GAP_EVENT_CONNECTED                =  0x204,        /*!< Event indicating connection is completed                              */
    GAP_EVENT_DISCONNECTED             =  0x205,        /*!< Event indicating connection has been disconnected                     */
    GAP_EVENT_CONNECTION_UPDATED       =  0x206,        /*!< Event indicating connection has been updated                          */
    GAP_EVENT_CONNECTION_CANCLED       =  0x207,        /*!< Event indicating connection creation has been canceld                 */
    GAP_EVENT_NAME_INFORMATION         =  0x208,        /*!< Event indicating name information has been received                   */
    GAP_EVENT_OBS_MODE_DATA            =  0x209,        /*!< Event indicating data is observed over air                            */
    GAP_EVENT_START_BROADCAST_RSP      =  0x20A,        /*!< Event indicating broadcast procedure has been started                 */
    GAP_EVENT_STOP_BROADCAST_RSP       =  0x20B,        /*!< Event indicating broadcast procedure has been stopped                 */
    GAP_EVENT_SET_BROADCAST_VLUE       =  0x20C,        /*!< Event indicating broadcast value has been set                         */
    GAP_EVENT_UPDATE_CONN_RSP          =  0x20D,        /*!< Event indicating update has been started                              */
    GAP_EVENT_LOCAL_NAMEUPDATE         =  0x20E,        /*!< Event indicating local name has been updated                          */

    /* Security manager events */
    SM_EVENT_PAIRING_REQUEST           =  0x300,         /*!< Events indicates that remote device has started pairing              */
    SM_EVENT_PASSKEY_REQUEST           =  0x301,         /*!< Events reported when passkey need to be provided                     */
    SM_EVENT_PASSKEY_DISPLAY           =  0x302,         /*!< Events reported when passkey need to be displayed                    */
    SM_EVENT_PAIRING_COMPLETE          =  0x303,         /*!< Events reported when pairing is completed                            */
    SM_EVENT_GEN_RANDOM_ADDR_RSP       =  0x304,         /*!< Events followed by generate random address call                      */

    /* Test events */
    TEST_EVENT_START_RCVR_TEST_RSP     =  0x400,          /*!< Event following the start receiver test                             */
    TEST_EVENT_START_XMT_TEST_RSP      =  0x401,          /*!< Event following the start transmitter test                          */

    /* Profile Events  */
    IMMEDIATE_ALERT_EVENT              =  0x600,          /*!< Immediate alert event                                               */
    LINKLOSS_ALERT_EVENT               =  0x700,          /*!< Link loss alert event                                               */

    /* Data exchange server events */
    DATA_EXCHANGE_RX_EVENT             = 0x500,          /*!< Data received from remote client                                     */
    DATA_EXCHANGE_TX_COMPLETE          = 0x501,          /*!< Transmit data is successful                                          */

} ADI_BLER_EVENT;


/**
 * \enum ADI_BLER_RESPONSE
 * \brief Response code to indicate the result of a command issued by the radio. 
 */
typedef enum
{
    ADI_RESPONSE_SUCCESS, 
    ADI_RESPONSE_FAILED,
    ADI_RESPONSE_PENDING,
    ADI_RESPONSE_BUSY,
    ADI_RESPONSE_INSUFFICENT_AUTHENTICATION,
    ADI_RESPONSE_INSUFFICENT_AUTHORIZATION,
    ADI_RESPONSE_INSUFFICENT_ENCRYPTION,
    ADI_RESPONSE_INSUFFICENT_ENCRYPTION_KEY_SIZE,
    ADI_RESPONSE_INVALID_PARAMS,
    ADI_RESPONSE_NO_RESOURCES,
    ADI_RESPONSE_NOT_SUPPORTED,
    ADI_RESPONSE_RADIO_ERROR,

} ADI_BLER_RESPONSE;

/**
 * \enum ADI_BLE_EVENT_FLAG
 * \brief Flags to indicate what the radio module is waiting for after issuing a command.
 */
typedef enum
{
  ADI_EVENT_FLAG_RESP_SUCCESS    = (1ul << 0u),     /*<! Flag triggered when response is successfully completed */
  ADI_EVENT_FLAG_RESP_FAILURE    = (1ul << 1u),     /*<! Flag triggered when response completed but failed */
  ADI_EVENT_FLAG_RADIO_FAILURE   = (1ul << 2u),     /*<! Flag triggered when radio processing failed */
  ADI_EVENT_FLAG_EVENT_COMPLETE  = (1ul << 3u),     /*<! Flag triggered when event is successfully completed */

} ADI_BLE_EVENT_FLAG;


/**
 * \enum ADI_BLER_RESULT
 * \brief All radio interface layer APIs returns this result code.
 */
typedef enum
{
    ADI_BLER_SUCCESS, /*!< Radio module API succeeded */
    ADI_BLER_FAILURE, /*!< Radio module API failed */

} ADI_BLER_RESULT;


/**
 * \enum ADI_BLER_IO_CAP
 * \brief The IO capabilities a BLE device could have
 */
typedef enum
{
    ADI_BLER_IO_CAP_DISPLAY_ONLY,       /*!< No input, numberic output                                  */
    ADI_BLER_IO_CAP_DISPLAY_YES_NO,     /*!< User can indicate yes or no for input, numeric output      */
    ADI_BLER_IO_CAP_KEYBOARD_ONLY,      /*!< Device has keyboard with 0-9 and a confirmation, and
                                             the user can indicate yes or no for input, no output        */
    ADI_BLER_IO_CAP_NO_IN_NO_OUT,       /*!< No input, no output                                         */
    ADI_BLER_IO_CAP_KEYBOARD_DISPLAY    /*!< Device has keyboard with 0-9 and a confirmation, and
                                             the user can indicate yes or no for input, numeric  output  */
} ADI_BLER_IO_CAP;


/**
 * \enum ADI_BLE_ROLE
 * \brief BLE device role is described by this enumeration
 */
typedef enum
{
   ADI_BLE_ROLE_BROADCASTER = 0x1, /*!< Device role is broadcaster */
   ADI_BLE_ROLE_OBSERVER    = 0x2, /*!< Device role is observer    */
   ADI_BLE_ROLE_PERIPHERAL  = 0x4, /*!< Device role is peripheral  */
   ADI_BLE_ROLE_CENTRAL     = 0x8, /*!< Device role is central     */

} ADI_BLE_ROLE;


/**
 * \enum ADI_BLE_GAP_MODE
 * \brief BLE GAP operational mode is described by this enumeration
 */
typedef enum
{
    ADI_BLE_GAP_MODE_NOTCONNECTABLE      = 0x00u, /*!< GAP mode not connectiable          */
    ADI_BLE_GAP_MODE_CONNECTABLE         = 0x01u, /*!< GAP mode is Connectable            */
    ADI_BLE_GAP_MODE_DIRECT_CONNECTABLE  = 0x02u, /*!< GAP mode Connectable by any device */
    ADI_BLE_GAP_MODE_CONNECTABLE_MASK    = 0x03u, /*!< GAP mode connectiable mask         */
    ADI_BLE_GAP_MODE_NOTDISCOVERABLE     = 0x00u, /*!< GAP mode not discoverable          */
    ADI_BLE_GAP_MODE_DISCOVERABLE        = 0x10u, /*!< GAP mode discoverable              */
    ADI_BLE_GAP_MODE_LIMITEDDISCOVERABLE = 0x20u, /*!< GAP mode limited discoverable      */
    ADI_BLE_GAP_MODE_DISCOVERABLE_MASK   = 0x30u, /*!< GAP mode discoverable mask         */

} ADI_BLE_GAP_MODE;


/**
 * \enum ADI_BLE_INQUIRY_MODE
 * \brief BLE inquiry mode
 */
typedef enum
{
   INQUIRY_MODE_LIMITED = 0x01u, /*!< Limited inquiry mode */
   INQUIRY_MODE_GENERAL = 0x02u  /*!< General inquiry mode */

} ADI_BLE_INQUIRY_MODE;


/**
 * \enum ADI_BLE_GAP_ADV_DATA_TYPE
 * \brief Advertising data fields 
 */
typedef enum
{
    BLE_GAP_ADV_DATA_TYPE_FLAG                       = 0x01u,
    BLE_GAP_ADV_DATA_TYPE_SERVICEUUID16_MORE         = 0x02u,
    BLE_GAP_ADV_DATA_TYPE_SERVICEUUID16_FULL         = 0x03u,
    BLE_GAP_ADV_DATA_TYPE_SERVICEUUID128_MORE        = 0x06u,
    BLE_GAP_ADV_DATA_TYPE_SERVICEUUID128_FULL        = 0x07u,
    BLE_GAP_ADV_DATA_TYPE_SHORTNAME                  = 0x08u,
    BLE_GAP_ADV_DATA_TYPE_COMPLETENAME               = 0x09u,
    BLE_GAP_ADV_DATA_TYPE_TXPOWERLEVEL               = 0x0Au,
    BLE_GAP_ADV_DATA_TYPE_SLAVECONNINTERVAL          = 0x12u,
    BLE_GAP_ADV_DATA_TYPE_SERVICEUUID16SOLICITATION  = 0x14u,
    BLE_GAP_ADV_DATA_TYPE_SERVICEUUID128SOLICITATION = 0x15u,
    BLE_GAP_ADV_DATA_TYPE_SERVICEDATA                = 0x16u,
    BLE_GAP_ADV_DATA_TYPE_PUBLICTARGETADDRESS        = 0x17u,
    BLE_GAP_ADV_DATA_TYPE_RANDOMTARGETADDRESS        = 0x18u,
    BLE_GAP_ADV_DATA_TYPE_APPEARANCE                 = 0x19u,
    BLE_GAP_ADV_DATA_TYPE_VENDORSPECIFIC             = 0xFFu
} ADI_BLE_GAP_ADV_DATA_TYPE;


/**
 * \enum ADI_BLE_GAP_APPEARANCE
 * \brief BLE GAP appearance 
 */
typedef enum
{
    BLE_GAP_APPEARANCE_UNKNOWN                  = 0x0000u,
    BLE_GAP_APPEARANCE_GENERIC_PHONE            = 0x0040u,
    BLE_GAP_APPEARANCE_GENERIC_COMPUTER         = 0x0080u,
    BLE_GAP_APPEARANCE_GENERIC_WATCH            = 0x00C0u,
    BLE_GAP_APPEARANCE_WATCH_SPORTWATCH         = 0x00C1u,
    BLE_GAP_APPEARANCE_GENERIC_CLOCK            = 0x00FFu,
    BLE_GAP_APPEARANCE_GENERIC_DISPLAY          = 0x0140u,
    BLE_GAP_APPEARANCE_GENERIC_REMOTECONTROL    = 0x0180u,
    BLE_GAP_APPEARANCE_GENERIC_EYEGLASSES       = 0x01C0u,
    BLE_GAP_APPEARANCE_GENERIC_TAG              = 0x0200u,
    BLE_GAP_APPEARANCE_GENERIC_KEYRING          = 0x0240u,
    BLE_GAP_APPEARANCE_GENERIC_MEDIAPLAYER      = 0x0280u,
    BLE_GAP_APPEARANCE_GENERIC_BARCODESCANNER   = 0x02C0u,
    BLE_GAP_APPEARANCE_GENERIC_THERMOMETER      = 0x0300u,
    BLE_GAP_APPEARANCE_THERMOMETER_EAR          = 0x0301u,
    BLE_GAP_APPEARANCE_GENERIC_HEARTRATE        = 0x0340u,
    BLE_GAP_APPEARANCE_HEARTRATE_BELT           = 0x0341u,
    BLE_GAP_APPEARANCE_GENERIC_BLOODPRESSURE    = 0x0380u,
    BLE_GAP_APPEARANCE_BLOODPRESSURE_ARM        = 0x0381u,
    BLE_GAP_APPEARANCE_BLOODPRESSURE_WRIST      = 0x0382u,
    BLE_GAP_APPEARANCE_GENERIC_HID              = 0x03C0u,
    BLE_GAP_APPEARANCE_HID_KEYBOARD             = 0x03C1u,
    BLE_GAP_APPEARANCE_HID_MOUSE                = 0x03C2u,
    BLE_GAP_APPEARANCE_HID_JOYSTICK             = 0x03C3u,
    BLE_GAP_APPEARANCE_HID_GAMEPAD              = 0x03C4u,
    BLE_GAP_APPEARANCE_HID_DIGITIZERTABLET      = 0x03C5u,
    BLE_GAP_APPEARANCE_HID_CARDREADER           = 0x03C6u,
    BLE_GAP_APPEARANCE_HID_DIGITALPEN           = 0x03C7u,
    BLE_GAP_APPEARANCE_HID_BARCODESCANNER       = 0x03C8u,

} ADI_BLE_GAP_APPEARANCE;

/**
 * \enum ADI_BLE_CONN_ROLE
 * \brief BLE connection role
 */
typedef enum {
    BLE_CONN_MASTER,
    BLE_CONN_SLAVE,
}
ADI_BLE_CONN_ROLE;


/**
 * \enum ADI_BLE_RANDOM_ADDR_TYPE
 * \brief BLE type of random address
 */
typedef enum
{
    RANDOM_STATIC = 0x1,                 /*!< Not Resolvable and must be regenerated at powerup */
    RANDOM_PRIVATE_RESOLVABLE = 0x2,     /*!< Resolvable and must be regenerated every 15 minutes by the application */
    RANDOM_PRIVEATE_NON_RESOLVABLE = 0x3 /*!< Not Resolvable and must be regerated every 15 minutes by the application */
} ADI_BLE_RANDOM_ADDR_TYPE;


/**
 * @struct ADI_BLE_EVENT_PACKET
 *
 * @brief details packet structure for event packet
 */
typedef struct
{
    uint8_t   nHciEventCode; /*!< HCI event code (constant) */
    uint8_t   nParamLen;     /*!< Number of valid bytes at pParam */
    uint16_t  nAciEventCode; /*!< ACI event code (variable) */
    uint8_t   nStatus;       /*!< Status code */
    uint8_t  *pParam;        /*!< Payload */
} ADI_BLE_EVENT_PACKET;


/*!
 *  @struct ADI_BLER_CONFIG_ADDR
 *
 *  @brief  Bluetooth address specification
 *
 */
typedef struct
{
  uint8_t aBD_ADDR[6u];   /*!< Bluetooth address of the remote device */
  uint8_t nAddrType;      /*!< Bluetooth address device type (PUBLIC or PRIVATE) */

} ADI_BLER_CONFIG_ADDR;


/*!
 *  @struct ADI_BLER_CONN_LIST
 *
 *  @brief  Structure use to get the number of active connections
 *
 */
typedef struct
{
    uint8_t  nConnectionListLen;                      /*!< Number of valid entries in the connection list */
    uint16_t aConnectionList[ADI_BLE_MAX_CONNECTION]; /*!< Connection list */
} ADI_BLER_CONN_LIST;





/*!
 *  @struct ADI_BLER_CONFIG_CONN
 *
 *  @brief  Structure for setting up connection parameters
 *
 */
typedef struct
{
  uint16_t nScanInterval; /*!< Scan Interval = 0.625ms * nScannInterval, valid values [0x0004u:0x4000u] */
  uint16_t nScanWindow;   /*!< Scan Window = 0.625ms * nScanWindow, valid values [0x0004u:0x4000u] */
  uint16_t nConnInterval; /*!< Conn Interval = 1.25ms * nConnInterval, valid values [0x0006u:0x0C80u] */
} ADI_BLER_CONFIG_CONN;


/*!
 *  @struct ADI_BLER_CONT_VERSION
 *
 *  @brief  Structure to store the controller version result in
 *
 */
typedef struct
{
    uint16_t nManufacturerId; /*!< Manufacturer ID */
    uint16_t nLmpSubVersion;  /*!< Link Manager Subversion */
} ADI_BLER_CONT_VERSION;


/*!
 *  @struct ADI_BLER_CONN_INFO
 *
 *  @brief  Bluetooth information about the current connection
 *
 */
typedef struct
{
  uint16_t             nConnHandle;         /*!< Connection handle */
  ADI_BLE_CONN_ROLE    eCurrentRole;        /*!< Current GAP role */
  ADI_BLER_CONFIG_ADDR eConnAddr;           /*!< Remote address */
  uint8_t              nDisConnReason;      /*!< Disconnect reason (TODO: Enumerate) */
  uint16_t             nConnInterval;       /*!< Connection interval */
  uint16_t             nConnLatency;        /*!< Connection latency */
  uint16_t             nConnTimeoutParam;   /*!< Connection timeout */

} ADI_BLER_CONN_INFO;


/*!
 *  @struct ADI_BLER_PAIR_INFO
 *
 *  @brief  Bluetooth information about the current pairing procedure
 *
 */
typedef struct
{
  uint16_t             nConnHandle;            /*!< Connection handle */
  union{
    uint8_t            DisplayPasskey[6]; 	   /*!< Passkey to display. This is valid during the SM_EVENT_PASSKEY_DISPLAY event */
    uint8_t            PairFailure;            /*!< Pairing failure code. This is valid during the SM_EVENT_PAIRING_COMPLETE event */
    uint8_t            RemotePairCapabilities; /*!< Remote device pairing capabilities. This is valid during the SM_EVENT_PAIRING_REQUEST event */
  } sm_event_data;                             /*!< Security manager event data */
} ADI_BLER_PAIR_INFO;


/*!
 *  @struct ADI_BLER_CONFG_CONN_EX
 *
 *  @brief  Structure for updating connection interval with more details
 *
 */
typedef struct
{
  uint16_t nConnIntervalMin; /*!< Min Conn Interval = 1.25 ms * nConnIntervalMin, valid values [0x0006u:0x0C80u] */
  uint16_t nConnIntervalMax; /*!< Max Conn Interval = 1.25 ms * nConnIntervalMax, valid values [0x0006u:0x0C80u] */
  uint16_t nConnLatency;     /*!< Connection latency is number of link layer connection events, valid values [0x0000u:0x01F3u] */
  uint16_t nConnTimeout;     /*!< Connection timeout = 10 ms * nConnTimeout, valid values [0x000Au:0x0C80u] */

} ADI_BLER_CONFG_CONN_EX;


/*!
 * @struct ADI_BLER_BUFFER
 *
 * @brief  Structure for receiving variable payloads from the radio. nDataLen is
 *         the number of valid bytes at pData. Note that pData is only valid until
 *         the next call to #adi_radio_GetEvent, or more simply put, only valid in
 *         the application callback.
 *
 */
typedef struct
{
    void *   pData;    /*!< Pointer to the received data */
    uint32_t nDataLen; /*!< Valid bytes at pData */
} ADI_BLER_BUFFER;


/*!
 * @struct ADI_BLER_OBSERVER_DATA
 *
 * @brief  Structure that contains data returned during an observation procedure. While the remote address 
 *         and RSSI are always present, the remaining advertising data is variable, which is why the buffer
 *         structure is used. The application can parse and filter the advertising data using the respective
 *         APIs to glean more information about the meaning of the data.
 */
typedef struct
{
    ADI_BLER_CONFIG_ADDR sRemoteAddress;  /*!< Remote address of the device observed */
    int8_t               nRssi;           /*!< RSSI of the device observed */
    ADI_BLER_BUFFER      sBuffer;         /*!< Advertising data of the device observed, use #adi_radio_ParseAdvData to learn more */
} ADI_BLER_OBSERVER_DATA;


/*!
 * @struct ADI_BLER_INQUIRY_DATA
 *
 * @brief  Structure that contains data returned during an inquiry procedure. While the remote address,
 *         RSSI, and mode are always present, the remaining advertising data is variable, which is why the buffer
 *         structure is used. The application can parse and filter the advertising data using the respective
 *         APIs to glean more information about the meaning of the data.
 */
typedef struct
{
    ADI_BLER_CONFIG_ADDR sRemoteAddress;  /*!< Remote address of the device found during inquiry */
    ADI_BLE_GAP_MODE     eRemoteMode;     /*!< Remote mode of the device found during inquiry */
    int8_t               nRssi;           /*!< RSSI of the device found during inquiry */
    ADI_BLER_BUFFER      sBuffer;         /*!< Advertising data received during inquiry, use #adi_radio_ParseAdvData API to learn more */
} ADI_BLER_INQUIRY_DATA;


/*!
 * @struct ADI_BLER_CONFIG_SCAN
 *
 * @brief Structure to configure the scan interval and window for starting a observation procedure
 * 
 */
typedef struct
{
    uint16_t nScanInterval; /*!< Scan interval */
    uint16_t nScanWindow;   /*!< Scan window */
} ADI_BLER_CONFIG_SCAN;


/*!
 * \enum ADI_BLE_ALERT_LEVEL
 * 
 * @brief describes the alert level for findme and proximity profiles
 *
 */
typedef enum
{
    ADI_BLE_NO_ALERT_LEVEL      = 0u,
    ADI_BLE_MEDIUM_ALERT_LEVEL  = 1u,
    ADI_BLE_HIGH_ALERT_LEVEL    = 2u,
} ADI_BLE_ALERT_LEVEL;


/*!
 * @struct ADI_BLE_IMMEDIATE_ALERT_DATA
 * 
 * @brief  Data received in an IMMEDIATE_ALERT_EVENT
 *
 */
typedef struct
{
    uint16_t            nConnHandle; /*!< Connection handle */
    ADI_BLE_ALERT_LEVEL eAlertLevel; /*!< Alert level */
} ADI_BLE_IMMEDIATE_ALERT_DATA;


/*!
 * @struct ADI_BLE_LINKLOSS_DATA
 * 
 * @brief  Data received in a #LINKLOSS_ALERT_EVENT
 *
 */
typedef struct
{
    uint16_t            nConnHandle;  /*!< Connection handle */
    uint8_t             aBD_ADDR[6u]; /*!< Remote address */
    ADI_BLE_ALERT_LEVEL eAlertLevel;  /*!< Alert level */
} ADI_BLE_LINKLOSS_DATA;


/*!
 * @struct ADI_BLE_VENDOR_DATA
 * 
 * @brief  Data received in a #CORE_VENDOR_COMMAND_COMPLETE event
 *
 */
typedef struct
{
    uint16_t  nVendorOpCode;  /*!< Vendor specific opcode */
    uint8_t   nDataLen;       /*!< Number of valid bytes at pData */
    uint8_t * pData;          /*!< Vendor payload */
} ADI_BLE_VENDOR_DATA;


/*!
 * @struct ADI_DATAEX_RX_DATA
 * 
 * @brief  Data received in to rxPkt
 *
 */
typedef struct
{
    uint16_t            nConnHandle;                     /*!< Connection handle */
    uint8_t             uPktLen;					     /*!< Length of packet received */
    uint8_t             rxPkt[ADI_DATAEX_MAX_RXPKT_LEN]; /*!< Packet received */
} ADI_DATAEX_RX_DATA;


/* Core */
ADI_BLER_RESULT adi_radio_Init                    (ADI_CALLBACK pCallbackFunc);
ADI_BLER_RESULT adi_radio_GetControllerVersion    (ADI_BLER_CONT_VERSION * const pControllerVersion);
ADI_BLER_RESULT adi_radio_GetCoreStackVersion     (uint32_t * const pVersion);
ADI_BLER_RESULT adi_radio_SendVendorCommand       (const uint16_t nVendorOpCode, const uint8_t nParamLen, uint8_t * const pParams);

/* GAP: General */
ADI_BLER_RESULT adi_radio_RegisterDevice          (const ADI_BLE_ROLE eBleRole);
ADI_BLER_RESULT adi_radio_SwitchRole              (ADI_BLE_ROLE * const peDesiredBleRole);
ADI_BLER_RESULT adi_radio_GetCurrentRole          (ADI_BLE_ROLE * const pCurrentRole);
ADI_BLER_RESULT adi_radio_GetLocalBluetoohAddr    (ADI_BLER_CONFIG_ADDR * const pConfigAddr);
ADI_BLER_RESULT adi_radio_SetMode                 (ADI_BLE_GAP_MODE peGapMode, uint16_t * const pAdvInterval, ADI_BLER_CONFIG_ADDR * const pConfigAddr);
ADI_BLER_RESULT adi_radio_GetMode                 (ADI_BLE_GAP_MODE * const pBleMode);

/* GAP: Central or Peripheral */
ADI_BLER_RESULT adi_radio_Disconnect              (const uint16_t pConnHandle);
ADI_BLER_RESULT adi_radio_UpdateConnectionInterval(const uint16_t nConnHandle, const uint16_t nConnInterval, ADI_BLER_CONFG_CONN_EX * const pConfigConnEx);
ADI_BLER_RESULT adi_radio_GetRemoteDeviceName     (const uint16_t nConnHandle);
ADI_BLER_RESULT adi_radio_GetConnectionHandle     (uint8_t * const pBleAddr, uint16_t * const pConnHandle);
ADI_BLER_RESULT adi_radio_GetConnectionAddr       (const uint16_t pConnHandle, uint8_t * const pAddr);
ADI_BLER_RESULT adi_radio_GetConnectionList       (ADI_BLER_CONN_LIST * pConnectionList);

/* GAP: Central */
ADI_BLER_RESULT adi_radio_Connect                 (ADI_BLER_CONFIG_ADDR * const pConfigAddr, ADI_BLER_CONFIG_CONN * const pConfigConn, uint16_t nConnTimeout);
ADI_BLER_RESULT adi_radio_CancelCreateConnection  (void);
ADI_BLER_RESULT adi_radio_StartInquiry            (const ADI_BLE_INQUIRY_MODE eInquiryMode);
ADI_BLER_RESULT adi_radio_CancelInquiry           (void);

/* GAP: Central or Observer */
ADI_BLER_RESULT adi_radio_ParseAdvData            (ADI_BLER_BUFFER * const pAdvData, uint32_t * const pAdvDataBitField);
ADI_BLER_RESULT adi_radio_FilterAdvData           (ADI_BLER_BUFFER * const pInquiryData, const ADI_BLE_GAP_ADV_DATA_TYPE eBlockType, ADI_BLER_BUFFER * const pFilteredData);

/* GAP: Observer */
ADI_BLER_RESULT adi_radio_StartObsvProc           (bool bDuplicateFilteringMode, ADI_BLER_CONFIG_SCAN * const pScanInfo);
ADI_BLER_RESULT adi_radio_StopObsvProc            (void);

/* GAP: Broadcaster */
ADI_BLER_RESULT adi_radio_SetLocalBluetoothDevName(uint8_t * const pName, uint8_t nNameLen, uint8_t nIsWriteable, uint16_t nMaxWriteableLen);
ADI_BLER_RESULT adi_radio_SetLocalAppearance      (const ADI_BLE_GAP_APPEARANCE eAppearance);
ADI_BLER_RESULT adi_radio_SetMfgSpecificData      (uint8_t * const pValue, const uint8_t nValueLen);
ADI_BLER_RESULT adi_radio_SetTxPowerLevel         (const int8_t nTxPowerLevel);
ADI_BLER_RESULT adi_radio_SetServiceDataValue     (const uint16_t nUUID, uint8_t * const pValue, const uint8_t nValueLen);
ADI_BLER_RESULT adi_radio_StartBroadcastProc      (const uint16_t nBroadcastInterval);
ADI_BLER_RESULT adi_radio_StopBroadcastProc       (void);
ADI_BLER_RESULT adi_radio_ApplyBroadcastValue     (void);

/* FindMe */
ADI_BLER_RESULT adi_radio_Register_FindmeTarget   (void);
ADI_BLER_RESULT adi_radio_FM_SetAlertLevel        (const ADI_BLE_ALERT_LEVEL eAlertLevel);

/* Proximity Reporter */
ADI_BLER_RESULT adi_radio_Register_ProximityReporter(void);
ADI_BLER_RESULT adi_radio_Proximity_SetAlertLevel (const ADI_BLE_ALERT_LEVEL eLinkLossAlertLevel,const bool bLinkLoss);

/* Data Exchange Server*/
ADI_BLER_RESULT adi_radio_Register_DataExchangeServer(void);
ADI_BLER_RESULT adi_radio_DE_SendData             (const uint16_t connHandle, uint8_t txDataLen, uint8_t *ptxData);

/* Test */
ADI_BLER_RESULT adi_radio_StartRecvTest           (const uint8_t rxFrequency);
ADI_BLER_RESULT adi_radio_StartTransmitTest       (const uint8_t txFrequency,uint8_t payloadLen, uint8_t payloadPattern);
ADI_BLER_RESULT adi_radio_StopTest                (void);

/* SMP */
ADI_BLER_RESULT adi_radio_InitiatePairing         (const uint16_t nConnHandle, const uint8_t nBond);
ADI_BLER_RESULT adi_radio_SetPairingPolicy        (const uint8_t nAccept, const uint8_t nBond);
ADI_BLER_RESULT adi_radio_ConfirmPasskey          (const uint16_t nConnHandle, uint8_t * const pPasskey);
ADI_BLER_RESULT adi_radio_RejectPasskey           (const uint16_t nConnHandle);
ADI_BLER_RESULT adi_radio_SetOobData              (uint8_t * const pOobData);
ADI_BLER_RESULT adi_radio_ClearOobData            (void);
ADI_BLER_RESULT adi_radio_SetIOCapability         (const ADI_BLER_IO_CAP eIOCapability);
ADI_BLER_RESULT adi_radio_SetMaxEncryptionKeySize (const uint8_t nKeySize);
ADI_BLER_RESULT adi_radio_GetLinkSecurityProperties(const uint16_t nConnHandle, uint8_t * const pProperties);
ADI_BLER_RESULT adi_radio_IsDeviceBonded          (uint8_t * const pAddr, const ADI_BLE_CONN_ROLE eBleConnRole, uint8_t * const bBonded);
ADI_BLER_RESULT adi_radio_UnBond                  (uint8_t * const pAddr);
ADI_BLER_RESULT adi_radio_GenerateAndSetRandomAddr(const ADI_BLE_RANDOM_ADDR_TYPE eAddrType);

/* Event Interface */
ADI_BLER_EVENT adi_radio_GetEvent(void);

/* Event Data */
void                adi_ble_GetObserverData       (ADI_BLER_OBSERVER_DATA * pObserverData);
void                adi_ble_GetInquiryData        (ADI_BLER_INQUIRY_DATA * pInquiryData);
void                adi_ble_GetRandomAddress      (ADI_BLER_CONFIG_ADDR * pRandomAddress);
void                adi_ble_GetVendorData         (ADI_BLE_VENDOR_DATA * pVendorData);
void                adi_ble_GetPairingInfo        (ADI_BLER_PAIR_INFO * pPairAddress);
void                adi_ble_GetConnectionInfo     (ADI_BLER_CONN_INFO * pConnectionInfo);
ADI_BLE_ALERT_LEVEL adi_ble_GetImmediateAlertLevel(void);
ADI_BLE_ALERT_LEVEL adi_ble_GetLinkLossAlertLevel (void);
void                adi_ble_GetNameBuffer         (ADI_BLER_BUFFER * pNameBuffer);
void                adi_ble_GetAuthTimeout        (uint32_t * pTimeout);
uint8_t             adi_ble_GetHardwareErrors     (void);
void                adi_ble_GetDataExchangeRxData (ADI_DATAEX_RX_DATA * pRxData);

/* Higher Order */
ADI_BLER_CMD_OPCODE adi_ble_GetCurCmd             (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_BLE_RADIO_H */


/* @} */
