#include "adc.hpp"

// TODO: pass in trigger timer
ADCController::ADCController(DelayTimer &delay) : adc( ADC2 ), delay(delay)
{
    // enable ADC clock
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC12EN);

    // exit deep power-down mode and enable voltage regulator
    MODIFY_REG(
        adc->CR,
        ADC_CR_DEEPPWD | ADC_CR_ADVREGEN,
        ADC_CR_ADVREGEN
    );

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
    uint32_t sqr1_num_conversions = (6U - 1) << ADC_SQR1_L_Pos; // 6 conversions in a sequence
    uint32_t sqr1_conversion_1 = 3U << ADC_SQR1_SQ1_Pos;
    uint32_t sqr1_conversion_2 = 3U << ADC_SQR1_SQ2_Pos;
    //uint32_t sqr1_conversion_3 = 12U << ADC_SQR1_SQ3_Pos;
    //uint32_t sqr1_conversion_4 = 12U << ADC_SQR1_SQ4_Pos;
    //uint32_t sqr2_conversion_5 = 13U << ADC_SQR2_SQ5_Pos;
    //uint32_t sqr2_conversion_6 = 13U << ADC_SQR2_SQ6_Pos;
    uint32_t sqr1_conversion_3 = 3U << ADC_SQR1_SQ3_Pos;
    uint32_t sqr1_conversion_4 = 3U << ADC_SQR1_SQ4_Pos;
    uint32_t sqr2_conversion_5 = 3U << ADC_SQR2_SQ5_Pos;
    uint32_t sqr2_conversion_6 = 3U << ADC_SQR2_SQ6_Pos;
    uint32_t sqr1_conversions = 
        sqr1_conversion_1
            | sqr1_conversion_2
            | sqr1_conversion_3
            | sqr1_conversion_4;
    uint32_t sqr2_conversions = sqr2_conversion_5 | sqr2_conversion_6;

    MODIFY_REG(
        adc->SQR1,
        0xffffffff,
        sqr1_num_conversions | sqr1_conversions
    );
    MODIFY_REG(
        adc->SQR2,
        0xffffffff,
        sqr2_conversions
    );

    uint32_t adc_cfgr_reset = 0x80000000U;
    uint32_t adc_cfgr_resolution = 0; // TODO: data resolution & timing
    MODIFY_REG(
        adc->CFGR,
        adc_cfgr_reset,
        ADC_CFGR_DMAEN
            | ADC_CFGR_DMACFG // DMA circular mode
            | adc_cfgr_resolution
            | ADC_CFGR_CONT // continuous conversion mode
            | ADC_CFGR_JQDIS // injected queue disabled
    );

    // TODO: configure DMA
}
