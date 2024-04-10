#include "interrupts.hpp"

extern "C"
{
    void default_interrupt_handler(void)
    {
        g_interrupt_handlers->default_handler.isr();
    }

    void isr_dac_dma_underrun(void)
    {
        g_interrupt_handlers->dac_dma_underrun_handler.isr_dma_underrun();
    }
}
