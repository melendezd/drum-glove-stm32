#include "dac.hpp"

#define STM32G431xx
#include "stm32g431xx.h"
#include "stm32g4xx.h"

DacController::DacController( dac::Settings settings ) : settings(settings), dac( DAC )
{
    uint32_t cr = 0;
    uint32_t cr_en_msk = DAC_CR_EN1;
    uint32_t cr_ten_msk = 0;

    SET_BIT(cr, settings.channel == 1);
        
    CLEAR_BIT( dac->CR, 0x1 << ( 16 * ( settings.channel - 1 ) ) );
    //SET_BIT(dac->CR, apply_channel(cr_en_msk, 
}

constexpr uint32_t DacController::apply_channel(uint32_t mask)
{
    if (settings.channel != 0 && settings.channel != 1) return 0;
    return mask << (settings.channel - 1) * 16;
}
