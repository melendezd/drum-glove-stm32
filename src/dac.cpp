#include "dac.hpp"

#include "mcu.hpp"
#include "stm32g431xx.h"
#include "util.hpp"
#include "global_constants.hpp"

DacController::DacController( dac::Settings settings ) : dac( DAC ), channel(settings.channel), indicator(settings.indicator)
{
    enable_clock();

    // ENx = 0: disable DAC for now
    // TENx = 0: trigger disabled, data is shifted one dac_hclk cycle after it is written
    // TSELx = 0: if trigger enabled, trigger is writing to SWTRIGx (software trigger)
    uint32_t cr = 0;

    MODIFY_REG(dac->CR, 0xFFFFFFFFU, cr);

    // We ensure the DAC_MCR configuration register is at its reset value 0x00000000. Hence:
    // - Input data is in unsigned format
    // - Channel is connected to external pin with buffer enabled
    CLEAR_REG(dac->MCR);

    switch(channel) {
        case dac::Channel::One:
            data_register = &dac->DHR8R1;
            break;
        case dac::Channel::Two:
            data_register = &dac->DHR8R2;
            break;
    }
        
    enable();
}

constexpr uint32_t DacController::apply_channel(uint32_t config)
{
    int channel_number = 0;
    switch(channel) {
        case dac::Channel::One:
            channel_number = 0;
            break;
        case dac::Channel::Two:
            channel_number = 1;
            break;
    }
    return config << channel_number * 16;
}

bool DacController::is_ready()
{
    return READ_BIT(dac->SR, apply_channel(DAC_SR_DAC1RDY)) != 0;
}

void DacController::enable()
{
    SET_BIT(dac->CR, apply_channel(DAC_CR_EN1));
}

bool DacController::write_if_ready(uint32_t data)
{
    if (is_ready()) {
        write(data);
        return true;
    } else return false;
}

void DacController::write(uint32_t data)
{
    *data_register = data;
}

void DacController::enable_clock()
{
    // enable DAC clock
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN);
}
