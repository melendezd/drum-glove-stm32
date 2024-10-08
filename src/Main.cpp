#include "Gpio.hpp"
#include "StatusIndicator.hpp"
#include "Timer.hpp"
#include "Util.hpp"
#include "AudioController.hpp"
#include "ADCController.hpp"
#include "TapDetector.hpp"

#include "InterruptHandlers.hpp"
#include "GlobalConstants.hpp"
#include "DrumMachine.hpp"
#include <span>

#include "samples.hpp"

// the following are commented out because no gpio setup is needed
// ("additional function" for DAC & ADC is set up through non-GPIO peripheral registers)

// PA4 used for DAC out
// const auto pin_dac_out_port = gpio::Port::A;
// const auto pin_dac_out_pin  = 4;

// ADC2_IN13
// const auto pin_ain_1_port = gpio::Port::A;
// const auto pin_ain1_pin  = 5;

// ADC2_IN3
// const auto pin_ain_port = gpio::Port::F;
// const auto pin_ain_pin  = 0;

// ADC2_IN12
// const auto pin_ain_3_port = gpio::Port::B;
// const auto pin_ain3_pin  = 2;

// ----------------

InterruptHandlers *g_interrupt_handlers = nullptr;

int main( void )
{
    InterruptHandlers       interrupt_handlers;
    g_interrupt_handlers = &interrupt_handlers;

    // this GPIO corresponds to the green user LED LD2 on the STM32G4 Nucleo board
    // here, this is used as an error indicator for if anything goes wrong
    GpioPin pin_led({
        .port       = hardware_constants::pin_led_port,
        .pin        = hardware_constants::pin_led_pin,
        .mode       = gpio::Mode::Output,
        .outputType = gpio::OutputType::PushPull,
        .speed      = gpio::Speed::Low,
        .pull       = gpio::Pull::None,
    });
    DefaultInterruptHandler default_handler( pin_led );
    g_interrupt_handlers->default_handler = &default_handler;
    pin_led.unset();

    DelayTimer timer_delay( hardware_constants::timer_delay_id, pin_led );
    TriggerTimer timer_dac_trigger({
        .id = hardware_constants::timer_trigger_id,
        .one_pulse_mode = false,
        .auto_reload_value = constants::clock_frequency / constants::sample_rate,
        .prescaler_value = 0,
        .error_indicator = pin_led
    });
    GpioPin pin_amp_active({
        .port       = hardware_constants::pin_amp_active_port,
        .pin        = hardware_constants::pin_amp_active_pin,
        .mode       = gpio::Mode::Output,
        .outputType = gpio::OutputType::PushPull,
        .speed      = gpio::Speed::Low,
        .pull       = gpio::Pull::Down
    });
    StatusIndicator indicator( pin_led, timer_delay );

    GpioPin pin_trigger_1({
        .port       = hardware_constants::pin_trigger_1_port,
        .pin        = hardware_constants::pin_trigger_1_pin,
        .mode       = gpio::Mode::Input,
        .outputType = gpio::OutputType::PushPull,
        .speed      = gpio::Speed::Low,
        .pull       = gpio::Pull::None
    });

    uint8_t buffer[constants::audio_out_buffer_length];
    std::span<uint8_t> audio_out_buffer_span(buffer, constants::audio_out_buffer_length);
    for (uint8_t &sample : audio_out_buffer_span) sample = 0;

    DrumMachine drum_machine(pin_trigger_1);
    TapDetector tap_detector;

    ADCController adc(timer_delay, indicator, tap_detector);
    g_interrupt_handlers->dma1_channel2_handler = &adc;

    AudioController audio({ 
        .indicator = indicator,
        .audio_out_buffer = audio_out_buffer_span,
        .timer = timer_dac_trigger,
        .delay = timer_delay,
        .amp_active = pin_amp_active,
        .drum_machine = drum_machine,
    });
    g_interrupt_handlers->dac_dma_underrun_handler = &audio;
    g_interrupt_handlers->dma1_channel1_handler = &audio;
    
    while (!audio.is_ready()) indicator.status_once(status::dac_not_ready);

    adc.start();
    audio.start();

    while(true) {
        for (int i = 0; i < constants::sample_count; i++) {
            uint8_t tap_strength = tap_detector.check_and_clear_tap_strength(i);
            if (tap_strength > 0) {
                drum_machine.play(i, tap_strength);
            }
        }
    }

    return 0;
}

DefaultInterruptHandler::DefaultInterruptHandler( GpioPin &pin ) : pin( pin ) { }

void DefaultInterruptHandler::isr()
{
    while ( 1 ) {
        pin.toggle();
        spin( 50'000 );
    }
}
