/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_REQUEST_H_
#define ARROW_REQUEST_H_

#include "http/request.h"
#include "sign.h"

#if !defined(CANONICAL_QUERY_LEN)
#define CANONICAL_QUERY_LEN 256
#endif

// Add needed headers for the Arrow cloud
void sign_request(http_request_t *req);

#endif /* ARROW_REQUEST_H_ */
