/**
Assembler functions.

http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/

#include "hardware.h"
#include "tasklist.h"

.extern procchar
.extern taskreg

/**
demux timer A interrupts
*/
interrupt(TIMERA1_VECTOR)
tax_int:
        add     &TAIV, r0               ; Add TA interrupt offset to PC
        reti                            ; CCR0 - no source
        jmp     ccr1                    ; CCR1
        jmp     ccr2                    ; CCR2
        reti                            ; CCR3 - no source
        reti                            ; CCR4 - no source
taover:                                 ; TAOVER (follows directly)
        reti
ccr1:   reti    ;br      #INT_TimerA_CCR1 ;redirect to an interrupt function
ccr2:   reti    ;br      #INT_TimerA_CCR2 ;redirect to an interrupt function


/**
UART receive interrupts on character
*/
interrupt(UART0RX_VECTOR)
uart0RX:
        push    r12                     ; registers have to be saved, as a C func is called
        push    r13
        push    r14
        push    r15
        call    #procchar
        tst     r15                     ;test return value
        bis     r15, taskreg            ;activate task(s) if nonzero
        bic     #CPUOFF+OSCOFF+SCG0+SCG1, 8(r1) ;set CPU to active mode (4*push->8)
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        reti

/**
send a single character over the UART, first wait until transmit buffer is empty
*/
.global putchar
        .type putchar, @function
putchar:                                ;send a byte
.L1:    bit.b   #TXEPT, &UTCTL0         ;tx buffer empty?
        jnc     .L1                     ;no -> loop until it is
        mov.b   r15, &TXBUF0
        mov     #1, r15
        ret
