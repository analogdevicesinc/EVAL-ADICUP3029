/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//Debug is disabled by default
#include "ntp/client.h"
#include <debug.h>
#include <bsd/socket.h>
#include <time/time.h>
#include <bsd/inet.h>
#include <arrow/mem.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull //Diff btw a UNIX timestamp (Starting Jan, 1st 1970) and a NTP timestamp (Starting Jan, 1st 1900)

int ntp_set_time(
        const char* host,
        uint16_t port,
        uint32_t wait_option) {
#ifdef DEBUG
  time_t ctTime;
  ctTime = time(NULL);
  DBG("Time is set to (UTC): %s", ctime(&ctTime));
#endif
  struct NTPPacket pkt;

  int udp_sock;
  socklen_t serverlen;
  struct sockaddr_in serveraddr;
  struct hostent *server;

  udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_sock < 0) {
      DBG("ERROR opening socket %d", udp_sock);
      return -1;
  }
  DBG("udp socket open %d", (int)udp_sock);

  if ( wait_option != 0 ) {
      struct timeval timeout;
      timeout.tv_sec = (time_t) (wait_option / 1000);
      timeout.tv_usec = (suseconds_t)(wait_option % 1000) * 1000;

      if ( setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 ) {
          DBG("UDP set timeout fail");
      }
  }

  /* gethostbyname: get the server's DNS entry */
  server = gethostbyname(host);
  if (server == NULL) {
      DBG("ERROR, no such host as %s\n", host);
      soc_close(udp_sock);
      return NTP_DNS;
  }

  /* build the server's Internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, (size_t)server->h_length);
  serveraddr.sin_port = htons(port);
  serverlen = sizeof(serveraddr);

  //Now ping the server and wait for response
  //Prepare NTP Packet:
  pkt.li = 0; //Leap Indicator : No warning
  pkt.vn = 4; //Version Number : 4
  pkt.mode = 3; //Client mode
  pkt.stratum = 0; //Not relevant here
  pkt.poll = 0; //Not significant as well
  pkt.precision = 0; //Neither this one is

  pkt.rootDelay = 0; //Or this one
  pkt.rootDispersion = 0; //Or that one
  pkt.refId = 0; //...

  pkt.refTm_s = 0;
  pkt.origTm_s = 0;
  pkt.rxTm_s = 0;
  time_t now1 = time(NULL);
  DBG("get time %d", (int)now1);
  DBG("ctime %s", ctime(&now1));

  pkt.txTm_s = _htonl( (uint32_t)NTP_TIMESTAMP_DELTA + (uint32_t)time(NULL) ); //WARN: We are in LE format, network byte order is BE
  pkt.refTm_f = pkt.origTm_f = pkt.rxTm_f = pkt.txTm_f = 0;

  //Set timeout, non-blocking and wait using select
  int ret;
  ret = sendto(udp_sock, (char*)&pkt, sizeof(struct NTPPacket), 0, (struct sockaddr*)&serveraddr, serverlen);
  if (ret < 0 ) {
    DBG("Could not send packet");
    soc_close(udp_sock);
    return NTP_CONN;
  }

  //Read response
  ret = recvfrom(udp_sock, (char*)&pkt, sizeof(struct NTPPacket), 0, (struct sockaddr*)&serveraddr, &serverlen);
  soc_close(udp_sock);

  if(ret < 0) {
      DBG("Could not receive packet");
      return NTP_CONN;
  }

  //TODO: Accept chunks
  if ( ret < (int)sizeof(struct NTPPacket) ) {
    DBG("Receive packet size does not match %d", ret);
    return NTP_PRTCL;
  }

  //Kiss of death message : Not good !
  if( pkt.stratum == 0) {
      DBG("Kissed to death!");
      return NTP_PRTCL;
  }

  //Correct Endianness
  pkt.refTm_s = _ntohl( pkt.refTm_s );
  pkt.refTm_f = _ntohl( pkt.refTm_f );
  pkt.origTm_s = _ntohl( pkt.origTm_s );
  pkt.origTm_f = _ntohl( pkt.origTm_f );
  pkt.rxTm_s = _ntohl( pkt.rxTm_s );
  pkt.rxTm_f = _ntohl( pkt.rxTm_f );
  pkt.txTm_s = _ntohl( pkt.txTm_s );
  pkt.txTm_f = _ntohl( pkt.txTm_f );

  //Compute offset, see RFC 4330 p.13
  uint32_t destTm_s = (uint32_t)NTP_TIMESTAMP_DELTA + (uint32_t)time(NULL);
  int64_t offset = ( (int64_t)( pkt.rxTm_s - pkt.origTm_s ) + (int64_t) ( pkt.txTm_s - destTm_s ) ) >> 1; //Avoid overflow
  set_time((time_t)offset + time(NULL));

  return NTP_OK;
}
