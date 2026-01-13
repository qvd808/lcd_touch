/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef H_SCLI_
#define H_SCLI_

#include "esp_err.h"

int scli_init(void);
int scli_receive_key(int *console_key);

#endif
