/*
 *  cc1100.c
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Modified / rewrite by Antoine Fraboulet on 04/04/07
 *  Copyright 2005,2006,2007 __WorldSens__. All rights reserved.
 *
 */

#include "cc1100_globals.h"
#include "cc1100_wsn430.h"
#include "cc1100_spi.h"
#include "cc1100.h"

#include "leds.h"

/**********************
 * Global variables
 **********************/

struct _cc1100_status_bits_t {
  uint8_t chip_ready :1;
  uint8_t state      :3;
  uint8_t fifo_bytes :4;
};


volatile uint8_t cc1100_status_register;

/* 0:normal mode 1:irq mode */
volatile uint8_t cc1100_intr;            

/* Rx irq callback */
typedef void (*cc1100_rx_irq_start_function)(void);
static volatile cc1100_rx_irq_start_function  cc1100_rx_irq_cb;  
 
/* pin configuration for interrupt handler */
volatile uint8_t  cc1100_gdo2_cfg                 = CC1100_GDOx_CHIP_RDY;
volatile uint8_t  cc1100_gdo0_cfg                 = CC1100_GDOx_CLK_XOSC_192;

/* Tx/Rx Fifo threshold */
/* int8_t            cc1100_fixed_packet_length      = -1; */
volatile uint8_t  cc1100_fifo_threshold           = 0x07;

/* State transition after I/O completion */
volatile uint8_t  cc1100_rx_off                   = 0x00;
volatile uint8_t  cc1100_tx_off                   = 0x00;

/* cc1100 I/O state : */
uint8_t           cc1100_freq_config              = 0;
uint8_t const    *cc1100_patable;

/* cc1100 tx variables */
volatile uint8_t  cc1100_tx_ongoing               = 0x00;
volatile uint8_t  cc1100_tx_error                 = 0x00;
uint8_t          *cc1100_tx_packet;
volatile uint8_t  cc1100_tx_offset                = 0x00;
volatile uint8_t  cc1100_tx_length                = 0x00;
volatile uint8_t  cc1100_tx_sent                  = 0x00;

/* cc1100 rx variables */
cc1100_rx_cb_t    cc1100_rx_cb                    = 0x00;
volatile uint8_t  cc1100_rx_ongoing               = 0x00;
// volatile uint8_t  cc1100_rx_received              = 0x00;

uint8_t          *cc1100_rx_packet;
volatile uint8_t  cc1100_rx_offset                = 0x00;
volatile uint8_t  cc1100_rx_length                = 0x00;

/* Power table configuration [1] page 37                                */
/*                     0     1      2     3     4     5    6      7     */
/*                     0    -5    -10   -15   -20   -30   +5    +10 dBm */
#define SCU8 static const uint8_t
SCU8 patable_315[]  = {0x51, 0x69, 0x26, 0x1D, 0x17, 0x04, 0x86, 0xC3 };
SCU8 patable_433[]  = {0x51, 0x3A, 0x06, 0x1C, 0x6C, 0x68, 0xC8, 0xC0 };
SCU8 patable_868[]  = {0x60, 0x67, 0x34, 0x1C, 0x0D, 0x03, 0x85, 0xC3 };
SCU8 patable_915[]  = {0x50, 0x67, 0x6D, 0x1B, 0x0B, 0x11, 0x85, 0xC1 };
static const uint8_t patable_length = 8;
const uint8_t *patable[] = {patable_315, patable_433, patable_868, patable_915}; 


/**********************
 * Macros
 **********************/

#define CC1100_ENTER_INTERRUPT() cc1100_intr = 1
#define CC1100_EXIT_INTERRUPT()  cc1100_intr = 0

/**********************
 * State management
**********************/

#define cc1100_update_status()           CC1100_SPI_STROBE(CC1100_STROBE_SNOP)
#define cc1100_get_state_from_status()   ((cc1100_status_register >> 4) & 0x07)

void cc1100_wait_status(uint8_t state) 
{
  uint8_t s;
  do {
      cc1100_update_status();
      s = cc1100_get_state_from_status();
  } while (s != state);
}


static inline int cc1100_check_tx_underflow(void)
{
  cc1100_update_status();
  return cc1100_get_state_from_status() == CC1100_STATUS_TXFIFO_UNDERFLOW;
}

static inline int cc1100_check_rx_overflow(void)
{
  cc1100_update_status();
  return cc1100_get_state_from_status() == CC1100_STATUS_TXFIFO_UNDERFLOW;
}

int cc1100_check_fifo_xflow_flush(void) 
{
  uint8_t s;
  cc1100_update_status();
  s = cc1100_get_state_from_status();
  if (s == CC1100_STATUS_TXFIFO_UNDERFLOW) 
    {
      cc1100_tx_ongoing = 0;                 /* kill tx */
      CC1100_SPI_STROBE(CC1100_STROBE_SFTX); /* flush   */
      cc1100_wait_status(CC1100_STATUS_IDLE);
      return -1;
    } 
  else if (s == CC1100_STATUS_RXFIFO_OVERFLOW) 
    {
      cc1100_rx_ongoing = 0;                 /* kill rx */
      CC1100_SPI_STROBE(CC1100_STROBE_SFRX); /* flush   */
      cc1100_wait_status(CC1100_STATUS_IDLE);
      return -1;
    }
  return 0;
}


void cc1100_calibrate(void)
{
  cc1100_idle();
  CC1100_SPI_STROBE(CC1100_STROBE_SCAL);
  cc1100_wait_status(CC1100_STATUS_IDLE);
}


void cc1100_set_fifo_threshold(uint8_t thr) 
{
  uint8_t reg;
  reg  = (thr & 0x0F);
  cc1100_fifo_threshold = reg;
  CC1100_SPI_WREG(CC1100_REG_FIFOTHR, reg);
}

/* **************************************************
 * I/O Configuration
 * **************************************************/


void cc1100_gdo0_set_signal(uint8_t signal)
{
  uint8_t reg = signal & 0x3F;
  cc1100_gdo0_cfg = reg;
  CC1100_SPI_WREG(CC1100_REG_IOCFG0, reg);
}


void cc1100_gdo2_set_signal(uint8_t signal) 
{
  uint8_t reg = signal & 0x3F;
  cc1100_gdo2_cfg = reg;
  CC1100_SPI_WREG(CC1100_REG_IOCFG2, reg);
}


/* **************************************************
 * CC1100 state machine automatic transition
 * configuration 
 * **************************************************/

/* auto-calibration when switching to/from IDLE/TX/RX
 *
 * 0x00 : never
 * 0x01 : when going from IDLE to RX or TX
 * 0x10 : when going from RX or TX back to IDLE
 * 0x11 : every 4th time when going from RX or TX to IDLE
 */

void cc1100_set_calibration_policy(uint8_t policy) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_MCSM0, reg);
  reg  = (reg & 0xCF) | ((policy & 0x03) << 4);
  CC1100_SPI_WREG(CC1100_REG_MCSM0, reg);
}


void cc1100_set_rxoff_mode(uint8_t policy) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_MCSM1, reg);
  reg  = (reg & 0xF3) | ((policy << 2) & 0x0C);
  CC1100_SPI_WREG(CC1100_REG_MCSM1, reg);
  cc1100_rx_off = policy & 0x03;
}


void cc1100_set_txoff_mode(uint8_t policy) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_MCSM1, reg);
  reg  = (reg & 0xFC) | ((policy) & 0x03);
  CC1100_SPI_WREG(CC1100_REG_MCSM1, reg);
  cc1100_tx_off = policy & 0x03;
}

/* **************************************************
 * Packet Control PKTCTRL0
 * **************************************************/

void cc1100_set_data_whitening(uint8_t mode)
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_PKTCTRL0, reg);
  reg = (reg & 0xbf) | ((mode << 6) & 0x40);
  CC1100_SPI_WREG(CC1100_REG_PKTCTRL0, reg);
}



/* packet format interface */
#define CC1100_FORMAT_NORMAL       0x00 /* default */
#define CC1100_FORMAT_SERIAL_SYNC  0x01
#define CC1100_FORMAT_RANDOM       0x02
#define CC1100_FORMAT_SERIAL_ASYNC 0x03

void cc1100_set_packet_format(uint8_t format)
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_PKTCTRL0, reg);
  reg  = (reg & 0xcf) | ((format & 0x03) << 4);
  CC1100_SPI_WREG(CC1100_REG_PKTCTRL0, reg);
}


void cc1100_set_crc_policy(uint8_t policy)
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_PKTCTRL0, reg);
  reg  = (reg & 0xFB) | ((policy & 0x01) << 2);
  CC1100_SPI_WREG(CC1100_REG_PKTCTRL0, reg);
}



/* packet length policy */
#define CC1100_PLENGTH_FIXED    0x00
#define CC1100_PLENGTH_VARIABLE 0x01
#define CC1100_PLENGTH_INFINITE 0x02
#define CC1100_PLENGTH_RESERVED 0x03

void  cc1100_set_plength_policy(uint8_t policy)
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_PKTCTRL0, reg);
  reg  = (reg & 0xFC) | (policy & 0x03);
  CC1100_SPI_WREG(CC1100_REG_PKTCTRL0, reg);
}

/* void cc1100_fixed_size(uint8_t size) */
/* { */
/*   /\* Set variable packet length *\/ */
/*   cc1100_fixed_packet_length = size; */
/*   cc1100_set_plength_policy(CC1100_PLENGTH_FIXED); */
/*   CC1100_SPI_WREG(CC1100_REG_PKTLEN, size); */
/* } */

/* /\* pkt < 64 bytes, wait EOP *\/ */
/* void cc1100_fixed_utx(char *buffer) */
/* { */
/*   cc1100_tx_error = 0; */
/*   cc1100_check_fifo_xflow_flush(); */
/*   /\* Fill tx fifo *\/ */
/*   CC1100_SPI_TX_FIFO_BURST (buffer, cc1100_fixed_packet_length); */
/*   /\* Send packet and wait for complete *\/ */
/*   cc1100_gdo2_set_signal(CC1100_GDOx_SYNC_WORD); */
/*   CC1100_HW_GDO2_DINT(); */
/*   CC1100_SPI_STROBE(CC1100_STROBE_STX); */
/*   while (! ( CC1100_HW_GDO2_READ() )); /\* GDO2 goes high = SYNC TX *\/ */
/*   while (  ( CC1100_HW_GDO2_READ() )); /\* GDO2 goes low  = EOP     *\/ */
/*   CC1100_HW_GDO2_EINT(); */
/* } */

/* **************************************************
 * Packet Control PKTCTRL1
 * **************************************************/

/* Packet quality is used to gate sync word quality */
/* defined page 47 */
void  cc1100_set_packet_quality_threshold(uint8_t policy) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_PKTCTRL1, reg);
  reg  = (reg & 0x1F) | ((policy << 5) & 0xE0);
  CC1100_SPI_WREG(CC1100_REG_PKTCTRL1, reg);
}


void cc1100_set_rx_packet_status(uint8_t policy)
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_PKTCTRL1, reg);
  reg  = (reg & 0xFB) | ((policy & 0x01) << 2);
  CC1100_SPI_WREG(CC1100_REG_PKTCTRL1, reg);
}

/* **************************************************
 * Modulation configuration MDMCFG1
 * **************************************************/

void cc1100_set_fec_policy(uint8_t mode)
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_MDMCFG1, reg);
  reg  = (reg & 0x7F) | ((mode & 0x1) << 7);
  CC1100_SPI_WREG(CC1100_REG_MDMCFG1, reg);
}

/* **************************************************
 * Modulation configuration MDMCFG2
 * **************************************************/

void cc1100_set_modulation(uint8_t modulation) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_MDMCFG2, reg);
  reg  = (reg & 0x8F) | ((modulation << 4) & 0x70);
  CC1100_SPI_WREG(CC1100_REG_MDMCFG2, reg);
}


void cc1100_set_manchester(uint8_t coding) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_MDMCFG2, reg);
  reg  = (reg & 0xF7) | ((coding << 3) & 0x08);
  CC1100_SPI_WREG(CC1100_REG_MDMCFG2, reg);
}


/* see page 51*/
#define CC1100_SYNC_MODE_NO       0x00
#define CC1100_SYNC_MODE_15_16    0x01
#define CC1100_SYNC_MODE_16_16    0x02
#define CC1100_SYNC_MODE_30_32    0x03
#define CC1100_SYNC_MODE_NO_CS    0x04
#define CC1100_SYNC_MODE_15_16_CS 0x05
#define CC1100_SYNC_MODE_16_16_CS 0x06
#define CC1100_SYNC_MODE_30_32_CS 0x07

void  cc1100_set_sync_mode(uint8_t policy) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_MDMCFG2, reg);
  reg  = (reg & 0xF8) | ((policy) & 0x07);
  CC1100_SPI_WREG(CC1100_REG_MDMCFG2, reg);
}


/* **************************************************
 * Tx/Rx Freq / Power / Gain
 * **************************************************/

void cc1100_set_tx_power(uint8_t config) 
{
  uint8_t reg;
  CC1100_SPI_RREG(CC1100_REG_FREND0,reg);
  reg = (reg & 0xF8) | (config & 0x07); /* bits 0,1,2 */
  CC1100_SPI_WREG(CC1100_REG_FREND0,reg);
}


void cc1100_set_freq_mhz(uint8_t config) 
{
  uint8_t reg_chan  = 0; /* channel number   */
  uint8_t reg_freq2 = 0; /* freq high byte   */
  uint8_t reg_freq1 = 0; /* freq mid byte    */
  uint8_t reg_freq0 = 0; /* freq low byte    */

  /* values taken from SmartRF Studio 6.5    */
  /*             MHz   315   433   868   915 */
#if defined(FREQ_BASE_26MHz)
  uint8_t freq2[] = { 0x0C, 0x10, 0x21, 0x23 };
  uint8_t freq1[] = { 0x1D, 0xA7, 0x62, 0x31 };
  uint8_t freq0[] = { 0x89, 0x62, 0x76, 0x3B };
  uint8_t chan [] = { 0x00, 0x00, 0x00, 0x00 };
#elif defined(FREQ_BASE_27MHz)
  uint8_t freq2[] = { 0x0B, 0x10, 0x20, 0x21 };
  uint8_t freq1[] = { 0xAA, 0x09, 0x25, 0xE3 };
  uint8_t freq0[] = { 0xAA, 0x7B, 0xED, 0x8E };
  uint8_t chan [] = { 0x00, 0x00, 0x00, 0x00 };
#else
  #error "cc1100 must define a crystal reference frequency"
#endif


  reg_freq2          = freq2   [config];
  reg_freq1          = freq1   [config];
  reg_freq0          = freq0   [config];
  reg_chan           = chan    [config];
  cc1100_patable     = patable [config];
  cc1100_freq_config = config; 

  CC1100_SPI_WREG(CC1100_REG_FREQ2,  reg_freq2); /* freq high byte */
  CC1100_SPI_WREG(CC1100_REG_FREQ1,  reg_freq1); /* freq mid  byte */
  CC1100_SPI_WREG(CC1100_REG_FREQ0,  reg_freq0); /* freq low  byte */
  CC1100_SPI_WREG(CC1100_REG_CHANNR, reg_chan);  /* channel number */
  CC1100_SPI_TX_BURST(CC1100_PATABLE_ADDR,cc1100_patable,patable_length);
}


void cc1100_set_channel(uint8_t chan)
{
  CC1100_SPI_WREG(CC1100_REG_CHANNR, chan);
}


void cc1100_set_rx_gain(uint8_t config)
{
#if 0
  /* Automatic Gain Control */
  CC1100_SPI_WREG(CC1100_REG_AGCCTRL2,reg2);
  CC1100_SPI_WREG(CC1100_REG_AGCCTRL1,reg1);
  CC1100_SPI_WREG(CC1100_REG_AGCCTRL0,reg0);
#endif
}

/* **************************************************
 * Tx operations
 * **************************************************/

/* pkt < 64 bytes, wait EOP */
void cc1100_utx(char *buffer, uint8_t length) 
{
  cc1100_tx_error = 0;
  cc1100_idle();

  /* Fill tx fifo */
  CC1100_SPI_TX_FIFO_BYTE  (length);
  CC1100_SPI_TX_FIFO_BURST (buffer, length);
  /* Send packet and wait for complete */
  CC1100_HW_GDO2_DINT();

  cc1100_gdo2_set_signal(CC1100_GDOx_SYNC_WORD); /* gdo2 sync & eop           */
  CC1100_SPI_STROBE(CC1100_STROBE_STX);

#define NO_STOP_READ_TX_FIFO_BYTES
#if defined(STOP_READ_TX_FIFO_BYTES)
  {
    uint8_t txbytes;
    do {
      CC1100_SPI_ROREG(CC1100_REG_TXBYTES, txbytes);
    } while ((txbytes & 0x7f) > 0);
  }
#else
  while ( ( CC1100_HW_GDO2_READ() ) == 0);      /* GDO2 goes high = SYNC TX */
  while ( ( CC1100_HW_GDO2_READ() ) != 0);      /* GDO2 goes low  = EOP     */
#endif
  CC1100_HW_GDO0_CLEAR_FLAG();
  CC1100_HW_GDO2_CLEAR_FLAG();
}

/* pkt < 64 bytes, don't wait for EOP */
void cc1100_utx_async(char *buffer, uint8_t length)
{
  cc1100_check_fifo_xflow_flush();
  //  while (cc1100_tx_ongoing || cc1100_rx_ongoing) ;
  cc1100_tx_ongoing = 1;
  cc1100_tx_error   = 0;
  cc1100_tx_sent    = 0;
  /* Fill tx fifo */
  CC1100_SPI_TX_FIFO_BYTE  (length);
  CC1100_SPI_TX_FIFO_BURST (buffer, length);

  /* Send packet but don't wait complete */
  cc1100_gdo2_set_signal(CC1100_GDOx_SYNC_WORD); /* gdo2 sync & eop           */
  CC1100_HW_GDO2_DINT();
  CC1100_HW_GDO0_DINT();
  CC1100_SPI_STROBE(CC1100_STROBE_STX);          /* start                     */
  while (! ( CC1100_HW_GDO2_READ() ));           /* GDO2 goes high = sync TX  */
  CC1100_HW_GDO2_IRQ_ON_DEASSERT();              /* want an interrupt for EOP */
  CC1100_HW_GDO2_EINT();                         /* allow interrupts on gdo2  */
}


/* called when done on asynchroneous Tx */
void cc1100_tx_done_intr(void) 
{
  cc1100_tx_ongoing = 0;
  if (cc1100_check_fifo_xflow_flush()) 
    {
      cc1100_tx_error = 1;
    }
  cc1100_tx_sent    = 1;

#if defined(CC_IRQ) 
#define MOV_R5R5()   __asm__ __volatile__ ("mov r5,r5")
#define STOP_ESIMU  MOV_R5R5
#warning "Esimu stop on EOP"
      /* */ STOP_ESIMU();
#endif
}


#define MIN(a,b) (a < b) ? (a) : (b)

#define TX_WRITE_BLOCK()                               \
do {                                                   \
  CC1100_SPI_ROREG(CC1100_REG_TXBYTES, txbytes);       \
  txbytes = 63 - txbytes; /* room free in Tx FIFO */   \
  tosend = MIN(txbytes, cc1100_tx_length);             \
  CC1100_SPI_TX_FIFO_BURST(cc1100_tx_packet, tosend);  \
  cc1100_tx_packet += tosend;                          \
  cc1100_tx_length -= tosend;                          \
} while (0)


void cc1100_tx_pkt_data()
{
  uint8_t txbytes; /* bytes free in the fifo */
  uint8_t tosend;

  TX_WRITE_BLOCK();
}


void cc1100_tx(char *buffer, uint8_t length)
{
  uint8_t txbytes; /* bytes free in the fifo */
  uint8_t tosend;
  
  if (length < 63)
    {
      cc1100_utx(buffer,length);
      return;
    }

  cc1100_check_fifo_xflow_flush();
  cc1100_tx_packet  = buffer;
  cc1100_tx_length  = length;
  cc1100_tx_offset  = 0;
  cc1100_tx_ongoing = 1;
  cc1100_tx_error   = 0;
  cc1100_tx_sent    = 0;
  /* Fill tx fifo */
  CC1100_SPI_TX_FIFO_BYTE  (length);
  TX_WRITE_BLOCK();

  /* Send packet but don't wait complete */
  CC1100_HW_GDO2_DINT();
  CC1100_HW_GDO0_DINT();

  cc1100_gdo0_set_signal(CC1100_GDOx_TX_FIFO);   /* gdo0 tx fifo            */
  cc1100_gdo2_set_signal(CC1100_GDOx_SYNC_WORD); /* gdo2 sync & eop         */
  CC1100_HW_GDO2_IRQ_ON_DEASSERT();              /* want an irq for EOP     */
  CC1100_HW_GDO0_IRQ_ON_DEASSERT();              /* want an irq on Tx < thr */
  CC1100_SPI_STROBE(CC1100_STROBE_STX);          /* start                   */
  while (! ( CC1100_HW_GDO2_READ() ));           /* GDO2 high = sync TX     */
  CC1100_HW_GDO0_EINT();                         /* allow irq on gdo0       */
  CC1100_HW_GDO2_EINT();                         /* allow irq on gdo2       */
}

/* **************************************************
 * Rx operations
 * **************************************************/

/* cc1100 operations */
void cc1100_rx_register_buffer   (char* buffer, uint8_t length);
void cc1100_rx_register_rx_cb    (cc1100_rx_cb_t f);
void cc1100_rx_enter             (void);
void cc1100_rx_exit              (void);

void cc1100_rx_pkt_start_polling (void); /* Polling Rx                         */
void cc1100_rx_pkt_start_irq     (void); /* SYNC intr                          */
void cc1100_rx_pkt_data          (void); /* fifo threshold intr during receive */



void cc1100_rx_register_buffer(char* buffer, uint8_t length)
{
  cc1100_rx_packet = buffer;
  cc1100_rx_offset = 0x00;
  cc1100_rx_length = length;
}

void cc1100_rx_enter(void) 
{
  cc1100_idle();

  cc1100_rx_offset   = 0;
  cc1100_rx_ongoing  = 0;

  CC1100_HW_GDO2_IRQ_ON_ASSERT();                /* want an interrupt  */
  cc1100_gdo2_set_signal(CC1100_GDOx_SYNC_WORD); /* interrupt on SYNC  */
  CC1100_HW_GDO0_CLEAR_FLAG();                   /* clear pending irq  */
  CC1100_HW_GDO2_CLEAR_FLAG();                   /* clear pending irq  */
  CC1100_HW_GDO2_EINT();                         /* eint               */

  CC1100_SPI_STROBE(CC1100_STROBE_SRX);
  cc1100_wait_status(CC1100_STATUS_RX);
}

void cc1100_rx_exit(void) 
{
  cc1100_check_fifo_xflow_flush();
  while (cc1100_rx_ongoing) ; 

  cc1100_idle();
}

void cc1100_rx_register_rx_cb(cc1100_rx_cb_t f)
{
  cc1100_rx_cb = f;
}

int cc1100_rx_switch_mode(int mode)
{
  switch (mode)
    {
    case CC1100_RX_MODE_IRQ:
      cc1100_rx_irq_cb = cc1100_rx_pkt_start_irq;
      break;
    case CC1100_RX_MODE_POLLING:
      cc1100_rx_irq_cb = cc1100_rx_pkt_start_polling;
      CC1100_HW_GDO0_DINT();                         /* disable IRQ               */
      break;
    default:
      return 1;
    }
  return 0;
}

#define CC1100_FLUSH_RX()					\
  do {								\
    CC1100_SPI_STROBE(CC1100_STROBE_SFRX);			\
    cc1100_wait_status(CC1100_STATUS_IDLE);			\
  } while (0)


#define CC1100_FLUSH_TX()					\
  do {								\
    CC1100_SPI_STROBE(CC1100_STROBE_SFTX);			\
    cc1100_wait_status(CC1100_STATUS_IDLE);			\
  } while (0)


/* ****************** */
/* ** RX polling **** */
/* ****************** */

void cc1100_rx_pkt_start_polling(void)
{
  uint8_t rxbytes;

  dint();
  cc1100_rx_offset   = 0;
  cc1100_rx_ongoing  = 1;
  // printf("    p"); 
  // cc1100_gdo2_set_signal(CC1100_GDOx_SYNC_WORD); /* for EOP                   */
  CC1100_HW_GDO0_DINT();                         /* disable IRQ               */
  CC1100_HW_GDO2_DINT();                         /* disable IRQ               */

  while ( CC1100_HW_GDO2_READ() != 0)            /* GDO2 goes low  = EOP      */
    {
      nop();           
    }
  
  /* packet is fully received */
#define SAFE_READ
#if defined(SAFE_READ)
  {
    int l;
    CC1100_SPI_ROREG(CC1100_REG_RXBYTES, rxbytes);
    do { 
      l = rxbytes;  
      CC1100_SPI_ROREG(CC1100_REG_RXBYTES, rxbytes);
    } while (rxbytes < 2 && rxbytes != l);
  }
#else
  CC1100_SPI_ROREG(CC1100_REG_RXBYTES, rxbytes); /* # of received bytes       */
#endif

  // printf(" - rx:%d -",rxbytes);
  if (rxbytes > 0)
    {
      if ((rxbytes & 0x80) == 0)
	{
	  uint8_t size;
	  CC1100_SPI_RX_FIFO_BYTE(size);
	  // printf(" - rx2:%d -",size);
	  /* 
	   * rxbytes can be different from size+1 
	   * this happens if packets is filled with a serie of
	   * same bytes
	   */
	  CC1100_SPI_RX_FIFO_BURST(cc1100_rx_packet, rxbytes - 1);
	  cc1100_rx_cb(cc1100_rx_packet,size);  
	}
      else
	{
	  // printf(" - rx2: over -");
	  CC1100_FLUSH_RX();
	  cc1100_rx_cb(cc1100_rx_packet,-ERXFLOW);
	}
    }
  else
    {
      // printf(" - rx2: empty ");
      cc1100_rx_cb(cc1100_rx_packet,-EEMPTY);  
    }
  // printf(" - cb done\n");
  cc1100_rx_ongoing  = 0;

  CC1100_HW_GDO0_CLEAR_FLAG();
  CC1100_HW_GDO2_CLEAR_FLAG();
  CC1100_HW_GDO2_EINT();                         /* re-enable IRQ             */
  eint();
}


/* ****************** */
/* ** RX IRQ ******** */
/* ****************** */


void cc1100_rx_pkt_start_irq(void) 
{
  cc1100_rx_offset   = 0;
  //  cc1100_rx_received = 0;
  cc1100_rx_ongoing  = 1;
  cc1100_rx_length   = 0;

  CC1100_HW_GDO2_IRQ_ON_DEASSERT();             /* gdo2 want an interrupt for EOP   */
  cc1100_gdo0_set_signal(CC1100_GDOx_RX_FIFO);  /* gdo0 on threshold                */
  CC1100_HW_GDO0_IRQ_ON_ASSERT();               /* want an interrupt on Rx fifo thr */
  CC1100_HW_GDO0_EINT();                        /* enable                           */
}

void cc1100_rx_pkt_data(void)
{
  uint8_t *buff;
  uint8_t  rxbytes;

  /*  printf("i"); */
  CC1100_SPI_ROREG(CC1100_REG_RXBYTES, rxbytes);  /* # of received bytes */
  if (rxbytes > 0)					     
    {
      if (cc1100_rx_offset == 0)                  /* first byte */
	{
	  CC1100_SPI_RX_FIFO_BYTE(cc1100_rx_length);
	  rxbytes --;
	}
      if (rxbytes > 0)
	{
	  buff = cc1100_rx_packet + cc1100_rx_offset;     
	  CC1100_SPI_RX_FIFO_BURST(buff, rxbytes);   
	  cc1100_rx_offset += rxbytes;
	}
    }							     

  if (CC1100_HW_GDO2_READ() == 0) /* EOP ? */
    {
      /* purge fifo */
      CC1100_SPI_ROREG(CC1100_REG_RXBYTES, rxbytes);  /* # of received bytes */
      if (rxbytes > 0)					     
	{
	  buff = cc1100_rx_packet + cc1100_rx_offset;     
	  CC1100_SPI_RX_FIFO_BURST(buff, rxbytes);   
	  cc1100_rx_offset += rxbytes;
	}

      /* packet finished */
      //      cc1100_rx_received  = 1;
      cc1100_rx_ongoing   = 0;
      CC1100_HW_GDO2_IRQ_ON_ASSERT();     /* interrupts for next SYNC */
      cc1100_rx_cb(cc1100_rx_packet,cc1100_rx_length);  
    }
}


/* **************************************************
 * Interupts handler
 * **************************************************/

void cc1100_interrupt_handler(uint8_t pin) 
{
  int interrupt_policy;

  if (pin == CC1100_GDO0)
    {
      interrupt_policy = cc1100_gdo0_cfg;
    }
  else if (pin == CC1100_GDO2)
    {
      interrupt_policy = cc1100_gdo2_cfg;
    }
  else 
    {
      return;
    }

  /* cc1100_tx() : gdo0 = tx fifo, gdo2 = sync & eop */
  
  /* Inside interruption handling */
  CC1100_ENTER_INTERRUPT();

  //  printf("    irq policy %x\n",interrupt_policy);

  switch (interrupt_policy) 
    {
    case CC1100_GDOx_RX_FIFO:          /* Rx FIFO threshold */
      cc1100_rx_pkt_data();
      break;

    case CC1100_GDOx_TX_FIFO:          /* Tx FIFO threshold */
      cc1100_tx_pkt_data(); 
      break;

    case CC1100_GDOx_SYNC_WORD:
      if (cc1100_tx_ongoing == 1)      /* Tx EOP */
	{
	  cc1100_tx_done_intr();
	}
      else if (cc1100_rx_ongoing == 1) /* Rx EOP */
	{
	  cc1100_rx_pkt_data();
	}
      else                             /* Rx SYNC */
	{
	  cc1100_rx_irq_cb();
	}
      break;
    case CC1100_GDOx_CHIP_RDY:
      break;

    default:
      break;
    }  
  
  CC1100_EXIT_INTERRUPT();
  /* Outside interruption handling */  
}


/* **************************************************
 * Modes Idle/Sleep/Xoff operations
 * **************************************************/

void cc1100_idle(void)
{
  cc1100_check_fifo_xflow_flush();
  CC1100_SPI_STROBE(CC1100_STROBE_SIDLE);
  cc1100_wait_status(CC1100_STATUS_IDLE);
}

/* xoff mode
 * - crystal is shut down
 * - configuration reset
 * - calibration needed after xoff
 */
int cc1100_xoff(void) 
{
  cc1100_check_fifo_xflow_flush();
  while (cc1100_tx_ongoing || cc1100_rx_ongoing) ;
  CC1100_SPI_STROBE(CC1100_STROBE_SXOFF);
  return 0;
}


/* sleep mode with wake on radio
 * - crystal is off
 * - configuration saved
 * - calibration needed after sleep
 */
int cc1100_sleep_wor(void) 
{
  cc1100_check_fifo_xflow_flush();
  while (cc1100_tx_ongoing || cc1100_rx_ongoing) ;
  CC1100_SPI_STROBE(CC1100_STROBE_SWOR);
  return 0;
}

/* sleep mode 
 * - crystal is off
 * - configuration saved except power table and test registers
 * - calibration and power table update needed after sleep
 */
int cc1100_sleep(void) 
{
  cc1100_check_fifo_xflow_flush();
  while (cc1100_tx_ongoing || cc1100_rx_ongoing) ;
  CC1100_SPI_STROBE(CC1100_STROBE_SPWD);
  return 0;
}

/* **************************************************
 * utils
 * **************************************************/

uint8_t cc1100_packet_status(void)
{
  uint8_t ps;
  CC1100_SPI_RREG(CC1100_REG_PKTSTATUS,ps);
  return ps;
}


int cc1100_cca(void)
{
  uint8_t cca = cc1100_packet_status();
  return (cca >> 4) & 0x01;
}


uint8_t cc1100_get_rssi(void)
{
  uint8_t rssi;
  /* cc1100 should be in Rx */
  CC1100_SPI_RREG(CC1100_REG_RSSI,rssi);
  return rssi;
}

/* **************************************************
 * Init / Reset
 * **************************************************/

void cc1100_variables_bootstrap(void) 
{
  cc1100_status_register = 0;
  cc1100_intr            = 0;

  /* Default values for internal cc1100 variables */
  cc1100_gdo2_cfg        = CC1100_GDOx_CHIP_RDY;
  cc1100_gdo0_cfg        = CC1100_GDOx_CLK_XOSC_192;

  cc1100_fifo_threshold  = 0x07;
  cc1100_rx_off          = 0x00;
  cc1100_tx_off          = 0x00;

  cc1100_freq_config     = CC1100_FREQ_868;
  cc1100_patable         = patable_868;

  /* Internal driver variables for tx/rx */
  cc1100_tx_ongoing      = 0x00;
  cc1100_tx_error        = 0x00;
  cc1100_tx_packet       = 0x00;
  cc1100_tx_offset       = 0x00;
  cc1100_tx_length       = 0x00;
  cc1100_tx_sent         = 0x00;

  cc1100_rx_ongoing      = 0x00;
  cc1100_rx_packet       = 0x00;
  cc1100_rx_offset       = 0x00;
  cc1100_rx_length       = 0x00;
  //  cc1100_rx_received     = 0x00;

  
  cc1100_rx_irq_cb       = cc1100_rx_pkt_start_polling;
  /* cc1100_rx_irq_cb       = cc1100_rx_pkt_start_irq; */

}



void cc1100_reset(void)
{
  cc1100_idle();
  CC1100_SPI_STROBE(CC1100_STROBE_SRES);
  cc1100_variables_bootstrap();	
}



void cc1100_init(void) 
{
  /* bootstrap global cc1100 variables */
  cc1100_variables_bootstrap();	
  
  /* Configure pins */
  CC100_SPI_HW_INIT();

  /* Reset */
  CC1100_SPI_STROBE(CC1100_STROBE_SRES);
  
  /* Set manual calibration policy */
  /* cc1100_set_calibration_policy(CC1100_CALIBRATION_NEVER); */
  cc1100_set_calibration_policy(CC1100_CALIBRATION_IDLE_TX_RX);
  
  /* Set Tx frequency to 868MHz */
  cc1100_set_freq_mhz(CC1100_FREQ_868);

  /* Calibrate */
  cc1100_calibrate();

  /* Set PQT & sync mode */
  cc1100_set_sync_mode(CC1100_SYNC_MODE_30_32);
  cc1100_set_packet_quality_threshold(0x06);

  /* Set variable packet length */
  cc1100_set_plength_policy(CC1100_PLENGTH_VARIABLE);
  
  /* Set output power */
  cc1100_set_tx_power(CC1100_TX_POWER_CONFIG_00DBM);
  
  /* Set modulation */
  cc1100_set_modulation(CC1100_MODULATION_2FSK);
  
  /* Unset manchester encoding */
  cc1100_set_manchester(CC1100_MANCHESTER_DISABLE);
  
  /* Set Data whitening */
  cc1100_set_data_whitening(CC1100_WHITEDATA_DISABLE);

  /* Set CRC on */
  cc1100_set_crc_policy(CC1100_CRC_ENABLE);

  /* Set FEC and interleaving */
  cc1100_set_fec_policy(CC1100_FEC_DISABLE);

  /* GDO0 asserted when rx fifo above threshold */
  cc1100_gdo0_set_signal(CC1100_GDOx_RX_FIFO);
  CC1100_HW_GDO0_IRQ_ON_ASSERT();
  CC1100_HW_GDO0_DINT();
  
  /* GDO2 Deasserted when packet rx/tx or fifo xxxflow */
  cc1100_gdo2_set_signal(CC1100_GDOx_SYNC_WORD);
  CC1100_HW_GDO2_IRQ_ON_ASSERT();
  CC1100_HW_GDO2_DINT();
  
  /* Go in idle state after tx and idle after rx */
  cc1100_set_txoff_mode(CC1100_TXOFF_IDLE);
  cc1100_set_rxoff_mode(CC1100_RXOFF_IDLE);
}
