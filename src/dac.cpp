#include "dac.hpp"

#include "global_constants.hpp"
#include "mcu.hpp"
#include "stm32g431xx.h"
#include "util.hpp"

AudioController::AudioController( dac::Settings settings )
    : dac( DAC )
    , dma( DMA1_Channel1 )
    , dmamux( DMAMUX1_Channel0 )
    , channel( settings.channel )
    , indicator( settings.indicator )
{
    configure_dma();
    configure_dac();

    enable_dma();
}

void AudioController::configure_dac()
{
    // TODO (somewhere): configure dac trigger timer
    SET_BIT( RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN );

    // ENx = 0: disable DAC for now
    // TENx = 0: trigger disabled, data is shifted one dac_hclk cycle after it is written
    // TSELx = 0: if trigger enabled, trigger is writing to SWTRIGx (software trigger)
    uint32_t cr = 0;

    MODIFY_REG( dac->CR, 0xFFFFFFFFU, cr );

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

    // configure DMA
    // MSIZE is left reset, so memory size is 8 bits
    MODIFY_REG(
        dma->CCR,
        (1 << 15) - 1,
        DMA_CCR_DIR // ready from memory
            | DMA_CCR_CIRC // circular mode
            | DMA_CCR_MINC // memory increment mode
            | DMA_CCR_PSIZE_1 // (0b10) 32 bit peripheral size
            | (DMA_CCR_PL_0 | DMA_CCR_PL_1) // (0b11) highest priority level for real-time audio
    );

    // TODO: configure DMA channel (no enable)
    // TODO: configure DMAMUX channel
    // TODO: enable DMA channel
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

void AudioController::enable_dma()
{
    SET_BIT( dac->CR, apply_channel( DAC_CR_EN1 ) );
}

bool AudioController::write_if_ready( uint32_t data )
{
    if ( is_ready() ) {
        write( data );
        return true;
    } else
        return false;
}

void AudioController::write( uint32_t data )
{
    *data_register = data;
}
