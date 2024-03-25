#pragma once

#include "stm32g431xx.h"
namespace dac
{

struct Settings
{
    // 1 or 2
    int channel;
};

} // namespace dac


class DacController
{
  public:
    DacController( dac::Settings settings );
  private:
    constexpr uint32_t apply_channel(uint32_t mask);

    dac::Settings settings;
    DAC_TypeDef *dac;
    GPIO &error_led;
};
