
/**
 *  \file   atmega128_digiIO.h
 *  \brief  Atmega128 MCU Digital IO ports
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef ATMEGA128_DIGIIO_H
#define ATMEGA128_DIGIIO_H

struct atmega128_digiIO_t
{
  uint8_t in_updated;
  uint8_t out_updated;

  uint8_t in[6];  
  uint8_t out[6];

  uint8_t direction[6];
  uint8_t selection[6];

  /** only for ports 1 & 2 **/
  uint8_t int_enable[2];
  uint8_t int_edge_select[2];
  uint8_t ifg[2];
};

void    atmega128_digiIO_reset             (void);

int8_t  atmega128_digiIO_mcu_read          (uint16_t addr);
void    atmega128_digiIO_mcu_write         (uint16_t addr, int8_t val);

int     atmega128_digiIO_dev_read          (int port_number, uint8_t *val);
void    atmega128_digiIO_dev_write         (int port_number, uint8_t val, uint8_t bitmask);

int     atmega128_digiIO_internal_dev_read (int port_number, uint8_t *val);
void    atmega128_digiIO_internal_dev_write(int port_number, uint8_t val, uint8_t bitmask);

void    atmega128_digiIO_update_done       (void);
int     atmega128_digiIO_chkifg            (void);

#endif
