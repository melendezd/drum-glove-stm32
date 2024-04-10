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

const auto pin_amp_active_port = gpio::Port::A;
const auto pin_amp_active_pin  = 7;

// ---- Timers ----
const auto timer_delay_id = timer::Id::Tim6;
const auto timer_trigger_id = timer::Id::Tim7;

// ----------------

InterruptHandlers *g_interrupt_handlers = nullptr;

void fill_buffer(std::span<uint8_t> buffer, uint8_t delta);

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
        .auto_reload_value = 100,
        .prescaler_value = 0,
        .error_indicator = pin_led
    });

    GPIO pin_amp_active(
        {
            .port       = pin_amp_active_port,
            .pin        = pin_amp_active_pin,
            .mode       = gpio::Mode::Output,
            .outputType = gpio::OutputType::PushPull,
            .speed      = gpio::Speed::Low,
            .pull       = gpio::Pull::Down
        }
    );

    StatusIndicator indicator( pin_led, timer_delay );

    const std::size_t buffer_size = 1024;
    uint8_t buffer_raw[buffer_size];
    std::span<uint8_t> buffer(buffer_raw, buffer_size);

    // init buffer values
    fill_buffer(buffer, 1);

    timer_delay.ms(500);

    AudioController audio({ 
        .channel = dac::Channel::One,
        .indicator = indicator,
        .buffer = buffer,
        .timer = timer_trigger,
        .amp_active = pin_amp_active,
    });

    DefaultInterruptHandler default_handler( pin_led );
    InterruptHandlers       interrupt_handlers {
        .default_handler = default_handler,
        .dac_dma_underrun_handler = audio,
    };
    g_interrupt_handlers = &interrupt_handlers;
    
    
    if (!audio.is_ready()) indicator.status_once(status::dac_not_ready);

    uint16_t delay_time_ms = 100;
    fill_buffer(buffer, 1);
    audio.start();
    while(true) {
        fill_buffer(buffer, 1);
        timer_delay.ms(delay_time_ms);
        fill_buffer(buffer, 2);
        timer_delay.ms(delay_time_ms);
        fill_buffer(buffer, 3);
        timer_delay.ms(delay_time_ms);
    }

    while(true)
    {
        indicator.status_once(status::ok);
    }

    return 0;
}

void fill_buffer(std::span<uint8_t> buffer, uint8_t delta)
{
    uint8_t val = 1;
    for (int i = 0; i < buffer.size(); i++) {
        buffer[i] = val;
        val += delta;
        if (val == 255 || val == 0) delta *= -1;
    }
}

DefaultInterruptHandler::DefaultInterruptHandler( GPIO &pin ) : pin( pin ) { }

void DefaultInterruptHandler::isr()
{
    while ( 1 ) {
        pin.toggle();
        spin( 50'000 );
    }
}
