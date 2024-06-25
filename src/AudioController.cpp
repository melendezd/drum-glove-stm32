#include "AudioController.hpp"

#include "GlobalConstants.hpp"
#include "McuIncludes.hpp"
#include "stm32g431xx.h"

AudioController::AudioController( audio::Settings settings )
    : dac( hardware_constants::dac_audio_controller )
    , dma( hardware_constants::dma_channel_audio_controller )
    , dma_isr( hardware_constants::dma_audio_controller )
    , dmamux( hardware_constants::dmamux_audio_controller )
    , indicator( settings.indicator )
    , timer( settings.timer )
    , delay( settings.delay )
    , amp_active( settings.amp_active )
    , drum_machine( settings.drum_machine )
    , adc_buffer( settings.adc_buffer )
    , stale_buffer_index( 0 )
    , buffer( settings.buffer )
    , buffers{
          { settings.buffer.data(), settings.buffer.size() / 2 },
          { settings.buffer.subspan(settings.buffer.size()/2, settings.buffer.size() - settings.buffer.size()/2)}
      }
{
    configure_dac();
    configure_dma();

    enable_dac();
    enable_dma();
}

void AudioController::configure_dac()
{
    SET_BIT( RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN );

    // We ensure the DAC_MCR configuration register is at its reset value 0x00000000. Hence:
    // - Input data is in unsigned format
    // - Channel is connected to external pin with buffer enabled
    CLEAR_REG( dac->MCR );

    // ENx = 0: disable DAC for now
    // TENx = 0: trigger disabled, data is shifted one dac_hclk cycle after it is written
    // TSELx = 0: if trigger enabled, trigger is writing to SWTRIGx (software trigger)
    uint32_t dac_cr_mask = 0xFFFFFFFFU & ~( ( 1U << 15 ) | ( 1U << 31 ) );
    MODIFY_REG(
        dac->CR, dac_cr_mask,
        DAC_CR_TEN1                                // enable trigger
            | get_tsel_value() << DAC_CR_TSEL1_Pos // trigger from timer
            | DAC_CR_DMAEN1                        // enable DMA mode
            | DAC_CR_DMAUDRIE1                     // enable DMA underrun interrupt
    );

    // enable interrupt
    NVIC_ClearPendingIRQ( TIM6_DAC_IRQn );
    NVIC_SetPriority( TIM6_DAC_IRQn, 1u );
    NVIC_EnableIRQ( TIM6_DAC_IRQn );

    NVIC_ClearPendingIRQ( hardware_constants::dma_irq_audio_controller );
    NVIC_SetPriority( hardware_constants::dma_irq_audio_controller, 1u );
    NVIC_EnableIRQ( hardware_constants::dma_irq_audio_controller );

    data_register = &dac->DHR8R1;
}

bool AudioController::is_status_dma_underrun()
{
    return READ_BIT( dac->SR, DAC_SR_DMAUDR1 ) != 0;
}

void AudioController::configure_dma()
{
    // enable DMA and DMAMUX clocks
    SET_BIT( RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN );

    // configure DMA control register
    // MSIZE is left at its reset value, so memory size is 8 bits
    MODIFY_REG(
        dma->CCR, ( 1 << 15 ) - 1, DMA_CCR_DIR                           // read from memory
            | DMA_CCR_CIRC                    // circular mode
            | DMA_CCR_MINC                    // memory increment mode
            | DMA_CCR_PSIZE_1                 // (0b10) 32 bit peripheral size
            | ( DMA_CCR_PL_0 | DMA_CCR_PL_1 ) // (0b11) highest priority level for real-time audio
            | DMA_CCR_HTIE                    // enable half transfer interrupt
            | DMA_CCR_TCIE                    // enable transfer complete interrupt
    );

    // DMA CNDTR - number of data to transfer
    MODIFY_REG( dma->CNDTR, 0xffffU, static_cast<uint32_t>(buffer.size()) );

    // DMA CPAR - peripheral address
    MODIFY_REG( dma->CPAR, 0xffffffffU, reinterpret_cast< uint32_t >( data_register ) );

    // DMA CMAR - memory address
    MODIFY_REG( dma->CMAR, 0xffffffffU, reinterpret_cast< uint32_t >( buffer.data() ) );

    // configure DMAMUX
    const uint32_t dmamux_ccr_mask = DMAMUX_CxCR_DMAREQ_ID | DMAMUX_CxCR_SOIE | DMAMUX_CxCR_EGE | DMAMUX_CxCR_SE |
                                     DMAMUX_CxCR_SE | DMAMUX_CxCR_SPOL | DMAMUX_CxCR_NBREQ | DMAMUX_CxCR_SYNC_ID;
    MODIFY_REG(
        dmamux->CCR, dmamux_ccr_mask,
        0x6 << DMAMUX_CxCR_DMAREQ_ID_Pos // DAC1_CH1
    );
}

void AudioController::enable_dma()
{
    SET_BIT( dma->CCR, DMA_CCR_EN );
}

bool AudioController::is_ready()
{
    return READ_BIT( dac->SR, DAC_SR_DAC1RDY ) != 0;
}

void AudioController::enable_dac()
{
    SET_BIT( dac->CR, DAC_CR_EN1 );
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
    amp_active.set();
    timer.start();
}

void AudioController::stop()
{
    timer.stop();
    amp_active.unset();
}

bool AudioController::is_status_half_transfer()
{
    return READ_BIT( dma_isr->ISR, DMA_ISR_HTIF1 ) != 0;
}

bool AudioController::is_status_full_transfer()
{
    return READ_BIT( dma_isr->ISR, DMA_ISR_TCIF1 ) != 0;
}

void AudioController::isr_dma_underrun()
{
    if ( is_status_dma_underrun() )
        indicator.status_forever( status::dac_dma_underrun );
}

void AudioController::isr_dma()
{
    // the first half transfer interrupt happens after the DMA reads the first half of the buffer,
    // so the stale buffer starts index at 0
    if ( is_status_full_transfer() || is_status_half_transfer() )
    {
        drum_machine.fill_buffer(buffers[stale_buffer_index]);

        // set up stale buffer index for next interrupt
        stale_buffer_index ^= 1;
    }
    SET_BIT(dma_isr->IFCR, DMA_IFCR_CGIF1);
}
