#pragma once

#include <span>
#include "Gpio.hpp"
#include "samples.hpp"

class DrumMachine
{
  public:
    DrumMachine(GpioPin &trigger1);
    void fill_buffer(std::span<uint8_t> target);
    void play(int index);

  private:
    bool is_sample_id_in_bounds(int index);

    struct Sample
    {
        std::span<uint8_t> source;
        volatile bool is_playing = false;
        volatile int position = 0;
        int size = static_cast<int>(source.size());
    };
    std::array<Sample, 3> samples = {{
        { .source = std::span(samples::kick, samples::kick_len) },
        { .source = std::span(samples::snare, samples::snare_len) },
        { .source = std::span(samples::woody, samples::woody_len) },
    }};

    GpioPin &trigger1;
    // GpioPin &trigger2;
    // GpioPin &trigger3;
};
