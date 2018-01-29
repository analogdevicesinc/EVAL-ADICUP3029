/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "ssl/ssl.h"
#include <wolfssl/ssl.h>
#include <arrow/mem.h>
#include <debug.h>
#include <unint.h>
#include <bsd/socket.h>
#ifdef DEBUG_WOLFSSL
#include <wolfcrypt/logging.h>
#endif

typedef struct socket_ssl {
WOLFSSL_METHOD  *method;
WOLFSSL_CTX     *ctx;
WOLFSSL         *ssl;
int socket;
struct socket_ssl *next;
} socket_ssl_t;

static socket_ssl_t *__sock = NULL;

#ifdef DEBUG_WOLFSSL
static void cli_wolfSSL_Logging_cb(const int logLevel,
                                  const char *const logMessage) {
    DBG("[http]:%d (%s)", logLevel, logMessage);
}
#endif

int create_ssl_sock(socket_ssl_t *s) {
	socket_ssl_t *last = __sock;
	if ( last ) {
		while(last->next) last = last->next;
		last->next = s;
	} else {
		__sock = s;
	}
	return 0;
}

int remove_ssl_sock(int sock) {
	socket_ssl_t *last = __sock;
	socket_ssl_t *old = NULL;
	while(last) {
		if ( last->socket == sock ) {
			if ( old ) {
				old->next = last->next;
				free(last);
				return 0;
			} else {
				__sock = last->next;
				free(last);
				return 0;
			}
		}
		old = last;
		last = last->next;
	}
	return -1;
}

socket_ssl_t *find_ssl_sock(int sock) {
	socket_ssl_t *last = __sock;
	while(last) {
		if ( last->socket == sock ) return last;
		last = last->next;
	}
	return NULL;
}

static int recv_ssl(WOLFSSL *wsl, char* buf, int sz, void* vp) {
    SSP_PARAMETER_NOT_USED(wsl);
    socket_ssl_t *s = (socket_ssl_t *)vp;
    if ( sz < 0 ) return sz;
//    struct timeval interval = { 5000/1000, (0 % 1000) * 1000 };
//    setsockopt(s->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
    int got = recv(s->socket, buf, (uint32_t)sz, 0);
//    DBG("recv ssl %d [%d]", got, sz);
    if (got <= 0)  return -2;  // IO_ERR_WANT_READ;
    return (int)got;
}

static int send_ssl(WOLFSSL *wsl, char* buf, int sz, void* vp) {
    SSP_PARAMETER_NOT_USED(wsl);
    socket_ssl_t *s = (socket_ssl_t *)vp;
    if ( sz < 0 ) return sz;
    int sent = send(s->socket, buf, (uint32_t)sz, 0);
//    DBG("send ssl %d [%d]", sent, sz);
    if (sent <= 0) return -2;  // IO_ERR_WANT_WRITE
    return (int)sent;
}

int __attribute__((weak)) ssl_connect(int sock) {
	if ( !__sock ) wolfSSL_Init();
	socket_ssl_t *s = (socket_ssl_t *)malloc(sizeof(socket_ssl_t));
	s->next = NULL;
	s->socket = sock;
	DBG("init ssl connect %d", sock)
	s->method = wolfTLSv1_2_client_method();
	s->ctx = wolfSSL_CTX_new(s->method);
	if ( s->ctx == NULL) {
		DBG("unable to get ctx");
	}
	wolfSSL_CTX_set_verify(s->ctx, SSL_VERIFY_NONE, NULL);
	wolfSSL_SetIORecv(s->ctx, recv_ssl);
	wolfSSL_SetIOSend(s->ctx, send_ssl);

	s->ssl = wolfSSL_new(s->ctx);
	if (s->ssl == NULL) {
		DBG("oops, bad SSL ptr");
	}
	wolfSSL_SetIOReadCtx(s->ssl, (void*)s);
	wolfSSL_SetIOWriteCtx(s->ssl, (void*)s);
#ifdef DEBUG_WOLFSSL
    wolfSSL_SetLoggingCb(cli_wolfSSL_Logging_cb);
    wolfSSL_Debugging_ON();
#endif
	int err = wolfSSL_connect(s->ssl);
	if (err != SSL_SUCCESS) {
		free(s);
		DBG("SSL connect fail");
		return -1;
	} else {
		create_ssl_sock(s);
		DBG("SSL connect done");
	}
  return 0;
}

int __attribute__((weak)) ssl_recv(int sock, char *data, int len) {
//	DBG("ssl r[%d]", len);
	socket_ssl_t *s = find_ssl_sock(sock);
	if ( !s ) return -1;
	return wolfSSL_read(s->ssl, data, (int)len);
}

int __attribute__((weak)) ssl_send(int sock, char* data, int length) {
//	DBG("ssl w[%d]", length);
	socket_ssl_t *s = find_ssl_sock(sock);
	if ( !s ) return -1;
	return wolfSSL_write(s->ssl, data, (int)length);
}

int __attribute__((weak)) ssl_close(int sock) {
	socket_ssl_t *s = find_ssl_sock(sock);
	DBG("close ssl");
  if ( s ) {
    wolfSSL_free(s->ssl);
    wolfSSL_CTX_free(s->ctx);
    remove_ssl_sock(sock);
    if ( !__sock ) wolfSSL_Cleanup();
  }
	return 0;
}
