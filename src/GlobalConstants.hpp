#pragma once
#include "Gpio.hpp"
#include "Timer.hpp"
#include "stm32g431xx.h"

#include <cstdint>

// ---- status codes ----
namespace status
{
// Invalid channel selected when setting up DAC
const uint32_t dac_invalid_channel = 1;
// V_REFBUF not ready when setting up DAC
const uint32_t dac_not_ready    = 2;
const uint32_t ok               = 3;
const uint32_t dac_dma_underrun = 4;
} // namespace status

namespace constants
{
const uint32_t sample_rate       = 32000;      // 32 kHz
const uint32_t clock_frequency   = 16'000'000; // 16 MHz
const int      sample_count      = 3;
const int      adc_window_length = 128;
} // namespace constants

namespace hardware_constants
{
const auto pin_led_port = gpio::Port::B;
const auto pin_led_pin  = 8;

const auto pin_amp_active_port = gpio::Port::A;
const auto pin_amp_active_pin  = 7;

const auto pin_trigger_1_port = gpio::Port::A;
const auto pin_trigger_1_pin  = 11;

const auto timer_delay_id = timer::Id::Tim6;
const auto timer_trigger_id = timer::Id::Tim7;

const auto dac_audio_controller = DAC;
const auto dma_channel_audio_controller = DMA1_Channel1;
const auto dma_irq_audio_controller = DMA1_Channel1_IRQn;
const auto dma_audio_controller = DMA1;
const auto dmamux_audio_controller = DMAMUX1_Channel0;
} // namespace hardware
