// Copyright 2025, Fermin Olaiz <ferminolaiz@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdint.h>

void timer_init();
uint32_t timer_read_time();
void timer_kick();
