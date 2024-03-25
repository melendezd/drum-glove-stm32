#pragma once

#include "io.hpp"

struct DefaultInterruptHandler
{
    DefaultInterruptHandler(GPIO &pin);
    GPIO &pin;
    void isr();
};

struct InterruptHandlers
{
    DefaultInterruptHandler &default_handler;
};

extern InterruptHandlers *g_interrupt_handlers; 
