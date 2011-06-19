
/**
 *  \file   msp430_digiIO.h
 *  \brief  MSP430 Digital IO ports definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_DIGIIO_H
#define MSP430_DIGIIO_H

/**
 * Digital IO port 
 **/

#define PORT1     0
#define PORT2     1
#define PORT3     2
#define PORT4     3
#define PORT5     4
#define PORT6     5
#define PORT7     6
#define PORT8     7


struct msp430_digiIO_t
{
  uint8_t in             [8];
  uint8_t in_updated     [8];

  uint8_t out            [8];
  uint8_t out_updated    [8];

  uint8_t direction      [8];
  uint8_t selection      [8];
  uint8_t selection2     [8];
  uint8_t resistor       [8];

  /** only for ports 1 & 2 **/
  uint8_t int_enable     [2];
  uint8_t int_edge_select[2];
  uint8_t ifg            [2];
};


void    msp430_digiIO_create            (void);
void    msp430_digiIO_reset             (void);
#define msp430_digiIO_update()          do { } while (0)

int8_t  msp430_digiIO_mcu_read          (uint16_t addr);
void    msp430_digiIO_mcu_write         (uint16_t addr, int8_t val);

int     msp430_digiIO_dev_read          (int port_number, uint8_t *val);
uint8_t msp430_digiIO_dev_read_dir      (int port_number);
void    msp430_digiIO_dev_write         (int port_number, uint8_t val, uint8_t bitmask);

int     msp430_digiIO_internal_dev_read (int port_number, uint8_t *val);
void    msp430_digiIO_internal_dev_write(int port_number, uint8_t val, uint8_t bitmask);

void    msp430_digiIO_update_done       (void);
int     msp430_digiIO_chkifg            (void);

#endif
