/*
 * Copyright  2008-2009 INRIA/SensTools
 * 
 * <dev-team@sentools.info>
 * 
 * This software is a set of libraries designed to develop applications
 * for the WSN430 embedded hardware platform.
 * 
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */

/**
 *  \file   clock.c
 *  \brief  msp430 system clock 
 *  \author Antoine Fraboulet
 *  \date   2005
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
 *
 ***************************************************************/

void set_mcu_speed_dco_mclk_4MHz_smclk_1MHz(void)
{
  /*
   * ACLK  = ??
   * MCLK  = dcoclk @ 4.16MHz
   * SMCLK = dcoclk / 2
   */
  
  // turn on XT1 
  
  //start up crystall oscillator XT2
  // DIVA_0 -> ACLK divider = 1
  // RSEL2 | RSEL1 | RSEL0 -> resistor select
  BCSCTL1 = XT2OFF | RSEL2 | RSEL1 | RSEL0;
  
  // SELM_0 = dcoclk
  // SELM_1 = dcoclk
  // SELM_2 = XT2CLK/LFXTCLK
  // SELM_3 = LFXTCLK
  // SELS   : SMCLK source XT2
  // DIVS_1 : SMCLK divider /2 
  BCSCTL2 = SELM_0 | DIVS_2;
  
  // dcox = 6, rsel = 7, mod = 0
  // according to "Bob L'éponge" the MCLK clock speed should
  // be set to 4.16MHz and SMCLK clock speed to 2.08Mhz
  DCOCTL  = DCO2 | DCO1;
  
  WAIT_CRISTAL();
}

/***************************************************************
 *
 ***************************************************************/

void set_mcu_speed_xt2_mclk_2MHz_smclk_1MHz(void)
{
  DCOCTL   = 0; /* dco */
  BCSCTL1  = 0; /* xt2 on + xts high + aclk full speed */
  BCSCTL2  = (SELM_2 | DIVM_2) | (SELS | DIVS_3); /* xt2/4, xt2/8 */

  WAIT_CRISTAL();
} 

/***************************************************************
 *
 ***************************************************************/

void set_mcu_speed_xt2_mclk_4MHz_smclk_1MHz(void)
{
  DCOCTL   = 0;
  BCSCTL1  = 0;
  BCSCTL2  = (SELM_2 | DIVM_1) | (SELS | DIVS_3);

  WAIT_CRISTAL();
} 

void set_aclk_div(uint16_t div)
{
  int f=0;
  switch (div)
    {
    case 1: f = DIVA_0; break;
    case 2: f = DIVA_1; break;
    case 4: f = DIVA_2; break;
    case 8: f = DIVA_3; break;
    default: f = DIVA_0; break;
    }
  BCSCTL1  &= ~(DIVA_3);
  BCSCTL1  |= f;
}

/***************************************************************
 *
 ***************************************************************/

void set_mcu_speed_xt2_mclk_8MHz_smclk_8MHz(void)
{
  DCOCTL  = 0;
  BCSCTL1 = 0;
  BCSCTL2 = SELM_2 | SELS;

  WAIT_CRISTAL();
}

/***************************************************************
 *
 ***************************************************************/

void set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz(void)
{
  DCOCTL  = 0;
  BCSCTL1 = 0;
  BCSCTL2 = SELM_2 | (SELS | DIVS_3) ;

  WAIT_CRISTAL();
}
