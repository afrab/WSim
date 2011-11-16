#include "hardware.h"
#include "taskhandler.h"
#include "tasklist.h"

/**
Simple Taskhandler

Each task is a function and has a corresponding bit in the variable
taskreg. TASK00 has bit 0x8000 which is the highest priority,
TASK01~0x4000 etc. Up to 16 tasks are supported.
Each task has to exit by itself (there is no task switching). This
causes a risk of priority inversion when a low priority task consumes
much time.

Each time a task exits, the taskbits are scanned from left to right,
starting at the higest prio. If no taskbit is set, a lowpower mode is
entered. Scanning for tasks is restarted when the CPU is woken up
again (external event, timer interrupt etc.).

A task does not need to save any registers as on each run the
taskhandler is initilizing its registers.

http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/

.extern TASK00
.extern TASK01
.extern TASK02
.extern TASK03
.extern TASK04
.extern TASK05
.extern TASK06
.extern TASK07
.extern TASK08
.extern TASK09
.extern TASK10
.extern TASK11
.extern TASK12
.extern TASK13
.extern TASK14
.extern TASK15

;variables
.data
        .global taskreg
        .comm   taskreg,2,2                     ;short var, aligned. taskbits are stored here

.text

/**
Taskhandler mainloop. This function never returns.
*/
.global taskhandler
        .type taskhandler, @function
taskhandler:
        mov   r15, taskreg                      ; save parameter to taskreg

.Ltsk1: mov   #0x8000, r15                      ; bitmask, used to scan task bits
        mov   #tasktable, r14                   ; pointer to task function

.Ltsk2: bit   r15, taskreg                      ; test mask on task bits
        jc    .calltask                         ; if bit is set -> call task function
        rrc   r15                               ; shift mask. carry is zero
        incd  r14                               ; increment pointer on task function
        cmp   #endtasktable, r14                ; check if bits for entire table have been tested
        jlo   .Ltsk2                            ; #endtasktable < R5 --> it was not last entry, loop

.golowpower:                                    ; if it was last entry...
        bic.b #LEDRT, &P2OUT                    ; Activity indicator
        bis   #CPUOFF, r2                         ; go in lowpower mode until wakeup trough interrupt
        bis.b #LEDRT, &P2OUT                    ; Activity indicator
        jmp   .Ltsk1                            ; restart searching on wakeup

.calltask:
        bic   r15, taskreg                      ; clear taskbit we're handling here
        call  @r14                              ; call task itself, no need to save registers
        jmp   .Ltsk1                            ; loop


/**
Table with task functions. User tasks are set up using macro definitions ("#define")
in "tasklist.h".

Task functions are "void f(void)". Any registers can be used and have not to be saved.
(actualy the functions can be void f(unsigned short taskbit) with taskbit beeing
the bit that is associated with that task. this can be used if a task wants to
restart itself: "taskreg |= taskbit;")
*/
tasktable:
.word TASK00    // highest prio
.word TASK01
.word TASK02
.word TASK03
.word TASK04
.word TASK05
.word TASK06
.word TASK07
.word TASK08
.word TASK09
.word TASK10
.word TASK11
.word TASK12
.word TASK13
.word TASK14
.word TASK15    // lowest prio
endtasktable:

