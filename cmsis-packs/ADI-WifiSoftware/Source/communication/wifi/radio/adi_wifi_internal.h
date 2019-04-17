/*!
 *****************************************************************************
  @file:    adi_wifi_internal.h

  @brief:   Header that includes the internal wifi driver 

  @details:
  -----------------------------------------------------------------------------

Copyright (c) 2017 Analog Devices, Inc.

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
 *  @addtogroup WiFi
 *  @{
 */


#ifndef ADI_WIFI_INTERNAL_H
#define ADI_WIFI_INTERNAL_H

#include <wifi/radio/adi_wifi.h>

/*! Maximum number of bytes in a single read/write operation. This is an estimated number. The
 * actual max packet size is 2048 bytes for the ESP8266 so there is a tradeoff between memory allocation
 * and functionality of the radio.
 */
#define ADI_WIFI_MAX_DATA_SIZE                         (400u)

/*! Maximum number of ascii characters that could come in as a message size. The real maximum buffer size for
 *  the ESP8266 is 1024 bytes. This is 4 ascii characters "1" "0" "2" "4". 
 */
#define ADI_WIFI_MAX_DATA_SIZE_ASCII                    (4u)

/*! This is the Maximum packet size for the MQTT request message. This is used to send MQTT packets.
 * Generally no more than 60 bytes is required.  
 */
#define ADI_WIFI_MQTT_PACKET_SIZE                      (400u)

/*! @cond PRIVATE */

/*!
 *  @brief   This structure contains the device specific elements of the Wi-Fi device.
 */
typedef struct 
{
    ADI_WIFI_AT_CMDCODE    	eCurCmdOpCode;                 		  /*!< Current command opcode.           		     	*/
    uint32_t 				nHardwareError;						  /*!< Device Hardware error. 		   			        */
    uint8_t                 pDataPkt[ADI_WIFI_MAX_DATA_SIZE]; 	  /*!< Data packet.                      			    */
    uint16_t				nResponseSize;					 	  /*!< Size of the response currently in pDataPkt.      */
} ADI_WIFI_DEVICE;

/*!
 *  @brief   This structure contains the MQTT specific elements of the Wi-Fi device.
 */
typedef struct
{
    uint8_t					nQosLevel;							         /*!< Quality of service level.			  */
    uint8_t                 pMQTTRequestPkt[ADI_WIFI_MQTT_PACKET_SIZE];  /*!< MQTT request packet.                */
} ADI_MQTT_DEVICE;

/*! @endcond */

#endif /* ADI_WIFI_INTERNAL_H */

/* @} */
