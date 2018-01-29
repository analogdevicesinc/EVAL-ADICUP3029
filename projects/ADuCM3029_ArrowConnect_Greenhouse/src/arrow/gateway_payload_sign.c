#include "arrow/gateway_payload_sign.h"
#include <arrow/sign.h>
#include <ssl/crypt.h>
#include <arrow/mem.h>
#include <debug.h>
#include <arrow/utf8.h>

int gateway_payload_sign(char *signature,
                         const char *hid,
                         const char *name,
                         int encrypted,
                         const char *canParString,
                         const char *signatureVersion) {
  // step 1
  CREATE_CHUNK(canonicalRequest, 512);
  strcpy(canonicalRequest, hid);
  strcat(canonicalRequest, "\n");
  strcat(canonicalRequest, name);
  strcat(canonicalRequest, "\n");
  if ( encrypted ) strcat(canonicalRequest, "true\n");
  else strcat(canonicalRequest, "false\n");
  strcat(canonicalRequest, canParString);
  strcat(canonicalRequest, "\n");
  char hex_hash_canonical_req[66];
  char hash_canonical_req[34];
  sha256(hash_canonical_req, canonicalRequest, (int)strlen(canonicalRequest));
  hex_encode(hex_hash_canonical_req, hash_canonical_req, 32);
  hex_hash_canonical_req[64] = '\0';
//  DBG("can: %s", hex_hash_canonical_req);

  // step 2
  char *stringtoSign = canonicalRequest;
  strcpy(stringtoSign, hex_hash_canonical_req);
  strcat(stringtoSign, "\n");
  strcat(stringtoSign, get_api_key());
  strcat(stringtoSign, "\n");
  strcat(stringtoSign, signatureVersion);

//  DBG("strToSign:\r\n[%s]", stringtoSign);
  // step 3
  CREATE_CHUNK(tmp, 128);
  CREATE_CHUNK(hex_tmp, 128);

//  DBG("api: %s", get_api_key());
//  DBG("sec: %s", get_secret_key());
  hmac256(tmp, get_api_key(), (int)strlen(get_api_key()), get_secret_key(), (int)strlen(get_secret_key()));
  hex_encode(hex_tmp, tmp, 32);
//  DBG("hex1: %s", hex_tmp);
  memset(tmp, 0, 128);
  hmac256(tmp, signatureVersion, (int)strlen(signatureVersion), hex_tmp, (int)strlen(hex_tmp));
  hex_encode(hex_tmp, tmp, 32);
//  DBG("hex2: [%d]%s", strlen(hex_tmp), hex_tmp);
  hmac256(tmp, hex_tmp, strlen(hex_tmp), stringtoSign, strlen(stringtoSign));
  hex_encode(signature, tmp, 32);
//  DBG("sig: [%d]%s", strlen(signature), signature);
  FREE_CHUNK(canonicalRequest);
  FREE_CHUNK(tmp);
  FREE_CHUNK(hex_tmp);
  return 0;
}
