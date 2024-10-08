#pragma once

#include "McuIncludes.hpp"
#include "StatusIndicator.hpp"
#include "DrumMachine.hpp"
#include <span>

namespace audio
{

struct Settings
{
    StatusIndicator &indicator;
    std::span<uint8_t> audio_out_buffer;
    TriggerTimer &timer;
    DelayTimer &delay;
    GpioPin &amp_active;
    DrumMachine &drum_machine;
    std::span<volatile uint8_t> adc_buffer;
};

} // namespace dac


class AudioController
{
  public:
    AudioController( audio::Settings settings );

    // Whether the DAC channel is ready to accept the trigger or output data.
    bool is_ready();

    void start();
    void stop();

    void isr_dma_underrun();
    void isr_dma();
  private:

    volatile uint8_t *get_data_register();
    uint32_t get_tsel_value();

    void enable_dac();

    void configure_dma();
    void enable_dma();
    void configure_dac();

    bool is_status_dma_underrun();
    bool is_status_half_transfer();
    bool is_status_full_transfer();

    volatile uint32_t *data_register;

    DAC_TypeDef *dac;
    DMA_Channel_TypeDef *dma;
    DMA_TypeDef *dma_isr;
    DMAMUX_Channel_TypeDef *dmamux;

    StatusIndicator &indicator;
    TriggerTimer &timer;
    DelayTimer &delay;
    GpioPin &amp_active;
    DrumMachine &drum_machine;

    std::span<volatile uint8_t> adc_buffer;

    volatile int stale_buffer_index;
    std::span<uint8_t> audio_out_buffer;
    std::span<uint8_t> audio_out_buffers[2];
};
