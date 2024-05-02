_estack = 0x20004000;

_min_leftover_ram = 0x400;

MEMORY
{
    FLASH ( rx ) : ORIGIN = 0x08000000, LENGTH = 128K
    RAM ( rxw )  : ORIGIN = 0x20000000, LENGTH = 16K
}

SECTIONS
{
    .vector_table :
    {
        . = ALIGN(4);
        KEEP (*(.vector_table))
        . = ALIGN(4);
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        . = ALIGN(4);
    } >FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    }

    .sample_data :
    {
        . = ALIGN(4);
        KEEP (*(.sample_data))
        . = ALIGN(4);
    } >FLASH

    _sidata = .;
    .data : AT(_sidata)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        _edata = .;
        . = ALIGN(4);
    } >RAM

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM

    .dynamic_allocations :
    {
        . = ALIGN(4);
        _ssystem_ram = .;
        . = . + _min_leftover_ram;
        . = ALIGN(4);
        _esystem_ram = .;
    } >RAM
}
