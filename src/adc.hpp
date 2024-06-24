#pragma once

#include "mcu.hpp"
#include "timer.hpp"
#include "status_indicator.hpp"
#include "global_constants.hpp"
#include <array>

class ADCController
{
  public:
    ADCController(DelayTimer &delay, StatusIndicator &indicator);
    std::array<std::array<volatile uint8_t, constants::adc_window_length>, constants::sample_count> out_buffer;

    void start();
    void isr_dma();
    volatile bool tap_detected[constants::sample_count];
  private:
    void configure_adc();
    void configure_dma();
    void enable_dma();

    bool is_status_full_transfer();

    ADC_TypeDef *adc;
    DMA_Channel_TypeDef *dma;
    DMA_TypeDef *dma_isr;
    DMAMUX_Channel_TypeDef *dmamux;

    DelayTimer &delay;
    StatusIndicator &indicator;

    volatile uint16_t *data_register;

    std::array<volatile uint8_t, 2 * constants::sample_count * constants::adc_window_length> out_buffer_combined;
};
