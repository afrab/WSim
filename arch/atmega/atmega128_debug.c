
/**
 *  \file   atmega128_debug.c
 *  \brief  Atmega128 MCU emulation debug
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "atmega128.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(DISASSEMBLE)
void atmega128_print_SR(void)
{
  HW_DMSG_DIS("ITHSVNZC\n");
  debug_write_binary(SR,8);
  HW_DMSG_DIS("\n");
}
#else
void atmega128_print_SR(void)
{
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* mcu_regname_str(unsigned r)
{
  switch (r)
    {
    case  0: return "r0";
    case  1: return "r1";
    case  2: return "r2"; /* cg1 */
    case  3: return "r3"; /* cg2 */
    case  4: return "r4";
    case  5: return "r5";
    case  6: return "r6";
    case  7: return "r7";
    case  8: return "r8";
    case  9: return "r9";
    case 10: return "r10";
    case 11: return "r11";
    case 12: return "r12";
    case 13: return "r13";
    case 14: return "r14";
    case 15: return "r15";
    default: return "xx";
    }
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#ifdef DEBUG_DIGI_IO 
char *gpio_reg_name[] =
    {
    "PINE", "DDRE", "PORTE",
    "PIND", "DDRD", "PORTD",
    "PINC", "DDRC", "PORTC",
    "PINB", "DDRB", "PORTB",
    "PINA", "DDRA", "PORTA",
    "PINF", "DDRF", "PORTF",
    "PING", "DDRG", "PORTG"
};


char* atmega128_debug_portname(uint16_t addr)
{
    uint8_t index = address_to_digiio_IDX(addr);
    
    if (index < 21)
    {
    return gpio_reg_name[index];
    }
    else
    {
        ERROR("Port register address out of range");
        return 0;
    }
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void atmega128_print_registers(void)
{
  int i;
  for(i=0; i < mcu_registers_number(); i++)
    {
      HW_DMSG_FD(" %3s : 0x%04x\n",mcu_regname_str(i),MCU_REGS[i] & 0xffffu);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


