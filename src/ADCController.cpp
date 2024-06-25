#include "ADCController.hpp"
#include "stm32g431xx.h"

ADCController::ADCController(DelayTimer &delay, StatusIndicator &indicator) 
    : tap_detected(false)
    , adc( ADC1 )
    , dma( DMA1_Channel2 )
    , dma_isr( DMA1 )
    , dmamux( DMAMUX1_Channel1 )
    , delay(delay)
    , indicator(indicator)
    , data_register( reinterpret_cast<volatile uint16_t *>(&adc->DR) )
{
    configure_adc();
    configure_dma();
    enable_dma();
}

void ADCController::configure_adc()
{
    // enable ADC clock
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC12EN);
    MODIFY_REG(RCC->CCIPR, 0x3U << RCC_CCIPR_ADC12SEL_Pos, 0x2U << RCC_CCIPR_ADC12SEL_Pos);

    // exit deep power-down mode and enable voltage regulator
    MODIFY_REG(adc->CR, ADC_CR_DEEPPWD | ADC_CR_ADVREGEN, ADC_CR_ADVREGEN);

    // wait for startup time (11us)
    delay.us(11);

    // run single-ended input calibration
    //ensure disabled
    CLEAR_BIT(adc->CR, ADC_CR_ADEN);
    //single-ended input (i.e. relative to ground)
    CLEAR_BIT(adc->CR, ADC_CR_ADCALDIF);
    //start calibration
    SET_BIT(adc->CR, ADC_CR_ADCAL);
    //wait until calibration complete
    while(READ_BIT(adc->CR, ADC_CR_ADCAL) != 0);

    // enable ADC
    SET_BIT(adc->CR, ADC_CR_ADEN);

    // configure conversions sequence channels
    //uint32_t sqr1_num_conversions = (6U - 1) << ADC_SQR1_L_Pos; // 6 conversions in a sequence
    uint32_t sqr1_num_conversions = (6U - 1) << ADC_SQR1_L_Pos; // 6 conversions in a sequence
    uint32_t sqr1_conversion_1 = 1U << ADC_SQR1_SQ1_Pos; // PA0
    uint32_t sqr1_conversion_2 = 1U << ADC_SQR1_SQ2_Pos; // PA0
    uint32_t sqr1_conversion_3 = 2U << ADC_SQR1_SQ3_Pos; // PA1
    uint32_t sqr1_conversion_4 = 2U << ADC_SQR1_SQ4_Pos; // PA1
    uint32_t sqr2_conversion_5 = 15U << ADC_SQR2_SQ5_Pos; // PB0
    uint32_t sqr2_conversion_6 = 15U << ADC_SQR2_SQ6_Pos; // PB0
    uint32_t sqr1_conversions = sqr1_conversion_1 | sqr1_conversion_2 | sqr1_conversion_3 | sqr1_conversion_4;
    uint32_t sqr2_conversions = sqr2_conversion_5 | sqr2_conversion_6;

    MODIFY_REG(adc->SQR1, 0xffffffff, sqr1_num_conversions | sqr1_conversions);
    MODIFY_REG(adc->SQR2, 0xffffffff, sqr2_conversions);

    // configure sampling times for channels 3, 12, 13
    // 0b101: 92.5 ADC clock cycles (5.78us > 4us minimum per datasheet)
    MODIFY_REG(
        adc->SMPR1,
        ADC_SMPR1_SMP1 | ADC_SMPR1_SMP2,
        (0b101U << ADC_SMPR1_SMP1_Pos) | (0b101U << ADC_SMPR1_SMP2_Pos)
    );
    MODIFY_REG(
        adc->SMPR2,
        ADC_SMPR2_SMP15,
        0b101U << ADC_SMPR2_SMP15_Pos
    );

    uint32_t adc_cfgr_reset = 0x8000'0000U; // reset value according to manual
    MODIFY_REG(
        adc->CFGR,
        adc_cfgr_reset,
        ADC_CFGR_DMAEN
            | ADC_CFGR_DMACFG // DMA circular mode
            | ADC_CFGR_RES_1 // 8-bit resolution
            | ADC_CFGR_CONT // continuous conversion mode -- no trigger timer needed, just start by setting ADSTART=1
            | ADC_CFGR_JQDIS // injected queue disabled
    );
}

void ADCController::configure_dma()
{
    // enable DMA and DMAMUX clocks
    SET_BIT( RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN );

    // configure DMA control register
    // MSIZE is left at its reset value, so memory size is 8 bits
    MODIFY_REG(
        dma->CCR, ( 1U << 15 ) - 1,
        DMA_CCR_CIRC          // circular mode
            | DMA_CCR_MINC    // memory increment mode
            | DMA_CCR_PSIZE_0 // (0b00) 16 bit peripheral size
            | DMA_CCR_PL_1    // (0b10) high priority
            | DMA_CCR_TCIE    // transfer complete interrupt
    );

    // enable interrupt
    NVIC_ClearPendingIRQ( DMA1_Channel2_IRQn );
    NVIC_SetPriority( DMA1_Channel2_IRQn, 1u );
    NVIC_EnableIRQ( DMA1_Channel2_IRQn );

    // DMA CNDTR - number of data to transfer
    MODIFY_REG( dma->CNDTR, 0xffffU, out_buffer_combined.size() );

    // DMA CPAR - peripheral address
    MODIFY_REG( dma->CPAR, 0xffffffffU, reinterpret_cast< uint32_t >( data_register ) );

    // DMA CMAR - memory address
    MODIFY_REG( dma->CMAR, 0xffffffffU, reinterpret_cast< uint32_t >( out_buffer_combined.data() ) );

    // configure DMAMUX
    const uint32_t dmamux_ccr_mask = DMAMUX_CxCR_DMAREQ_ID | DMAMUX_CxCR_SOIE | DMAMUX_CxCR_EGE | DMAMUX_CxCR_SE |
                                     DMAMUX_CxCR_SE | DMAMUX_CxCR_SPOL | DMAMUX_CxCR_NBREQ | DMAMUX_CxCR_SYNC_ID;
    MODIFY_REG(
        dmamux->CCR, dmamux_ccr_mask,
        5U << DMAMUX_CxCR_DMAREQ_ID_Pos // ADC1
    );
}

bool ADCController::is_status_full_transfer()
{
    return READ_BIT( dma_isr->ISR, DMA_ISR_TCIF2 );
}

void ADCController::start()
{
    SET_BIT(adc->CR, ADC_CR_ADSTART);
}

void ADCController::enable_dma()
{
    SET_BIT( dma->CCR, DMA_CCR_EN );
}

void ADCController::isr_dma()
{
    if (!is_status_full_transfer()) return;

    // transfer contents of raw out buffer to individual out buffers
    for (int sample_id = 0; sample_id < constants::sample_count; sample_id++) {
        for (int i = 0; i < constants::adc_window_length; i++) {
            int combined_index = (2 * constants::sample_count * i) + (2 * sample_id + 1);
            out_buffer[sample_id][i] = out_buffer_combined[combined_index];
        }
    }

    // process buffers to detect taps
    const uint8_t threshold = 128;
    for (int sample_id = 0; sample_id < constants::sample_count; sample_id++) {
        for (uint8_t sample : out_buffer[sample_id]) {
            if (sample >= threshold) {
                tap_detected[sample_id] = true;
                break;
            }
        }
    }

    SET_BIT(dma_isr->IFCR, DMA_IFCR_CTCIF2);
}
