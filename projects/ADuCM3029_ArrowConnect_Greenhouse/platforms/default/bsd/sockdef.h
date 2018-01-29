/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

    #ifndef _COMMON_BSD_SOCKDEF_H_
#define _COMMON_BSD_SOCKDEF_H_
 
#include <unint.h>
    
#define  SOL_SOCKET  0xfff    /* options for socket level */

#define AF_UNSPEC       0
#define AF_INET         2
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_UDPLITE 136


/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

    
/*
 * Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c)
 */
#define  SO_DEBUG       0x0001 /* Unimplemented: turn on debugging info recording */
#define  SO_ACCEPTCONN  0x0002 /* socket has had listen() */
#define  SO_REUSEADDR   0x0004 /* Allow local address reuse */
#define  SO_KEEPALIVE   0x0008 /* keep connections alive */
#define  SO_DONTROUTE   0x0010 /* Unimplemented: just use interface addresses */
#define  SO_BROADCAST   0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */
#define  SO_USELOOPBACK 0x0040 /* Unimplemented: bypass hardware when possible */
#define  SO_LINGER      0x0080 /* linger on close if data present */
#define  SO_OOBINLINE   0x0100 /* Unimplemented: leave received OOB data in line */
#define  SO_REUSEPORT   0x0200 /* Unimplemented: allow local address & port reuse */

#define SO_DONTLINGER   ((int)(~SO_LINGER))

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF    0x1001    /* Unimplemented: send buffer size */
#define SO_RCVBUF    0x1002    /* receive buffer size */
#define SO_SNDLOWAT  0x1003    /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT  0x1004    /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO  0x1005    /* Unimplemented: send timeout */
#define SO_RCVTIMEO  0x1006    /* receive timeout */
#define SO_ERROR     0x1007    /* get error status and clear */
#define SO_TYPE      0x1008    /* get socket type */
#define SO_CONTIMEO  0x1009    /* Unimplemented: connect timeout */
#define SO_NO_CHECK  0x100a    /* don't create UDP checksum */
    
    
struct in_addr {
  uint32_t s_addr;
};

/* members are in network byte order */
struct sockaddr_in {
  uint8_t sin_len;
  uint8_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr {
  uint8_t sa_len;
  uint8_t sa_family;
  char sa_data[14];
};

# ifndef socklen_t
#  define socklen_t uint32_t
# endif
    
#endif
