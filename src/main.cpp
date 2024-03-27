#include "io.hpp"
#include "timer.hpp"
#include "util.hpp"

#include "interrupts.hpp"

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
        { .port       = pin_led_port,
          .pin        = pin_led_pin,
          .mode       = gpio::Mode::Output,
          .outputType = gpio::OutputType::PushPull,
          .speed      = gpio::Speed::Low,
          .pull       = gpio::Pull::None }
    );
    pin_led.unset();

    // used to test the timer
    GPIO pin_test(
        { .port       = pin_test_port,
          .pin        = pin_test_pin,
          .mode       = gpio::Mode::Output,
          .outputType = gpio::OutputType::PushPull,
          .speed      = gpio::Speed::Low,
          .pull       = gpio::Pull::None }
    );

    DelayTimer timer_delay(timer_delay_id, pin_led);

    DefaultInterruptHandler default_handler(pin_led);
    InterruptHandlers interrupt_handlers { .default_handler = default_handler };
    g_interrupt_handlers = &interrupt_handlers;

    while ( 1 ) { 
        pin_test.toggle();
        timer_delay.ms(100);
    }

    return 0;
}

DefaultInterruptHandler::DefaultInterruptHandler(GPIO &pin) : pin(pin) {}

void DefaultInterruptHandler::isr()
{
    while(1) {
        pin.toggle();
        spin(50'000);
    }
}
