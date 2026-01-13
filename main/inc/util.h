/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef H_UTIL_
#define H_UTIL_

#include <stdint.h>
#include "nimble/ble.h"

struct os_mbuf;

void print_bytes(const uint8_t *bytes, int len);
void print_addr(const void *addr);
char *addr_str(const void *addr);
void print_mbuf(const struct os_mbuf *om);

#endif
