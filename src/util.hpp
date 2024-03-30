#pragma once

#include <cstdint>

void spin(volatile uint32_t cycle_count);

// Casts an unsigned 32-bit integer reference to a unsigned 16-bit integer reference.
// Useful for when you want to do explicit halfword writes.
volatile uint16_t & cast16( volatile uint32_t &lvalue );

// Casts an unsigned 32-bit integer reference to a unsigned 8-bit integer reference.
// Useful for when you want to do explicit halfword writes.
volatile uint8_t & cast8( volatile uint32_t &lvalue );
