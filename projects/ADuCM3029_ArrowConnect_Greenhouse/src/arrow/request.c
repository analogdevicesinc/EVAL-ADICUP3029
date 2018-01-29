/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/request.h"
#include <time/time.h>
#include <arrow/mem.h>
#if !defined(__XCC__)
#include <string.h>
#endif
#include <debug.h>

static void get_canonical_string(char *buffer, http_request_t *req){
    http_query_t *query = req->query;
    buffer[0] = '\0';
    while (query) {
        strcat(buffer, P_VALUE(query->key));
        strcat(buffer, "=");
        strcat(buffer, P_VALUE(query->value));
        strcat(buffer, "\r\n");
        query = query->next;
    }
}

void sign_request(http_request_t *req) {
    static char ts[25];
    static char signature[70];
    char *canonicalQuery = NULL;
    if ( req->query ) {
      canonicalQuery = (char*)malloc(CANONICAL_QUERY_LEN);
      get_canonical_string(canonicalQuery, req);
    }
    get_time(ts);
    http_request_add_header(req,
                            p_const("x-arrow-apikey"),
                            p_const(get_api_key()));
    http_request_add_header(req,
                            p_const("x-arrow-date"),
                            p_const(ts));
    http_request_add_header(req,
                            p_const("x-arrow-version"),
                            p_const("1"));

    sign(signature, ts, P_VALUE(req->meth),
         P_VALUE(req->uri), canonicalQuery,
         P_VALUE(req->payload.buf), "1");

    if (canonicalQuery) free(canonicalQuery);
    http_request_add_header(req,
                            p_const("x-arrow-signature"),
                            p_const(signature));
    http_request_set_content_type(req, p_const("application/json"));
    http_request_add_header(req,
                            p_const("Accept"),
                            p_const("application/json"));
    http_request_add_header(req,
                            p_const("Connection"),
                            p_const("Keep-Alive"));
    http_request_add_header(req,
                            p_const("Accept-Encoding"),
                            p_const("gzip, deflate"));
    http_request_add_header(req,
                            p_const("User-Agent"),
                            p_const("Eos"));
}
