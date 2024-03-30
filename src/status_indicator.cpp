#include "status_indicator.hpp"

StatusIndicator::StatusIndicator( GPIO &out, DelayTimer &delay ) : out( out ), delay( delay ) { }

void StatusIndicator::status_forever(uint32_t status_code)
{
    while(1) {
        status_once(status_code);
    }
}

void StatusIndicator::status_once(uint32_t status_code)
{
    const uint16_t long_ms = 600;
    const uint16_t short_ms = 175;

    out.set();
    delay.ms(long_ms);
    out.unset();

    delay.ms(long_ms);
    
    for (int i = 0; i < status_code; i++) {
        out.set();
        delay.ms(short_ms);
        out.unset();
        delay.ms(short_ms);
    }

    delay.ms(long_ms);
}
