/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "ssl/crypt.h"

#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/hmac.h"

#include <arrow/mem.h>

void __attribute__((weak)) sha256(char *shasum, char *buf, int size) {
  Sha256 sh;
  wc_InitSha256(&sh);
  wc_Sha256Update(&sh, (byte*)buf, (word32)size);
  wc_Sha256Final(&sh, (byte*)shasum);
}

void __attribute__((weak)) hmac256(char *hmacdig, const char *key, int key_size, const char *buf, int buf_size) {
  Hmac hmac;
  wc_HmacSetKey(&hmac, SHA256, (const byte*)key, (word32)key_size);
  wc_HmacUpdate(&hmac, (const byte*)buf, (word32)buf_size);
  wc_HmacFinal(&hmac, (byte*)hmacdig);
}

