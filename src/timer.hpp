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

struct TriggerTimerSettings
{
    Id id;
    bool one_pulse_mode;
    uint16_t auto_reload_value;
    uint16_t prescaler_value;
    GPIO &error_indicator;
};

} // namespace timer


// Timer used for microsecond and millisecond delays
class DelayTimer
{
  public:
      DelayTimer(timer::Id id, GPIO &error_indicator);

      // Spin for the specified number of microseconds
      void us( uint16_t us );

      // Spin for the specified number of milliseconds
      void ms( uint16_t ms );

  private:
      TIM_TypeDef *tim;

      GPIO &error_indicator;

      TIM_TypeDef *get_timer_base(timer::Id timer_id);
      void enable_clock(timer::Id timer_id);
      void error();
};


class TriggerTimer
{
  public:
      TriggerTimer(timer::TriggerTimerSettings settings);

      void start();
      void stop();

      timer::Id id;

  private:
      TIM_TypeDef *tim;

      GPIO &error_indicator;

      TIM_TypeDef *get_timer_base(timer::Id timer_id);
      void enable_clock(timer::Id timer_id);
      void error();
};
