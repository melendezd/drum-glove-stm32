#pragma once

#include "mcu.hpp"
#include "status_indicator.hpp"
#include "drum_machine.hpp"
#include <span>

namespace dac
{

enum class Channel
{
    One, Two
};

struct Settings
{
    // DAC channel; must be 1 or 2
    Channel channel;
    StatusIndicator &indicator;
    std::span<uint8_t> buffer;
    TriggerTimer &timer;
    DelayTimer &delay;
    GPIO &amp_active;
    DrumMachine &drum_machine;
};

} // namespace dac


class AudioController
{
  public:
    AudioController( dac::Settings settings );

    // Whether the DAC channel is ready to accept the trigger or output data.
    bool is_ready();

    void start();
    void stop();

    void isr_dma_underrun();
    void isr_dma();
  private:

    // The DAC_CR register contains two copies of the same configuration option bits;
    // the least significant 16 for channel 1, and the most significant 16 for channel 2.
    // This function takes config bits in the least siginficant 16 bits and shifts them depending
    // on which channel you select.
    constexpr uint32_t apply_channel(uint32_t config);
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

    dac::Channel channel;

    StatusIndicator &indicator;
    TriggerTimer &timer;
    DelayTimer &delay;
    GPIO &amp_active;
    DrumMachine &drum_machine;

    volatile int stale_buffer_index;
    std::span<uint8_t> buffer;
  public:
    std::span<uint8_t> buffers[2];
};
