#pragma once

#include "mcu.hpp"
#include "timer.hpp"

class ADCController
{
  public:
    ADCController(DelayTimer &delay);
  private:
    ADC_TypeDef *adc;

    DelayTimer &delay;
};
