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
 *  \file   uart0.h
 *  \brief  MSP430 uart0 driver header
 *  \author Antoine Fraboulet
 *  \author Colin Chaballier
 *  \author Clément Burin des Roziers <clement.burin-des-roziers@inria.fr>
 *  \date   2008
 **/


#ifndef _UART0_H_
#define _UART0_H_

typedef uint16_t (*uart0_cb_t)(uint8_t c);

#define UART0_CONFIG_8MHZ_115200 0
#define UART0_CONFIG_1MHZ_38400  1
#define UART0_CONFIG_1MHZ_115200 2

/**
 * Configure the USART0 for UART use.
 * \param config configuration to use depending
 * on SMCLK speed and desired baudrate
 */
void uart0_init(uint16_t config);

/**
 * Wait until a character is read and return it.
 * \return the character read
 */
int uart0_getchar_polling(void);

/**
 * Send a character.
 * \param c the character to send
 * \return the same sent character
 */
int uart0_putchar(int c);

/**
 * Stop the peripheral.
 */
void uart0_stop(void);

/**
 * Register a callback that will be called every time
 * a char is received.
 * \param the callback function pointer
 */
void uart0_register_callback( uart0_cb_t cb);

#endif

