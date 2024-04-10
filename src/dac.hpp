#pragma once

#include "mcu.hpp"
#include "status_indicator.hpp"
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
    GPIO &amp_active;
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
    void configure_dac();
    void configure_timer();

    volatile uint32_t *data_register;

    DAC_TypeDef *dac;
    DMA_Channel_TypeDef *dma;
    DMAMUX_Channel_TypeDef *dmamux;

    dac::Channel channel;

    StatusIndicator &indicator;
    TriggerTimer &timer;
    GPIO &amp_active;

    std::span<uint8_t> buffer;
};
