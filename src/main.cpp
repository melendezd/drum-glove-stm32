#include "io.hpp"
#include "interrupts.hpp"

// ---- GPIO ----
const auto pin_led_port = gpio::Port::B;
const auto pin_led_pin  = 8;

void woof( unsigned int cycles );

InterruptHandlers *g_interrupt_handlers = nullptr;

int main( void )
{
    GPIO pin_led(
        { .port       = pin_led_port,
          .pin        = pin_led_pin,
          .mode       = gpio::Mode::Output,
          .outputType = gpio::OutputType::PushPull,
          .speed      = gpio::Speed::Low,
          .pull       = gpio::Pull::None }
    );

    DefaultInterruptHandler default_handler(pin_led);
    InterruptHandlers interrupt_handlers { .default_handler = default_handler };
    g_interrupt_handlers = &interrupt_handlers;

    // testing timing for the ISR via logic analyzer
    while ( 1 ) { 
        pin_led.toggle();
        woof(100'000);
    }

    return 0;
}

void woof( unsigned int cycles )
{
    for ( volatile unsigned int x = 0; x < cycles; x++ )
        ;
}


DefaultInterruptHandler::DefaultInterruptHandler(GPIO &pin) : pin(pin) {}

void DefaultInterruptHandler::isr()
{
    while(1) {
        pin.toggle();
        woof(50'000);
    }
}
