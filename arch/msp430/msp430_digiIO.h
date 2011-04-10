
/**
 *  \file   msp430_digiIO.h
 *  \brief  MSP430 Digital IO ports definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_DIGIIO_H
#define MSP430_DIGIIO_H


#define DIGIIO_START 0x0018
#define DIGIIO_END   0x0037

#define P3IN      0x0018
#define P3OUT     0x0019
#define P3DIR     0x001a
#define P3SEL     0x001b

#define P4IN      0x001c
#define P4OUT     0x001d
#define P4DIR     0x001e
#define P4SEL     0x001f

#define P1IN      0x0020 
#define P1OUT     0x0021
#define P1DIR     0x0022
#define P1IFG     0x0023
#define P1IES     0x0024
#define P1IE      0x0025
#define P1SEL     0x0026

// 0x027 undefined

#define P2IN      0x0028 
#define P2OUT     0x0029
#define P2DIR     0x002a
#define P2IFG     0x002b
#define P2IES     0x002c
#define P2IE      0x002d
#define P2SEL     0x002e

// 0x02f undefined

#define P5IN      0x0030
#define P5OUT     0x0031
#define P5DIR     0x0032
#define P5SEL     0x0033

#define P6IN      0x0034
#define P6OUT     0x0035
#define P6DIR     0x0036
#define P6SEL     0x0037

/** 
 * direction : port switch
 * 8bit register
 * PxDIR : 0 = input direction, 1 = output direction
 */
#define DIGIIO_DIR(p)    MCU.digiIO.direction[p]

/** 
 * selection : IO or device 
 * 8bit register
 * PxSEL : 0 = I/O function, 1 = Peripheral module
 */
#define DIGIIO_SEL(p)    MCU.digiIO.selection[p]


/**
 * Digital IO port 
 **/

#define PORT1     0
#define PORT2     1
#define PORT3     2
#define PORT4     3
#define PORT5     4
#define PORT6     5

struct msp430_digiIO_t
{
  uint8_t in_updated[6];
  uint8_t out_updated[6];

  uint8_t in[6];  
  uint8_t out[6];

  uint8_t direction[6];
  uint8_t selection[6];

  /** only for ports 1 & 2 **/
  uint8_t int_enable[2];
  uint8_t int_edge_select[2];
  uint8_t ifg[2];
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
