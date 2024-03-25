#include "interrupts.hpp"

extern "C"
{
    void default_interrupt_handler(void)
    {
        g_interrupt_handlers->default_handler.isr();
    }
}
