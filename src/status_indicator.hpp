#pragma once

#include "io.hpp"
#include "timer.hpp"

class StatusIndicator
{
  public:
    StatusIndicator( GPIO &out, DelayTimer &delay );
    void status( uint32_t status_code );
    void status_once( uint32_t status_code );

  private:
    GPIO       &out;
    DelayTimer &delay;
};
