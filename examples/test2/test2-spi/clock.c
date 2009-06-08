/**
 *  \file   clock.c
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "clock.h"

/***************************************************************
 * we have to wait OFIFG to be sure the switch is ok 
 * slau049e.pdf page 4-12 [pdf page 124]
 ***************************************************************/

#define WAIT_CRISTAL()                                                 \
do {                                                                   \
  int i;                                                               \
  do {                                                                 \
    IFG1 &= ~OFIFG;                  /* Clear OSCFault flag  */        \
    for (i = 0xff; i > 0; i--)       /* Time for flag to set */        \
       nop();                        /*                      */        \
  }  while ((IFG1 & OFIFG) != 0);    /* OSCFault flag still set? */    \
} while (0)


/***************************************************************
 * XT2
 ***************************************************************/

void set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz()
{
  DCOCTL  = 0;
  BCSCTL1 = 0;
  BCSCTL2 = SELM_2 | (SELS | DIVS_3) ;
  WAIT_CRISTAL();
}


/***************************************************************
 *
 ***************************************************************/
