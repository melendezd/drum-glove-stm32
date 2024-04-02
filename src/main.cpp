#include "io.hpp"
#include "status_indicator.hpp"
#include "timer.hpp"
#include "util.hpp"

#include "dac.hpp"
#include "interrupts.hpp"
#include "global_constants.hpp"

// ---- GPIO ----
const auto pin_led_port = gpio::Port::B;
const auto pin_led_pin  = 8;

const auto pin_test_port = gpio::Port::B;
const auto pin_test_pin  = 5;

// ---- Timers ----
const auto timer_delay_id = timer::Id::Tim6;

// ----------------

InterruptHandlers *g_interrupt_handlers = nullptr;

int main( void )
{
    // this GPIO corresponds to the green user LED LD2 on the STM32G4 Nucleo board
    // here, this is used as an error indicator for if anything goes wrong
    GPIO pin_led(
        {
            .port       = pin_led_port,
            .pin        = pin_led_pin,
            .mode       = gpio::Mode::Output,
            .outputType = gpio::OutputType::PushPull,
            .speed      = gpio::Speed::Low,
            .pull       = gpio::Pull::None
        }
    );
    pin_led.unset();
    DefaultInterruptHandler default_handler( pin_led );
    InterruptHandlers       interrupt_handlers{ .default_handler = default_handler };
    g_interrupt_handlers = &interrupt_handlers;

    // used to test the timer
    GPIO pin_test(
        {
            .port       = pin_test_port,
            .pin        = pin_test_pin,
            .mode       = gpio::Mode::Output,
            .outputType = gpio::OutputType::PushPull,
            .speed      = gpio::Speed::Low,
            .pull       = gpio::Pull::None
        }
    );

    DelayTimer timer_delay( timer_delay_id, pin_led );

    StatusIndicator indicator( pin_led, timer_delay );

    DacController dac( { .channel = dac::Channel::One, .indicator = indicator } );

    uint8_t val = 0;
    int delta = 1;
    while ( 1 ) {
        timer_delay.us( 500 );
        while ( !dac.write_if_ready( val ) ) {
            indicator.status_once( status::dac_not_ready );
        }
        val += delta;
        if (val == 0 || val == 255) delta *= -1;
    }

    return 0;
}

DefaultInterruptHandler::DefaultInterruptHandler( GPIO &pin ) : pin( pin ) { }

void DefaultInterruptHandler::isr()
{
    while ( 1 ) {
        pin.toggle();
        spin( 50'000 );
    }
}
