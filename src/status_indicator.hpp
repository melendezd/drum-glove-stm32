#pragma once

#include "io.hpp"
#include "timer.hpp"

// Allows you to toggle an output to a GPIO pin indicating a status code.
// Useful for indicating errors.
//
// Dependencies:
// - GPIO out: Pin to which the status will be output. Recommended to use an LED.
// - DelayTimer delay: Delay timer which will be used for timing in the output pattern.
class StatusIndicator
{
  public:
    StatusIndicator( GPIO &out, DelayTimer &delay );

    // This function will loop forever.
    //
    // Toggles the output pin in a pattern to indicate the status code forever.
    // The pattern is: One long blink, and then `status_code` short blinks
    void status( uint32_t status_code );

    // Toggles the output pin in a pattern to indicate the status code once.
    // The pattern is: One long blink, and then `status_code` short blinks
    void status_once( uint32_t status_code );

  private:
    GPIO       &out;
    DelayTimer &delay;
};
