#ifndef TIMER_H
#define TIMER_H

void timerA3_keep_active(void);
void timerA3_ACLK_start_Hz(int Hz); 

void timerA3_ACLK_start_2S(); /* IRQ every 2S */
void timerA3_ACLK_start_4S(); /* IRQ every 4S */
void timerA3_ACLK_start_8S(); /* IRQ every 8S */

void timerA3_stop();
typedef void (*timerA3cb)(void);
void timerA3_register_callback(timerA3cb f);


void timerB7_SMCLK_start_Hz(int Hz); 
void timerB7_stop();
typedef void (*timerB7cb)(void);
void timerB7_register_callback(timerB7cb f);

#endif
