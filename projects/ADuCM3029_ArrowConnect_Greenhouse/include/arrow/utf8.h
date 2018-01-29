/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_UTF8_H_
#define ARROW_UTF8_H_

int utf8check(const char *s);
void fix_urldecode(char *query);
void urlencode(char *dst, char *src, int len);
void hex_encode(char *dst, const char *src, int size);

#endif
