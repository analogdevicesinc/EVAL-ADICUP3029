/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_ACCOUNT_H_
#define ARROW_ACCOUNT_H_

// create or login this account
int arrow_create_account(const char *name, const char *email, const char *pass);

#define arrow_login_account arrow_create_account

#endif
