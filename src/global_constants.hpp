#pragma once

#include <cstdint>

// ---- status codes ----
namespace status
{

// Invalid channel selected when setting up DAC
const uint32_t dac_invalid_channel = 1;

// V_REFBUF not ready when setting up DAC
const uint32_t dac_not_ready = 2;

const uint32_t ok = 3;

}
