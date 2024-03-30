#pragma once

#include "mcu.hpp"
#include "io.hpp"
#include "status_indicator.hpp"

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
};

} // namespace dac


class DacController
{
  public:
    DacController( dac::Settings settings );

    bool write_if_ready(uint32_t data);
    void write(uint32_t data);

    // Whether the DAC channel is ready to accept the trigger or output data.
    bool is_ready();
  private:

    // The DAC_CR register contains two copies of the same configuration option bits;
    // the least significant 16 for channel 1, and the most significant 16 for channel 2.
    // This function takes config bits in the least siginficant 16 bits and shifts them depending
    // on which channel you select.
    constexpr uint32_t apply_channel(uint32_t config);
    volatile uint8_t *get_data_register();

    void enable();
    void enable_clock();

    volatile uint32_t *data_register;

    DAC_TypeDef *dac;
    dac::Channel channel;
    StatusIndicator &indicator;
};
