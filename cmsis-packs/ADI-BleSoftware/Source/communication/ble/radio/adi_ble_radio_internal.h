/*!
 *****************************************************************************
  @file:    adi_ble_radio_internal.h

  @brief:   Header that includes the internal radio definitons

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


#ifndef ADI_BLE_RADIO_INTERNAL_H
#define ADI_BLE_RADIO_INTERNAL_H


/*! \cond PRIVATE */


#include <radio/adi_ble_radio.h>
#include <rtos_map/adi_rtos_map.h>
#include <radio/adi_ble_logevent.h>
#include <common/adi_error_handling.h>
#include <transport/adi_ble_transport.h>


/*! Maximum number of bytes in a single write operation */
#define ADI_CMDPKT_SIZE                         (64u)

/*! Maximum number of bytes in a single read operation */
#define ADI_EVTPKT_SIZE                         (64u)

/*! Opcode to specify ACI */
#define ADI_VNDR_CMDOPCODE                      (0xFCC0)

/*! Opcode to specify ACI Byte 0 */
#define ADI_VNDR_CMDOPCODE_BYTE0                (0xC0u)

/*! Opcode to specify ACI Byte 1 */
#define ADI_VNDR_CMDOPCODE_BYTE1                (0xFCu)

/*! Minimum command opcode  value */
#define ADI_BLE_MIN_CMD_OPCODE                  (0x100)   

/*! Maximum command opcode value  */
#define ADI_BLE_MAX_CMD_OPCODE                  (0x807)   

/*! Event code for the response packet */
#define ADI_BLE_EVENT_RESP_CODE                 (0x0Eu)   

/*! Event code for the event packet */
#define ADI_BLE_EVENT_EVT_CODE                  (0xFFu)   

/*! ACI event code to identify an event packet */
#define ADI_BLE_ACI_EVT_CODE                    (0xA0u)


/**
 * \enum ADI_BLE_DEV_STATE
 * \brief BLE device state
 */
typedef enum 
{
    DEV_STATE_IDLE = 0,          /*!< Radio Interface layer can new accept command */
    DEV_STATE_INITIALIZED,       /*!< Radio Interface layer can new accept command */
    DEV_STATE_CLOSING,           /*!< Radio Interface layer can new accept command */
} ADI_BLE_DEV_STATE;


/**
 * \enum ADI_HCI_PACKET_TYPE 
 * \brief specifies the packet types for host controller interface packets 
 */
typedef enum
{
    HCI_PKT_TYPE_CMD              = 0x1,              /*!< HCI command type          */
    HCI_PKT_TYPE_RESPONSE         = 0x2,              /*!< HCI command response      */
    HCI_PKT_TYPE_SYN_DATA         = 0x3,              /*!< HCI synchronous data type */
    HCI_PKT_TYPE_EVENT            = 0x4,              /*!< HCI event type            */

} ADI_HCI_PACKET_TYPE;


/**
 * \brief ADI_BLE_RADIO_Device structure contains the critical device specific elements of
 * the bluetooth radio interface layer.
 */
typedef struct 
{
    /* State of the companion module */
    ADI_BLE_DEV_STATE      eBleDeviceState;              /*!< Current state of the device      */
    ADI_BLE_ROLE           eBleDevRole;                  /*!< BLE device role                  */
    ADI_BLE_GAP_MODE       eBleGapMode;                  /*!< BLE device GAP mode              */
    ADI_BLER_CMD_OPCODE    curCmdOpCode;                 /*!< Current command opcode           */

    /* Working memory */
    uint8_t                cmdPkt     [ADI_CMDPKT_SIZE]; /*!< Command packet                    */
    uint8_t                eventPkt   [ADI_CMDPKT_SIZE]; /*!< Event packet                      */
  
    /* Response pointer registration */
    ADI_BLER_BUFFER        responseData;                 /*!< Where the response data should go */

    /* Event memory caching */
    ADI_BLER_OBSERVER_DATA          sObserverData;                /*!< Most recent observer packet        */
    ADI_BLER_INQUIRY_DATA           sInquiryData;                 /*!< Most recent inquiry packet         */
    ADI_BLER_CONFIG_ADDR            sRandomAddress;               /*!< Random address received            */
    ADI_BLE_VENDOR_DATA             sVendorData;                  /*!< Vendor specific data               */
    ADI_BLER_PAIR_INFO              sPairInfo;                    /*!< Pairing information                */
    ADI_BLER_CONN_INFO              eBleConnInfo;                 /*!< Current connection info            */
    ADI_BLE_IMMEDIATE_ALERT_DATA    sImmAlertLevel;               /*!< Memory for findme profile data     */
    ADI_BLE_LINKLOSS_DATA           sLinkLossLevel;               /*!< Memory for proximity profile data  */
    ADI_BLER_BUFFER                 sNameBuffer;                  /*!< Get name buffer                    */
    uint8_t                         nHardwareError;               /*!< Hardware error code                */
    ADI_DATAEX_RX_DATA              rxDataPkt;                    /*!< Rx Data packet for Data Exchange   */
    uint32_t                        nAuthenticationTimeout;       /*!< Authentication Payload Timeout     */
} ADI_BLE_RADIO_Device;


static inline void SET_CMDPKT_OPCODE(uint8_t * pBuffer)
{
    *pBuffer = HCI_PKT_TYPE_CMD;
}


static inline void SET_CMDPKT_VENDORCODE(uint8_t * pBuffer)
{    
    *((uint16_t*)(pBuffer+1)) = (uint16_t)ADI_VNDR_CMDOPCODE;
}


static inline void SET_CMDPKT_LEN(uint8_t * pBuffer, uint8_t pktLen)
{
   *(pBuffer + 3u)  = pktLen;
}


static inline void SET_CMDPKT_CMDCODE(uint8_t * pBuffer, ADI_BLER_CMD_OPCODE cmdOpCode)
{    
    *((uint16_t*)(pBuffer+4)) = (uint16_t)cmdOpCode;
}


/*! Command header: 1 byte packet type, 2 byte vendor opcode, 1 byte parameter len, 2 byte ACI opcode */
#define ADI_RADIO_CMD_HEADER_LEN    (6u)   

/*! Response header: 1 byte packet type, 1 byte event code, 1 byte parameter len, 1 byte num commands, 2 byte vendor opcode, 2 byte ACI opcode, 1 byte status */
#define ADI_RADIO_RES_HEADER_LEN    (9u)

/*! Event header: 1 byte packet type, 1 byte event code, 1 byte parameter len, 1 byte ACI event code, 2 byte ACI event opcode, 1 byte status */
#define ADI_RADIO_EVT_HEADER_LEN    (7u)


/*! \endcond */


#endif /* ADI_BLE_RADIO_INTERNAL_H */

/* @} */
