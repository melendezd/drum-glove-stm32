#pragma once

#include "io.hpp"
#include "mcu.hpp"
#include <cstdint>

namespace timer
{
enum class Id
{
    Tim6,
    Tim7
};
} // namespace timer


class DelayTimer
{
  public:
      DelayTimer(timer::Id id, GPIO &error_indicator);
      void us( uint16_t us );
      void ms( uint16_t ms );

  private:
      TIM_TypeDef *tim;

      GPIO &error_indicator;

      TIM_TypeDef *get_timer_base(timer::Id timer_id);
      void enable_clock(timer::Id timer_id);
      void error();
};


