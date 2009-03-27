/**
 *  \file   uart.h
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#ifndef UART_H
#define UART_H

/* ************************************************** */
/* UART                                               */
/* ************************************************** */

typedef int (*uart_cb_t)(uint8_t data);

void uart_init             (void);
void uart_stop             (void);
void uart_register_cb      (uart_cb_t);

int  putchar               (int);
int  getchar               (void);

/* ************************************************** */
/*                                                    */
/* ************************************************** */

#endif
