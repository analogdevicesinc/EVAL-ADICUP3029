/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef HTTPCLIENT_REQUEST_H_
#define HTTPCLIENT_REQUEST_H_

#include <config.h>
#include <unint.h>
#include <arrow/mem.h>

#if defined(HTTP_DEBUG)
#define HTTP_DBG DBG
#else
#define HTTP_DBG(...)
#endif

#if defined(_ARIS_)
# include "wifi_thread.h"
# include "driver/include/m2m_wifi.h"
# include "driver/source/nmasic.h"
# include "bus_wrapper/include/nm_bus_wrapper.h"
# include "reloc_macro.h"
# include "reloc_assert.h"
#elif defined(__MBED__)
# include <inttypes.h>
# include <stdio.h>
#elif defined(__linux__)
# include <stdint.h>
# include <string.h>
#elif defined(__XCC__)
# include <qcom/base.h>
# include <qcom_common.h>
#endif

#define HEAD_FIELD_LEN 100
#define CONTENT_TYPE "Content-Type"

enum METH {
  GET,
  POST,
  PUT,
  DELETE,
  HEAD,
  METH_count
};

enum Scheme {
  http,
  https,
  Scheme_count
};


typedef struct __attribute_packed__ http_header_ {
    property_t key;
    property_t value;
    struct http_header_ *next;
} http_header_t;

typedef struct http_header_ http_query_t;

typedef struct __attribute_packed__ {
    uint32_t size;
    property_t buf;
} http_payload_t;

// the payload handler for response
// for OTA firmware update
// if it's impossible to allocate memory for binary file
typedef int(*__payload_handler)(void *,property_t,int);
typedef struct __payload_meth {
  __payload_handler _p_set_handler;
  __payload_handler _p_add_handler;
} _payload_meth_t;

typedef struct __attribute_packed__ {
    property_t meth;
    property_t scheme;
    property_t host;
    property_t uri;
    uint16_t port;
    int8_t is_corrupt;
    int8_t is_cipher;
    int8_t is_chunked;
    http_header_t *header;
    http_header_t content_type;
    http_query_t *query;
    http_payload_t payload;
    _payload_meth_t _response_payload_meth;
} http_request_t;

typedef struct {
    int m_httpResponseCode;
    int is_chunked;
    http_header_t *header;
    http_header_t content_type;
    http_payload_t payload;
    _payload_meth_t _p_meth;
} http_response_t;

void http_request_init(http_request_t *req, int meth, const char *url);
void http_request_close(http_request_t *req);
void http_request_add_header(http_request_t *req, property_t key, property_t value);
void http_request_add_query(http_request_t *req, property_t key, property_t value);
void http_request_set_content_type(http_request_t *req, property_t value);
http_header_t *http_request_first_header(http_request_t *req);
http_header_t *http_request_next_header(http_request_t *req, http_header_t *head);
void http_request_set_payload(http_request_t *req, property_t payload);

void http_response_init(http_response_t *req, _payload_meth_t *handler);
void http_response_free(http_response_t *res);
void http_response_add_header(http_response_t *req, property_t key, property_t value);
void http_response_set_content_type(http_response_t *req, property_t value);
void http_response_set_payload(http_response_t *req, property_t payload, uint32_t size);
void http_response_add_payload(http_response_t *req, property_t payload, uint32_t size);

#endif /* HTTPCLIENT_REQUEST_H_ */
