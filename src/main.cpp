#include "io.hpp"
#include "status_indicator.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "dac.hpp"

#include "interrupts.hpp"
#include "global_constants.hpp"
#include <span>

// ---- GPIO ----
const auto pin_led_port = gpio::Port::B;
const auto pin_led_pin  = 8;

const auto pin_test_port = gpio::Port::B;
const auto pin_test_pin  = 5;

// ---- Timers ----
const auto timer_delay_id = timer::Id::Tim6;
const auto timer_trigger_id = timer::Id::Tim7;

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
    TriggerTimer timer_trigger({
        .id = timer_trigger_id,
        .one_pulse_mode = false,
        .auto_reload_value = 1000,
        .prescaler_value = 0,
        .error_indicator = pin_led
    });

    StatusIndicator indicator( pin_led, timer_delay );

    const std::size_t buffer_size = 256;
    uint8_t buffer_raw[buffer_size];
    std::span<uint8_t> buffer(buffer_raw, 256);

    // init buffer values
    uint8_t val = 0;
    uint8_t delta = 1;
    for (int i = 0; i < buffer.size(); i++) {
        buffer[i] = val;
        val += delta;
    }

    AudioController audio({ 
        .channel = dac::Channel::One,
        .indicator = indicator,
        .buffer = buffer,
        .timer = timer_trigger
    });
    
    //timer_delay.ms(500);
    
    if (!audio.is_ready()) indicator.status_once(status::dac_not_ready);

    audio.start();

    while(true)
    {
        indicator.status_once(status::ok);
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
