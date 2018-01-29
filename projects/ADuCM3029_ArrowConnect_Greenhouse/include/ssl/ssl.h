/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_SSL_H_
#define ARROW_SSL_H_

int ssl_connect(int sock);
int ssl_recv(int sock, char *data, int len);
int ssl_send(int sock, char* data, int length);
int ssl_close(int sock);

#endif
