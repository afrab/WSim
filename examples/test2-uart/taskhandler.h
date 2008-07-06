#ifndef TASKHANDLER_H
#define TASKHANDLER_H

//Bit definitions and function prototypes for the taskhandler
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>

#ifndef _GNU_ASSEMBLER_
void __attribute__((noreturn)) taskhandler(unsigned short taskreginit);   //Taskhandler loop
extern unsigned short taskreg;  //priority/activation bits of tasks
#endif

// Bit masks for taskbit register
// see taskhandler.s for details about tasktable and taskbit usage
// also look at tasklist.h where the task table is defined
#define TASK00_bits 0x8000
#define TASK01_bits 0x4000
#define TASK02_bits 0x2000
#define TASK03_bits 0x1000
#define TASK04_bits 0x0800
#define TASK05_bits 0x0400
#define TASK06_bits 0x0200
#define TASK07_bits 0x0100
#define TASK08_bits 0x0080
#define TASK09_bits 0x0040
#define TASK10_bits 0x0020
#define TASK11_bits 0x0010
#define TASK12_bits 0x0008
#define TASK13_bits 0x0004
#define TASK14_bits 0x0002
#define TASK15_bits 0x0001

#endif //TASKHANDLER_H
