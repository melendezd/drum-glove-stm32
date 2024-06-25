.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

.global vtable
.global reset_handler
.global default_interrupt_handler

.type reset_handler, %function
reset_handler:
    ldr r0, =_estack
    mov sp, r0

    movs r0, #0 // copy counter
    ldr r1, =_sdata
    ldr r2, =_edata
    ldr r3, =_sidata
    b copy_sidata_loop

    copy_sidata:
        ldr r4, [r3, r0]
        str r4, [r1, r0]
        adds r0, r0, #4

    copy_sidata_loop:
        adds r4, r0, r1
        cmp r4, r2
        bcc copy_sidata

    // fill bss section with zeros
    mov r0, #0
    ldr r1, =_sbss
    ldr r2, =_ebss
    b zero_bss_loop

    zero_bss:
        str r0, [r1]
        adds r1, r1, #4
    zero_bss_loop:
        cmp r1, r2
        bcc zero_bss

    b main
.size reset_handler, .-reset_handler

// .section .text.default_interrupt_handler,"ax",%progbits
// default_interrupt_handler:
//     default_handler_loop:
//         B default_handler_loop
// .size default_interrupt_handler, .-default_interrupt_handler
