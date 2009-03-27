/**
 *  \file   timer.h
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#ifndef MSP430_TIMER_H
#define MSP430_TIMER_H

/* ************************************************** */
/*                                                    */
/* ************************************************** */

typedef void (*timer_cb)(void);

/* timer A is set on SMCLK /8 */
void timerA_init        (void);
void timerA_register_cb (timer_cb);
void timerA_set_wakeup  (int);
void timerA_start       (uint16_t ticks);
void timerA_stop        (void);

/* ************************************************** */
/*                                                    */
/* ************************************************** */

#endif
