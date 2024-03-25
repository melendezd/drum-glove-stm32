#pragma once
#include "stm32g431xx.h"

namespace gpio
{
enum class Port
{
    A, B, C, D, E, F, G
};
enum class Mode
{
    Input,
    Output,
    AlternateFunction,
    Analog,
};

enum class OutputType
{
    PushPull,
    OpenDrain,
};

enum class Speed
{
    Low,
    Medium,
    High,
    VeryHigh,
};

enum class Pull
{
    None,
    Up,
    Down,
};

struct Settings
{
    Port port;
    int pin;
    Mode mode;
    OutputType outputType;
    Speed speed;
    Pull pull;
};
}

constexpr GPIO_TypeDef *const get_port(gpio::Port port);
constexpr uint32_t get_clock_enable_mask(gpio::Port port);

class GPIO
{
  public:
    GPIO(gpio::Settings settings);

    void set();
    void unset();
    void toggle();

    void write(int val);
    int read();

  private:
    void reset_registers();
    GPIO_TypeDef *port;
    int pin;
};
