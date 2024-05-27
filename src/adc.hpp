#pragma once

#include <span>

#include "mcu.hpp"
#include "timer.hpp"

class ADCController
{
  public:
    ADCController(DelayTimer &delay, std::span<volatile uint8_t> out_buffer);
    std::span<volatile uint8_t> out_buffer;
    void start();
  private:
    void configure_adc();
    void configure_dma();
    void enable_dma();

    ADC_TypeDef *adc;
    DMA_Channel_TypeDef *dma;
    DMA_TypeDef *dma_isr;
    DMAMUX_Channel_TypeDef *dmamux;

    DelayTimer &delay;

    volatile uint16_t *data_register;
};
