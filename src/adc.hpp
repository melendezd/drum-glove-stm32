#pragma once

#include <span>

#include "mcu.hpp"
#include "timer.hpp"

class ADCController
{
  public:
    ADCController(DelayTimer &delay, std::span<uint8_t> out_buffer);
    std::span<uint8_t> out_buffer;
  private:
    ADC_TypeDef *adc;

    DelayTimer &delay;
};
