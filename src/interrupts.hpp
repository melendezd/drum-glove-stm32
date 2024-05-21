#pragma once

#include "io.hpp"
#include "dac.hpp"

struct DefaultInterruptHandler
{
    DefaultInterruptHandler(GPIO &pin);
    GPIO &pin;
    void isr();
};

struct InterruptHandlers
{
    DefaultInterruptHandler *default_handler;
    AudioController *dac_dma_underrun_handler;
    AudioController *dma1_channel1_handler;
};

extern InterruptHandlers *g_interrupt_handlers; 
