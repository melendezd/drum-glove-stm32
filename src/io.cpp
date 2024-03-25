#include "io.hpp"
#include "stm32g431xx.h"

constexpr GPIO_TypeDef *const get_port( gpio::Port port )
{
    switch ( port ) {
    case gpio::Port::A:
        return GPIOA;
    case gpio::Port::B:
        return GPIOB;
    case gpio::Port::C:
        return GPIOC;
    case gpio::Port::D:
        return GPIOD;
    case gpio::Port::E:
        return GPIOE;
    case gpio::Port::F:
        return GPIOF;
    case gpio::Port::G:
        return GPIOG;
    }

    // unreachable
    return nullptr;
}

constexpr uint32_t get_clock_enable_mask( gpio::Port port )
{
    switch ( port ) {
    case gpio::Port::A:
        return RCC_AHB2ENR_GPIOAEN;
    case gpio::Port::B:
        return RCC_AHB2ENR_GPIOBEN;
    case gpio::Port::C:
        return RCC_AHB2ENR_GPIOCEN;
    case gpio::Port::D:
        return RCC_AHB2ENR_GPIODEN;
    case gpio::Port::E:
        return RCC_AHB2ENR_GPIOEEN;
    case gpio::Port::F:
        return RCC_AHB2ENR_GPIOFEN;
    case gpio::Port::G:
        return RCC_AHB2ENR_GPIOGEN;
    }

    // unreachable
    return 0;
}

GPIO::GPIO( gpio::Settings settings )
{
    using namespace gpio;

    port = get_port( settings.port );
    pin  = settings.pin;

    reset_registers();

    uint32_t clock_enable = get_clock_enable_mask( settings.port );
    RCC->AHB2ENR |= clock_enable;

    // zero out MODER
    port->MODER &= ~( 0x3 << ( pin * 2 ) );
    // set MODER
    switch ( settings.mode ) {
    case Mode::Input:
        port->MODER |= 0x0U << ( pin * 2 );
        break;
    case Mode::Output:
        port->MODER |= 0x1U << ( pin * 2 );
        break;
    case Mode::AlternateFunction:
        port->MODER |= 0x2U << ( pin * 2 );
        break;
    case Mode::Analog:
        port->MODER |= 0x3U << ( pin * 2 );
        break;
    }

    // zero out OTYPER
    port->OTYPER &= ~( 0x1U << pin );
    // set OTYPER
    switch ( settings.outputType ) {
    case OutputType::PushPull:
        port->OTYPER |= 0x0U << pin;
        break;
    case OutputType::OpenDrain:
        port->OTYPER |= 0x1U << pin;
        break;
    }

    // zero out OSPEEDR
    port->OSPEEDR &= ~( 0x0U << ( pin * 2 ) );
    // set OSPEEDR
    switch ( settings.speed ) {
    case Speed::Low:
        port->OSPEEDR |= 0x0U << ( pin * 2 );
        break;
    case Speed::Medium:
        port->OSPEEDR |= 0x1U << ( pin * 2 );
        break;
    case Speed::High:
        port->OSPEEDR |= 0x2U << ( pin * 2 );
        break;
    case Speed::VeryHigh:
        port->OSPEEDR |= 0x3U << ( pin * 2 );
        break;
    }

    // zero out PUPDR
    port->PUPDR &= ~( 0x1U << ( pin * 2 ) );
    // set PUPDR
    switch ( settings.pull ) {
    case Pull::None:
        port->PUPDR |= 0x0U << ( pin * 2 );
        break;
    case Pull::Up:
        port->PUPDR |= 0x1U << ( pin * 2 );
        break;
    case Pull::Down:
        port->PUPDR |= 0x2U << ( pin * 2 );
        break;
    }
}

void GPIO::reset_registers()
{
    port->MODER &= ~( GPIO_MODER_MODE0 << ( pin * 2 ) );
    port->OTYPER &= ~( GPIO_OTYPER_OT0 << pin );
    port->OSPEEDR &= ~( GPIO_OSPEEDR_OSPEED0 << ( pin * 2 ) );
    port->PUPDR &= ~( GPIO_PUPDR_PUPD0 << ( pin * 2 ) );
}

void GPIO::set()
{
    port->ODR |= 1UL << pin;
}

void GPIO::unset()
{
    port->ODR &= ~( 1UL << pin );
}

void GPIO::toggle()
{
    port->ODR ^= 1UL << pin;
}

void GPIO::write( int val )
{
    if ( val )
        set();
    else
        unset();
}

int GPIO::read()
{
    return port->IDR & ( 1UL << pin );
}
