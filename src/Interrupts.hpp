#pragma once

#include "Gpio.hpp"
#include "AudioController.hpp"
#include "ADCController.hpp"

struct DefaultInterruptHandler
{
    DefaultInterruptHandler(Gpio &pin);
    Gpio &pin;
    void isr();
};

struct InterruptHandlers
{
    DefaultInterruptHandler *default_handler;
    AudioController *dac_dma_underrun_handler;
    AudioController *dma1_channel1_handler;
    ADCController *dma1_channel2_handler;
};

extern InterruptHandlers *g_interrupt_handlers; 
