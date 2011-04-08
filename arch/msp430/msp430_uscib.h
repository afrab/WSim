
/**
 *  \file   msp430_uscib.h
 *  \brief  MSP430 USCIB definition (based on "msp430_usart.h" SPI MODE)
 *  \author Julien Carpentier
 *  \date   2011
 **/

#ifndef MSP430_USCIB_H
#define MSP430_USCIB_H

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucbxctl0_t {
  uint8_t
    ucckph:1,
    ucckpl:1,    
    ucmsb:1,    
    uc7bit:1,  
    ucmst:1, 
    ucmode:2,   
    ucsync:1;       
};
#else
struct __attribute__ ((packed)) ucbxctl0_t {
  uint8_t
    ucsync:1,
    ucmode:2,
    ucmst:1,
    uc7bit:1,
    ucmsb:1,
    ucckpl:1,
    ucckph:1; 
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucbxctl1_t {
  uint8_t
    ucssel:2,
    unused:5,
    ucswrst:1;
};
#else
struct __attribute__ ((packed)) ucbxctl1_t {
  uint8_t
    ucswrst:1,
    unused:5,
    ucssel:2;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucbxstat_t {
  uint8_t
    uclisten:1,
    ucfce:1,
    ucoe:1,
    unused:4,
    ucbusy:1;
};
#else
struct __attribute__ ((packed)) ucbxstat_t {
  uint8_t
    ucbusy:1,
    unused:4,
    ucoe:1,
    ucfce:1,
    uclisten:1;
};
#endif

  
struct msp430_uscib_t
{
  union {
    struct ucbxctl0_t  b;
    int8_t             s;
  } ucbxctl0;
  union {
    struct ucbxctl1_t  b;
    int8_t             s;
  } ucbxctl1;
  union {
    struct ucbxstat_t  b;
    int8_t             s;
  } ucbxstat;
  
  uint8_t   ucbxbr0;
  uint8_t   ucbxbr1;
  uint8_t   ucbxrxbuf;
  uint8_t   ucbxtxbuf;
  uint32_t  ucbxbr_div;
  
  uint8_t   ucbxrxbuf_full;
  uint8_t   ucbxrx_shift_buf;
  uint8_t   ucbxrx_shift_empty;
  uint8_t   ucbxrx_shift_ready;
  int32_t   ucbxrx_shift_delay;
  uint8_t   ucbxrx_slave_rx_done;

  uint8_t   ucbxtxbuf_full;
  uint8_t   ucbxtx_shift_buf;
  uint8_t   ucbxtx_shift_empty;
  uint8_t   ucbxtx_shift_ready;
  int32_t   ucbxtx_shift_delay;
  int32_t   ucbxtx_full_delay;  /* delay between tx_buff and shifter */
  
};





/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_uscib0)
#define USCIB0_START  0x068
#define USCIB0_END    0x06f

#define UCB0CTL0     0x068
#define UCB0CTL1     0x069
#define UCB0BR0      0x06a
#define UCB0BR1      0x06b
#define UCB0STAT     0x06d
#define UCB0RXBUF    0x06e
#define UCB0TXBUF    0x06f

void   msp430_uscib0_create();
void   msp430_uscib0_reset();
void   msp430_uscib0_update();
int8_t msp430_uscib0_read (uint16_t addr);
void   msp430_uscib0_write(uint16_t addr, int8_t val);
int    msp430_uscib0_chkifg();


int    msp430_uscib0_dev_read_spi      (uint8_t *val);
void   msp430_uscib0_dev_write_spi     (uint8_t val);
int    msp430_uscib0_dev_write_spi_ok  ();


#else
#define msp430_uscib0_create() do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


