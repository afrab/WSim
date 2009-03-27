/**
 *  \file   spi.h
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#ifndef SPI_H
#define SPI_H

#include <stdint.h>

/* ************************************************** */
/* SPI                                                */
/* ************************************************** */

#define SPI_DUMMY_BYTE 0x55

void    spi_init               (void);

int     spi_tx_rx              (int x);
void    spi_tx_burst           (char*,int);
void    spi_rx_burst           (char*,int);
int     spi_check_miso_high    (void); 

#define spi_rx()               spi_tx_rx(SPI_DUMMY_BYTE)

void    spidev_select          (void);
void    spidev_deselect        (void);

/* ************************************************** */
/*                                                    */
/* ************************************************** */

#endif
