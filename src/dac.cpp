#include "dac.hpp"

#include "mcu.hpp"
#include "stm32g431xx.h"

// TODO: make this actually work for channels other than channel 1
AudioController::AudioController( dac::Settings settings )
    : dac( DAC )
    , dma( DMA1_Channel1 )
    , dmamux( DMAMUX1_Channel0 )
    , channel( settings.channel )
    , indicator( settings.indicator )
    , timer( settings.timer )
    , buffer( settings.buffer )
{
    configure_dma();
    configure_dac();
    enable_dac();
}

void AudioController::configure_dac()
{
    // TODO (somewhere): configure dac trigger timer
    SET_BIT( RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN );

    // ENx = 0: disable DAC for now
    // TENx = 0: trigger disabled, data is shifted one dac_hclk cycle after it is written
    // TSELx = 0: if trigger enabled, trigger is writing to SWTRIGx (software trigger)
    uint32_t dac_cr_mask = 0xFFFFFFFFU & ~((1U << 15) | (1U << 31));

    MODIFY_REG(
        dac->CR,
        dac_cr_mask,
        DAC_CR_TEN1 // enable trigger
            | get_tsel_value() << DAC_CR_TSEL1_Pos // trigger from timer
            | DAC_CR_DMAEN1 // enable DMA mode
    );

    // We ensure the DAC_MCR configuration register is at its reset value 0x00000000. Hence:
    // - Input data is in unsigned format
    // - Channel is connected to external pin with buffer enabled
    CLEAR_REG( dac->MCR );

    switch ( channel ) {
    case dac::Channel::One:
        data_register = &dac->DHR8R1;
        break;
    case dac::Channel::Two:
        data_register = &dac->DHR8R2;
        break;
    }
}

void AudioController::configure_dma()
{
    // enable DMA and DMAMUX clocks
    SET_BIT( RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN );

    // configure DMA control register
    // MSIZE is left at its reset value, so memory size is 8 bits
    MODIFY_REG(
        dma->CCR,
        (1 << 15) - 1,
        DMA_CCR_DIR // read from memory
            | DMA_CCR_CIRC // circular mode
            | DMA_CCR_MINC // memory increment mode
            | DMA_CCR_PSIZE_1 // (0b10) 32 bit peripheral size
            | (DMA_CCR_PL_0 | DMA_CCR_PL_1) // (0b11) highest priority level for real-time audio
    );

    // DMA CNDTR - number of data to transfer
    MODIFY_REG(dma->CNDTR, 0xffffU, buffer.size());

    // DMA CPAR - peripheral address
    MODIFY_REG(dma->CPAR, 0xffffffffU, reinterpret_cast<uint32_t>(data_register));

    // DMA CMAR - memory address
    MODIFY_REG(dma->CMAR, 0xffffffffU, reinterpret_cast<uint32_t>(buffer.data()));

    // configure DMAMUX
    const uint32_t dmamux_ccr_mask = 
        DMAMUX_CxCR_DMAREQ_ID
        | DMAMUX_CxCR_SOIE
        | DMAMUX_CxCR_EGE
        | DMAMUX_CxCR_SE
        | DMAMUX_CxCR_SE
        | DMAMUX_CxCR_SPOL
        | DMAMUX_CxCR_NBREQ
        | DMAMUX_CxCR_SYNC_ID;
    MODIFY_REG(
        dmamux->CCR,
        dmamux_ccr_mask,
        0x6 << DMAMUX_CxCR_DMAREQ_ID_Pos // DAC1_CH1
    );
    
    // enable DMA
    SET_BIT(dma->CCR, DMA_CCR_EN);
}

void AudioController::configure_timer()
{
}

constexpr uint32_t AudioController::apply_channel( uint32_t config )
{
    int channel_number = 0;
    switch ( channel ) {
    case dac::Channel::One:
        channel_number = 0;
        break;
    case dac::Channel::Two:
        channel_number = 1;
        break;
    }
    return config << channel_number * 16;
}

bool AudioController::is_ready()
{
    return READ_BIT( dac->SR, apply_channel( DAC_SR_DAC1RDY ) ) != 0;
}

void AudioController::enable_dac()
{
    SET_BIT( dac->CR, apply_channel( DAC_CR_EN1 ) );
}

uint32_t AudioController::get_tsel_value()
{
    uint32_t val = 0;
    switch ( timer.id ) {
        case timer::Id::Tim6:
            val = 7U;
            break;
        case timer::Id::Tim7:
            val = 2U;
            break;
    }

    return val;
}

void AudioController::start()
{
    timer.start();
}

void AudioController::stop()
{
    timer.stop();
}
