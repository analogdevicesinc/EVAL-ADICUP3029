/*!
 *****************************************************************************
   @file    adi_wifi_logevent.h
   @brief   support functions to log wifi events

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

#ifndef ADI_WIFI_LOGEVENT_H
#define ADI_WIFI_LOGEVENT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    /* Basic AT commands */
    LOGID_CMD_AT,                                 /*!< Startup/Test AT interface.            */
    LOGID_CMD_AT_RST,                             /*!< Restart Module.                       */
    LOGID_CMD_AT_GMR,                             /*!< Get Version Information.              */
    LOGID_CMD_AT_GSLP,                            /*!< Disable Echo.                         */

    /* WiFi layer commands */
    LOGID_CMD_AT_CWMODE,                          /*!< List valid modes.                     */
    LOGID_CMD_AT_CWJAP,                           /*!< Connect to AP.                        */
    LOGID_CMD_AT_CWLAP,                           /*!< List available APs.                   */
    LOGID_CMD_AT_CWQAP,                           /*!< Disconnect from AP.                   */
    LOGID_CMD_AT_CWSAP,                           /*!< Configuration of softAP mode.         */
    LOGID_CMD_AT_CWLIF,                           /*!< List of clients connected to softAP.  */
    LOGID_CMD_AT_CWDHCP,                          /*!< Enable Disable DHCP.                  */
    LOGID_CMD_AT_CIPSTAMAC,                       /*!< Set static MAC station.               */
    LOGID_CMD_AT_CIPAPMAC,                        /*!< Set static MAC softAP.                */
    LOGID_CMD_AT_CIPSTA,                          /*!< Set IP address for station.           */
    LOGID_CMD_AT_CIPAP,                           /*!< Set IP address fori softAP.           */

    /* TCP/IP layer  */
    LOGID_CMD_AT_CIPSTATUS,                       /*!< Information about the connection.     */
    LOGID_CMD_AT_CIPSTART,                        /*!< Start connection.                     */
    LOGID_CMD_AT_CIPSEND,                         /*!< Send Data. (data size)                */
    LOGID_CMD_AT_CIPSEND_CONN,                    /*!< Send Data. (connect)                  */
    LOGID_CMD_AT_CIPSEND_DISCONN,                 /*!< Send Data. (disconnect)               */
    LOGID_CMD_AT_CIPSEND_PUB,                     /*!< Send Data. (publisher)                */
    LOGID_CMD_AT_CIPSEND_PUBREL,                  /*!< Send Data. (PUBREL)                   */
    LOGID_CMD_AT_CIPSEND_SUB,                     /*!< Send Data. (subscriber)               */
    LOGID_CMD_AT_CIPSEND_PINGREQ,                 /*!< Send Data. (PINGREQ)                  */
    LOGID_CMD_AT_CIPCLOSE,                        /*!< Close TCP/UDP connection.             */
    LOGID_CMD_AT_CIFSR,                           /*!< Get local ip address.                 */
    LOGID_CMD_AT_CIPMUX,                          /*!< Enable/Disbale Multiple connections.  */
    LOGID_CMD_AT_CIPSERVER,                       /*!< Configure as server.                  */
    LOGID_CMD_AT_CIPMODE,                         /*!< Set transfer mode.                    */
    LOGID_CMD_AT_CIPSTO,                          /*!< Set server timeout.                   */
    LOGID_CMD_AT_CIUPDATE,                        /*!< Update through network.               */
    LOGID_CMD_AT_IPD,                             /*!< Receive Network data.                 */
	LOGID_CMD_AT_SSLSIZE,                         /*!< Receive Network data.                 */

    LOGID_CMD_AT_INVALID                         /*!< INVALID COMMAND NOT SUPPORTED.         */
} ADI_WIFI_LOG_ID;

#if (ADI_CFG_WIFI_LOGEVENT == 1)
#include <stdint.h>

void adi_wifi_LogEvent(const ADI_WIFI_LOG_ID event);
void adi_wifi_LogEventData(const ADI_WIFI_LOG_ID event,const uint32_t data);

#define ADI_WIFI_LOGEVENT(event)             adi_wifi_LogEvent(event)
#define ADI_WIFI_LOGEVENT_DATA(event,data)   adi_wifi_LogEventData(event,data);

static inline uint32_t ADI_WIFI_GET_TIMESTAMP()   {
     return 0;
   } // FIXME

#else
#define ADI_WIFI_LOGEVENT(event)
#define ADI_WIFI_LOGEVENT_DATA(event,data)  
#endif  /* ADI_CFG_WIFI_LOGEVENT */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_WIFI_LOGEVENT_H */
