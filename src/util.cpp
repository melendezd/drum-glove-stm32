#include "util.hpp"

void spin( volatile uint32_t cycle_count )
{
    for ( volatile uint32_t i = 0; i < cycle_count; i++ )
        ;
}

volatile uint16_t &cast16( volatile uint32_t &lvalue )
{
    return ( volatile uint16_t & ) lvalue;
}
