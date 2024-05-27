#pragma once

#include <span>

#include "mcu.hpp"
#include "timer.hpp"
#include "status_indicator.hpp"

class ADCController
{
  public:
    ADCController(DelayTimer &delay, std::span<volatile uint8_t> out_buffer, StatusIndicator &indicator);
    std::span<volatile uint8_t> out_buffer;
    void start();
    void isr_dma();
    bool is_tap_detected();
    void clear_tap_detected();
  private:
    void configure_adc();
    void configure_dma();
    void enable_dma();

    bool is_status_full_transfer();

    volatile bool tap_detected;

    ADC_TypeDef *adc;
    DMA_Channel_TypeDef *dma;
    DMA_TypeDef *dma_isr;
    DMAMUX_Channel_TypeDef *dmamux;

    DelayTimer &delay;
    StatusIndicator &indicator;

    volatile uint16_t *data_register;
};
