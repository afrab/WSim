
#ifndef UART_H
#define UART_H

int uart1_init();

void uart1_eint();
void uart1_eint_tx();
void uart1_eint_rx();
void uart1_dint();

int uart1_putchar(int);
int uart1_getchar(int *c);
int uart1_getchar_polling();

#endif
