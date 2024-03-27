#pragma once

#include <cstdint>

void spin(volatile uint32_t cycle_count);
volatile uint16_t & cast16( volatile uint32_t &lvalue );
