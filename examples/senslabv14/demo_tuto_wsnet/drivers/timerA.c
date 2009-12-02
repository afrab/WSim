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
 *  \file   timerA.c
 *  \brief  MSP430 timerA driver
 *  \author Clément Burin des Roziers
 *  \date   November 08
 **/

#include <io.h>
#include <signal.h>
#include "timerA.h"

static timerAcb timerA_callbacks[TIMERA_CCR_NUMBER+1];
static uint16_t timerA_periods[TIMERA_CCR_NUMBER];
static uint16_t *TACCTLx = (uint16_t*) 0x162;
static uint16_t *TACCRx  = (uint16_t*) 0x172;

void timerA_init()
{
    uint16_t i;
    
    // stop everything
    TACTL = 0;
    
    // clear the CCR and CCTL registers, and the associated callbacks
    for (i=0;i<TIMERA_CCR_NUMBER;i++)
    {
        TACCTLx[i] = 0;
        TACCRx[i] = 0;
        timerA_callbacks[i] = 0x0;
        timerA_periods[i] = 0;
    }
    // clear the overflow callback
    timerA_callbacks[TIMERA_CCR_NUMBER+1] = 0x0;
}

uint16_t timerA_start_ACLK_div (uint16_t s_div)
{
    // check if divider is correct
    if (s_div > 3)
    {
        return 0;
    }
    
    // update configuration register
    TACTL = (TASSEL_1) | MC_2 | (s_div<<6);
    
    return 1;
}

uint16_t timerA_register_cb (uint16_t alarm, timerAcb f)
{
    if (alarm >= TIMERA_CCR_NUMBER+1)
    {
        return 0;
    }
    
    timerA_callbacks[alarm] = f;
    
    if (alarm == TIMERA_ALARM_OVER)
    {
        // if callback is NULL, disable overflow interrupt
        if (f == 0x0)
        {
            TACTL &= ~(TAIE);
        }
        // if not NULL, enable OF interrupt
        else
        {
            TACTL |= TAIE;
        }
    }
    return 1;
}

uint16_t timerA_time()
{
    return TAR;
}

uint16_t timerA_set_alarm_from_now  (uint16_t alarm, uint16_t ticks, uint16_t period)
{
    uint16_t now;
    now = TAR;
    
    if (alarm > TIMERA_CCR_NUMBER)
    {
        return 0;
    }
    
    TACCRx[alarm] = now + ticks;
    TACCTLx[alarm] = CCIE;
    timerA_periods[alarm] = period;
    
    return 1;
}

uint16_t timerA_set_alarm_from_time (uint16_t alarm, uint16_t ticks, uint16_t period, uint16_t ref)
{    
    if (alarm > TIMERA_CCR_NUMBER)
    {
        return 0;
    }
    
    TACCRx[alarm] = ref + ticks;
    TACCTLx[alarm] = CCIE;
    timerA_periods[alarm] = period;
    
    return 1;
}

uint16_t timerA_unset_alarm(uint16_t alarm)
{
    if (alarm > TIMERA_CCR_NUMBER)
    {
        return 0;
    }
    
    TACCRx[alarm] = 0;
    TACCTLx[alarm] = 0;
    timerA_periods[alarm] = 0;
    timerA_callbacks[alarm] = 0;
    
    return 1;
}

void timerA_stop()
{
    // stop mode
    TACTL &= ~(MC0|MC1);
}

interrupt (TIMERA0_VECTOR) timerA0irq( void )
{
    if (timerA_periods[0])
    {
        TACCRx[0] += timerA_periods[0];
    }
    else
    {
        TACCRx[0] = 0;
        TACCTLx[0] = 0;
    }
    
    if (timerA_callbacks[0])
    {
        if ( timerA_callbacks[0]() )
        {
            LPM4_EXIT;
        }
    }
}

interrupt (TIMERA1_VECTOR) timerA1irq( void )
{
    uint16_t alarm;
    
    alarm = TAIV >> 1;
    
    // if overflow, just call the callback
    if (alarm == 0x05)
    {
        if (timerA_callbacks[TIMERA_ALARM_OVER])
        {
            if ( timerA_callbacks[TIMERA_ALARM_OVER]() )
            {
                LPM4_EXIT;
            }
        }
    }
    else
    {
        if (timerA_periods[alarm])
        {
            TACCRx[alarm] += timerA_periods[alarm];
        }
        else
        {
            TACCRx[alarm] = 0;
            TACCTLx[alarm] = 0;
        }
        
        if (timerA_callbacks[alarm])
        {
            if ( timerA_callbacks[alarm]() )
            {
                LPM4_EXIT;
            }
        }
    }
}
