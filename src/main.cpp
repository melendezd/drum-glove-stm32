#include "io.hpp"
#include "status_indicator.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "dac.hpp"

#include "interrupts.hpp"
#include "global_constants.hpp"
#include "drum_machine.hpp"
#include <span>

#include "samples.hpp"

// ---- GPIO ----
const auto pin_led_port = gpio::Port::B;
const auto pin_led_pin  = 8;

const auto pin_amp_active_port = gpio::Port::A;
const auto pin_amp_active_pin  = 7;

const auto pin_trigger_1_port = gpio::Port::A;
const auto pin_trigger_1_pin  = 11;

// the following are commented out because no gpio setup is needed
// ("additional function" for DAC & ADC is set up through non-GPIO peripheral registers)

// PA4 used for DAC out
// const auto pin_dac_out_port = gpio::Port::A;
// const auto pin_dac_out_pin  = 4;

// ADC2_IN13
// const auto pin_ain_1_port = gpio::Port::A;
// const auto pin_ain1_pin  = 5;

// ADC2_IN3
// const auto pin_ain_2_port = gpio::Port::A;
// const auto pin_ain2_pin  = 6;

// ADC2_IN12
// const auto pin_ain_3_port = gpio::Port::B;
// const auto pin_ain3_pin  = 2;

// ---- Timers ----
const auto timer_delay_id = timer::Id::Tim6;
const auto timer_trigger_id = timer::Id::Tim7;

// ----------------

InterruptHandlers *g_interrupt_handlers = nullptr;

int main( void )
{
    InterruptHandlers       interrupt_handlers;
    g_interrupt_handlers = &interrupt_handlers;

    // this GPIO corresponds to the green user LED LD2 on the STM32G4 Nucleo board
    // here, this is used as an error indicator for if anything goes wrong
    GPIO pin_led({
        .port       = pin_led_port,
        .pin        = pin_led_pin,
        .mode       = gpio::Mode::Output,
        .outputType = gpio::OutputType::PushPull,
        .speed      = gpio::Speed::Low,
        .pull       = gpio::Pull::None,
    });
    DefaultInterruptHandler default_handler( pin_led );
    g_interrupt_handlers->default_handler = &default_handler;
    pin_led.unset();

    DelayTimer timer_delay( timer_delay_id, pin_led );
    TriggerTimer timer_dac_trigger({
        .id = timer_trigger_id,
        .one_pulse_mode = false,
        .auto_reload_value = constants::clock_frequency / constants::sample_rate,
        .prescaler_value = 0,
        .error_indicator = pin_led
    });
    GPIO pin_amp_active({
        .port       = pin_amp_active_port,
        .pin        = pin_amp_active_pin,
        .mode       = gpio::Mode::Output,
        .outputType = gpio::OutputType::PushPull,
        .speed      = gpio::Speed::Low,
        .pull       = gpio::Pull::Down
    });
    StatusIndicator indicator( pin_led, timer_delay );

    GPIO pin_trigger_1({
        .port       = pin_trigger_1_port,
        .pin        = pin_trigger_1_pin,
        .mode       = gpio::Mode::Input,
        .outputType = gpio::OutputType::PushPull,
        .speed      = gpio::Speed::Low,
        .pull       = gpio::Pull::None
    });

    const int buffer_len = 512;
    uint8_t buffer[buffer_len];
    std::span<uint8_t> buffer_span(buffer, buffer_len);
    for (uint8_t &sample : buffer_span) sample = 0;

    DrumMachine drum_machine(pin_trigger_1);

    AudioController audio({ 
        .indicator = indicator,
        .buffer = buffer_span,
        .timer = timer_dac_trigger,
        .delay = timer_delay,
        .amp_active = pin_amp_active,
        .drum_machine = drum_machine,
    });
    g_interrupt_handlers->dac_dma_underrun_handler = &audio;
    g_interrupt_handlers->dma1_channel1_handler = &audio;
    
    while (!audio.is_ready()) indicator.status_once(status::dac_not_ready);

    audio.start();

    bool button_was_down = false;
    bool button_is_down = false;
    int sample_index = 0;
    while(true) {
        button_was_down = button_is_down;
        button_is_down = pin_trigger_1.read();

        if (button_is_down && !button_was_down) {
            drum_machine.play(sample_index);
            sample_index = (sample_index + 1) % 3;
        }
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
