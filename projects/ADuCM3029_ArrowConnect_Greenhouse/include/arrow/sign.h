/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_SIGN_H_
#define ARROW_SIGN_H_

// API key
char *get_api_key(void);

// secret key
char *get_secret_key(void);

// set a new API key
void set_api_key(char *newkey);

// set a new secret key
void set_secret_key(char *newkey);

// create a signature by the Arrow algorithm
void sign(char *signature,
          const char* timestamp,
          const char *meth,
          const char *uri,
          const char *canQueryString,
          const char *payload,
          const char *apiVersion);

#endif /* ARROW_SIGN_H_ */
