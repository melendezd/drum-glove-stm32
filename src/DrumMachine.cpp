#include "DrumMachine.hpp"
#include <algorithm>
#include <limits>

DrumMachine::DrumMachine(Gpio &trigger1) : trigger1(trigger1) {}

void DrumMachine::fill_buffer(std::span<uint8_t> target)
{
    int val = 0;
    const int upper_limit = static_cast<int>(std::numeric_limits<uint8_t>::max());
    const int half_full = upper_limit / 2;

    for (uint8_t &out : target) {
        val = 0;
        for (Sample &sample : samples) {
            if (!sample.is_playing) continue;
            val += static_cast<int>(sample.source[sample.position]) - half_full;
            sample.position++;

            // stop playing when we reach the end of the sample
            if (sample.position >= sample.size) sample.is_playing = false;
        }

        // mix samples equally
        val /= static_cast<int>(samples.size());
        //val /= 3;
        // ensure that result is in uint8_t bounds
        out = std::clamp(val + half_full, 0, upper_limit);
    }
}

void DrumMachine::play(int sample_id)
{
    if (!is_sample_id_in_bounds(sample_id)) return;

    samples[sample_id].position = 0;
    samples[sample_id].is_playing = true;
}

bool DrumMachine::is_sample_id_in_bounds(int index)
{
    return index <= samples.size();
}
