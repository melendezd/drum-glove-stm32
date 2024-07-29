#include "TapDetector.hpp"

TapDetector::TapDetector() { }

void TapDetector::process_frame(
    std::array< std::array< volatile uint8_t, constants::adc_window_length >, constants::sample_count > &in_buffer
)
{
    for (int sample_index = 0; sample_index < constants::sample_count; sample_index++) {
        const auto &window = in_buffer[sample_index];
        auto &current_samples_after_threshold_count = samples_after_threshold_count[sample_index];
        for (const auto &value : window) {
            // check if piezo reading is past the threshold, indicating a tap
            if (value > trigger_threshold) current_samples_after_threshold_count = 1;

            if (current_samples_after_threshold_count > 0) {
                // a tap has been detected at some point.
                // we are now figuring out the strength of the tap.
                
                // find the peak of the window after the threshold was passed
                if (value > max_value[sample_index]) max_value[sample_index] = value;

                current_samples_after_threshold_count++;
                if (current_samples_after_threshold_count > after_threshold_window_length) {
                    // finished finding the tap strength after threshold passed.
                    // output tap strength and reset.
                    tap_detected_strength[sample_index] = max_value[sample_index];
                    max_value[sample_index] = 0;
                    current_samples_after_threshold_count = 0;
                }
            }
        }
    }
}

uint8_t TapDetector::check_and_clear_tap_strength(int sample_index)
{
    uint8_t tap_strength = tap_detected_strength[sample_index];
    tap_detected_strength[sample_index] = 0;
    return tap_strength;
}
