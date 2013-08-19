
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

#if defined(__msp430_have_new_uscib)

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucbxie_t {
  uint8_t
    reserved:6,
    uctxie:1,
    ucrxie:1;
};
#else
struct __attribute__ ((packed)) ucbxie_t {
  uint8_t
    ucrxie:1,
    uctxie:1,
    reserved:6;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucbxifg_t {
  uint8_t
    reserved:6,
    uctxifg:1,
    ucrxifg:1;
};
#else
struct __attribute__ ((packed)) ucbxifg_t {
  uint8_t
    ucrxifg:1,
    uctxifg:1,
    reserved:6;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ucbxiv_t {
  uint16_t
    reserved:13,
    ucivx:2,
    reserved1:1;
};
#else
struct __attribute__ ((packed)) ucbxiv_t {
  uint16_t
    reserved1:1,
    ucivx:2,
    reserved:13;
};
#endif

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
#if defined(__msp430_have_new_uscib)
  union {
    struct ucbxie_t  b;
    int8_t             s;
  } ucbxie;
  union {
    struct ucbxifg_t  b;
    int8_t             s;
  } ucbxifg;
  union {
    struct ucbxiv_t  b;
    int16_t             s;
  } ucbxiv;
#endif
  
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
#if defined(__msp430_have_new_uscib)

#define USCIB0_START  USCIB_BASE
#define USCIB0_END    USCIB_BASE + 0x1e

#define UCB0CTL0     USCIB0_START + 0x01
#define UCB0CTL1     USCIB0_START + 0x00
#define UCB0BR0      USCIB0_START + 0x06
#define UCB0BR1      USCIB0_START + 0x07
#define UCB0STAT     USCIB0_START + 0x0a
#define UCB0RXBUF    USCIB0_START + 0x0c
#define UCB0TXBUF    USCIB0_START + 0x0e
#define UCB0ICTL     USCIB0_START + 0x1c
#define UCB0IE       USCIB0_START + 0x1c
#define UCB0IFG      USCIB0_START + 0x1d
#define UCB0IV       USCIB0_START + 0x1e

#else

#define USCIB0_START  USCIB_BASE
#define USCIB0_END    USCIB_BASE+7
 
#define UCB0CTL0     USCIB_BASE
#define UCB0CTL1     USCIB_BASE+1
#define UCB0BR0      USCIB_BASE+2
#define UCB0BR1      USCIB_BASE+3
#define UCB0STAT     USCIB_BASE+5
#define UCB0RXBUF    USCIB_BASE+6
#define UCB0TXBUF    USCIB_BASE+7

#endif

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
#define msp430_uscib0_reset()  do { } while (0)
#define msp430_uscib0_update() do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


