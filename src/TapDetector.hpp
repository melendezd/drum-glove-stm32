#pragma once

#include "GlobalConstants.hpp"
#include <array>
#include <cstdint>

class TapDetector
{
  public:
    TapDetector();
    void process_frame(
        std::array< std::array< volatile uint8_t, constants::adc_window_length >, constants::sample_count > &out_buffer
    );
    volatile uint8_t tap_detected_strength [ constants::sample_count ] = { 0, 0, 0 };
    uint8_t check_and_clear_tap_strength(int sample_index);

  private:
    uint8_t max_value [ constants::sample_count ]                     = { 0, 0, 0 };
    uint8_t samples_after_threshold_count [ constants::sample_count ] = { 0, 0, 0 };
    uint8_t trigger_threshold                                         = constants::piezo_trigger_threshold;
    int     after_threshold_window_length                             = constants::piezo_after_threshold_window_length;
    bool    is_after_threshold( int sample );
};
