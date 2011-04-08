
/**
 *  \file   msp430_usart.h
 *  \brief  MSP430 USART definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_USART_H
#define MSP430_USART_H

#if defined(__msp430_have_usart0) || defined(__msp430_have_usart1)


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) uxctl_t {
  uint8_t
    pena:1,   // parity enable 0:disabled 1:enable
    pev:1,    // parity select 0:odd      1:even
    spb:1,    // uart mode : 0 = 1 stop bit, 1 = 2 stop bits
              // spi mode  : 0 = SPI, 1 = i2c
    charb:1,  // length : 0:7bits, 1:8bits. charb instead of char
    listen:1, // listen enable
    sync:1,   // usart / uart
    mm:1,     // multiprocessor mode select (0 idle line, 1 address bit)
    swrst:1;  // reset
};
#else
struct __attribute__ ((packed)) uxctl_t {
  uint8_t
    swrst:1,
    mm:1,
    sync:1,
    listen:1,
    charb:1,  // charb instead of char
    spb:1,    // uart mode : 0 = Odd parity, 1 = even parity
              // spi mode : 0 = SPI, 1 = i2c
    pev:1,
    pena:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) uxctl_spi_t {
  uint8_t
    unused:2,
    i2c:1,
    charb:1,  // charb instead of char
    listen:1,
    sync:1,
    mm:1,
    swrst:1;
};
#else
struct __attribute__ ((packed)) uxctl_spi_t {
  uint8_t
    swrst:1,
    mm:1,
    sync:1,
    listen:1,
    charb:1,  // charb instead of char
    i2c:1,
    unused:2;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) uxtctl_t {
  uint8_t
    ckph:1,   // spi mode
    ckpl:1,
    ssel:2,
    urxse:1,
    txwake:1,
    stc:1,    // spi mode 
    txept:1;
};
#else
struct __attribute__ ((packed)) uxtctl_t {
  uint8_t
    txept:1,
    stc:1,    // spi mode
    txwake:1,
    urxse:1,
    ssel:2,
    ckpl:1,
    ckph:1;   // spi mode
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) uxtctl_spi_t {
  uint8_t
    ckph:1,
    cpkl:1,
    ssel:2,
    unused1:1,
    unused2:1,
    stc:1,
    txept:1;
};
#else
struct __attribute__ ((packed)) uxtctl_spi_t {
  uint8_t
    txept:1,
    stc:1,
    unused2:1,
    unused1:1,
    ssel:2,
    cpkl:1,
    ckph:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) uxrctl_t {
  uint8_t
    fe:1,
    pe:1,
    oe:1,
    brk:1,
    urxeie:1,
    urxwie:1,
    rxwake:1,
    rxerr:1;
};
#else
struct __attribute__ ((packed)) uxrctl_t {
  uint8_t
    rxerr:1,
    rxwake:1,
    urxwie:1,
    urxeie:1,
    brk:1,
    oe:1,
    pe:1,
    fe:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) uxrctl_spi_t {
  uint8_t
    fe:1,
    undefined:1,
    oe:1,
    unused1:1,
    unused2:1,
    unused3:1,
    unused4:1,
    unused5:1;
};
#else
struct __attribute__ ((packed)) uxrctl_spi_t {
  uint8_t
    unused5:1,
    unused4:1,
    unused3:1,
    unused2:1,
    unused1:1,
    oe:1,
    undefined:1,
    fe:1;
};
#endif


enum usart_mode_t {
  USART_MODE_UART =  0,
  USART_MODE_SPI  =  1,
  USART_MODE_I2C  =  2
};

enum usart_char_type_t {
  usart_data,
  usart_address
};

/**
 * USART Data Structure
 **/
struct msp430_usart_t
{
  union {
    struct uxctl_t      b;
    struct uxctl_spi_t  bspi;
    uint8_t             s;
  } uxctl;
  union {
    struct uxtctl_t     b;
    struct uxtctl_spi_t bspi;
    uint8_t             s;
  } uxtctl;
  union {
    struct uxrctl_t     b;
    struct uxrctl_spi_t bspi;
    uint8_t             s;
  } uxrctl;

  uint8_t   uxmctl;
  uint8_t   uxbr0;
  uint8_t   uxbr1;
  uint32_t  uxbr_div;

  uint8_t   uxrxbuf;
  uint8_t   uxrxbuf_full;
  uint8_t   uxrx_shift_buf;
  uint8_t   uxrx_shift_empty;
  uint8_t   uxrx_shift_ready;
  int32_t   uxrx_shift_delay;
  uint8_t   uxrx_slave_rx_done;

  uint8_t   uxtxbuf;
  uint8_t   uxtxbuf_full;
  uint8_t   uxtx_shift_buf;
  uint8_t   uxtx_shift_empty;
  uint8_t   uxtx_shift_ready;
  int32_t   uxtx_shift_delay;
  int32_t   uxtx_full_delay;  /* delay between tx_buff and shifter */

  enum usart_mode_t     mode;

  /* onlu used in UART mode */
  enum usart_char_type_t  curr_tx_type;
  enum usart_char_type_t  next_tx_type;
  enum usart_char_type_t  curr_rx_type;

};

#endif // uart0 || usart1

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_usart0)
#define USART0_START 0x0070
#define USART0_END   0x0077

#define U0CTL        0x0070
#define U0TCTL       0x0071
#define U0RCTL       0x0072
#define U0MCTL       0x0073
#define U0BR0        0x0074
#define U0BR1        0x0075
#define U0RXBUF      0x0076
#define U0TXBUF      0x0077

#define I2CCIE       0x0050
#define I2CCFG       0x0051
#define I2CNDAT      0x0052
#define I2COA        0x0118 //short
#define I2CSA        0x011A //short
#define I2CIV        0x011C

void   msp430_usart0_create();
void   msp430_usart0_reset();
void   msp430_usart0_update();
int8_t msp430_usart0_read (uint16_t addr);
void   msp430_usart0_write(uint16_t addr, int8_t val);
int    msp430_usart0_chkifg();

int    msp430_usart0_dev_read_spi      (uint8_t *val);
void   msp430_usart0_dev_write_spi     (uint8_t val);
int    msp430_usart0_dev_write_spi_ok  ();

int    msp430_usart0_dev_read_uart     (uint8_t *val);
void   msp430_usart0_dev_write_uart    (uint8_t val);
int    msp430_usart0_dev_write_uart_ok ();
#else
#define msp430_usart0_create() do { } while (0)
#define msp430_usart0_reset()  do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_usart1)
#define USART1_START 0x0078
#define USART1_END   0x007f

#define U1CTL        0x0078
#define U1TCTL       0x0079
#define U1RCTL       0x007a
#define U1MCTL       0x007b
#define U1BR0        0x007c
#define U1BR1        0x007d
#define U1RXBUF      0x007e
#define U1TXBUF      0x007f

void   msp430_usart1_create();
void   msp430_usart1_reset();
void   msp430_usart1_update();
int8_t msp430_usart1_read (uint16_t addr);
void   msp430_usart1_write(uint16_t addr, int8_t val);
int    msp430_usart1_chkifg();


int    msp430_usart1_dev_read_spi      (uint8_t *val);
void   msp430_usart1_dev_write_spi     (uint8_t val);
int    msp430_usart1_dev_write_spi_ok  ();

int    msp430_usart1_dev_read_uart     (uint8_t *val);
void   msp430_usart1_dev_write_uart    (uint8_t val);
int    msp430_usart1_dev_write_uart_ok ();
#else
#define msp430_usart1_create() do { } while (0)
#define msp430_usart1_reset()  do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
