/**
 *  \file   main.c
 *  \brief  eZ430-RF2500 tutorial, radio
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#include <msp430x22x4.h>

#if defined(__GNUC__) && defined(__MSP430__)
/* This is the MSPGCC compiler */
#include <io.h>
#include <iomacros.h>
#include <signal.h>
#elif defined(__IAR_SYSTEMS_ICC__)
/* This is the IAR compiler */
//#include <io430.h>
#endif

#include <stdio.h>
#include <string.h>

#include "math.h"
#include "leds.h"
#include "clock.h"
#include "timer.h"
#include "uart.h"
#include "watchdog.h"
#include "spi.h"
#include "cc2500.h"
#include "lpm_compat.h"

#define MSG_SIZE 60

#define USE_CONFIGURATION_0
// #define USE_CONFIGURATION_1
// #define USE_CONFIGURATION_3

/* ************************************************************ */
/* ************************************************************ */
#if defined(USE_CONFIGURATION_0)
#warning "** **************************************************"
#warning "** Using user configuration 0"
#warning "** **************************************************"
#define USER_RFCONFIG config0

// Chipcon
// Product = CC2500
// Chip version = E   (VERSION = 0x03)
// Crystal accuracy = 10 ppm
// X-tal frequency = 26 MHz
// RF output power = 0 dBm
// RX filterbandwidth = 541.666667 kHz
// Phase = 1
// Datarate = 249.938965 kBaud
// Modulation = (7) MSK
// Manchester enable = (0) Manchester disabled
// RF Frequency = 2432.999908 MHz
// Channel spacing = 199.951172 kHz
// Channel number = 0
// Optimization = Current
// Sync mode = (3) 30/32 sync word bits detected
// Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
// CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
// Forward Error Correction = (0) FEC disabled
// Length configuration = (1) Variable length packets, packet length configured by the first received byte after sync word.
// Packetlength = 255
// Preamble count = (2)  4 bytes
// Append status = 1
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
// GDO2 signal selection = (41) CHIP_RDY
const RF_SETTINGS config0 = {
    0x12,   // FSCTRL1   Frequency synthesizer control.
    0x00,   // FSCTRL0   Frequency synthesizer control.
    0x5D,   // FREQ2     Frequency control word, high byte.
    0x93,   // FREQ1     Frequency control word, middle byte.
    0xB1,   // FREQ0     Frequency control word, low byte.
    0x2D,   // MDMCFG4   Modem configuration.
    0x3B,   // MDMCFG3   Modem configuration.
    0xF3,   // MDMCFG2   Modem configuration.
    0x22,   // MDMCFG1   Modem configuration.
    0xF8,   // MDMCFG0   Modem configuration.
    0x03,   // CHANNR    Channel number.
    0x01,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    0xB6,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Front end TX configuration.
    0x18,   // MCSM0     Main Radio Control State Machine configuration.
    0x1D,   // FOCCFG    Frequency Offset Compensation Configuration.
    0x1C,   // BSCFG     Bit synchronization Configuration.
    0xC7,   // AGCCTRL2  AGC control.
    0x00,   // AGCCTRL1  AGC control.
    0xB0,   // AGCCTRL0  AGC control.
    0xEA,   // FSCAL3    Frequency synthesizer calibration.
    0x0A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x11,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.
    0x88,   // TEST2     Various test settings.
    0x31,   // TEST1     Various test settings.
    0x0B,   // TEST0     Various test settings.
    0x07,   // FIFOTHR   RXFIFO and TXFIFO thresholds.
    0x29,   // IOCFG2    GDO2 output pin configuration.
    0x06,   // IOCFG0D   GDO0 output pin configuration. Refer to SmartRF® Studio User Manual for detailed pseudo register explanation.
    0x04,   // PKTCTRL1  Packet automation control.
    0x05,   // PKTCTRL0  Packet automation control.
    0x00,   // ADDR      Device address.
    0xFF    // PKTLEN    Packet length.
};


#elif defined(USE_CONFIGURATION_1)
#warning "** **************************************************"
#warning "** Using user configuration 1"
#warning "** **************************************************"
#define USER_RFCONFIG config1

// same as library config 0 + Manchester + Whitening

// Chipcon
// Product = CC2500
// Chip version = E   (VERSION = 0x03)
// Crystal accuracy = 10 ppm
// X-tal frequency = 26 MHz
// RF output power = 0 dBm
// RX filterbandwidth = 541.666667 kHz
// Phase = 1
// Datarate = 249.938965 kBaud
// Modulation = (7) MSK
// Manchester enable = (1) Manchester enabled + data whitening
// RF Frequency = 2432.999908 MHz
// Channel spacing = 199.951172 kHz
// Channel number = 0
// Optimization = Current
// Sync mode = (3) 30/32 sync word bits detected
// Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
// CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
// Forward Error Correction = (0) FEC disabled
// Length configuration = (1) Variable length packets, packet length 
//                         configured by the first received byte after sync word.
// Packetlength = 255
// Preamble count = (2)  4 bytes
// Append status = 1
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, 
//                          and de-asserts at the end of the packet
// GDO2 signal selection = (41) CHIP_RDY

const RF_SETTINGS config1 = {
    0x12,   // FSCTRL1   Frequency synthesizer control.
    0x00,   // FSCTRL0   Frequency synthesizer control.
    0x5D,   // FREQ2     Frequency control word, high byte.
    0x93,   // FREQ1     Frequency control word, middle byte.
    0xB1,   // FREQ0     Frequency control word, low byte.
    0x2D,   // MDMCFG4   Modem configuration.
    0x3B,   // MDMCFG3   Modem configuration.
    0xF3,   // MDMCFG2   Modem configuration.
    0x22,   // MDMCFG1   Modem configuration.
    0xFB,   // MDMCFG0   Modem configuration. // Manchester
    0x00,   // CHANNR    Channel number.
    0x01,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    0xB6,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Front end TX configuration.
    0x18,   // MCSM0     Main Radio Control State Machine configuration.
    0x1D,   // FOCCFG    Frequency Offset Compensation Configuration.
    0x1C,   // BSCFG     Bit synchronization Configuration.
    0xC7,   // AGCCTRL2  AGC control.
    0x00,   // AGCCTRL1  AGC control.
    0xB0,   // AGCCTRL0  AGC control.
    0xEA,   // FSCAL3    Frequency synthesizer calibration.
    0x0A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x11,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.
    0x88,   // TEST2     Various test settings.
    0x31,   // TEST1     Various test settings.
    0x0B,   // TEST0     Various test settings.
    0x07,   // FIFOTHR   RXFIFO and TXFIFO thresholds.
    0x29,   // IOCFG2    GDO2 output pin configuration.
    0x06,   // IOCFG0D   GDO0 output pin configuration. 
    0x04,   // PKTCTRL1  Packet automation control.
    0x45,   // PKTCTRL0  Packet automation control. // Whitening
    0x00,   // ADDR      Device address.
    0xFF    // PKTLEN    Packet length.
};

#elif defined(USE_CONFIGURATION_2)
#warning "** **************************************************"
#warning "** Using user configuration 2"
#warning "** **************************************************"
#define USER_RFCONFIG config2

// Chipcon
// Product = CC2500
// Chip version = E   (VERSION = 0x03)
// Crystal accuracy = 10 ppm
// X-tal frequency = 26 MHz
// RF output power = 0 dBm
// RX filterbandwidth = 232.142857 kHz
// Deviation = 38 kHz
// Datarate = 9.992599 kBaud
// Modulation = (0) 2-FSK
// Manchester enable = (1) Manchester enabled
// RF Frequency = 2432.999908 MHz
// Channel spacing = 199.951172 kHz
// Channel number = 0
// Optimization = Sensitivity
// Sync mode = (0) No preamble/sync
// Format of RX/TX data = (1) Serial Synchronous mode, used for backwards compatibility
// CRC operation = (0) CRC disabled for TX and RX
// Forward Error Correction = (0) FEC disabled
// Length configuration = (2) Enable infinite length packets.
// Packetlength = 255
// Preamble count = (2)  4 bytes
// Append status = 1
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = (12) Serial Synchronous Data Output
// GDO2 signal selection = (11) Serial Clock
const RF_SETTINGS config2 = {
    0x06,   // FSCTRL1   Frequency synthesizer control.
    0x00,   // FSCTRL0   Frequency synthesizer control.
    0x5D,   // FREQ2     Frequency control word, high byte.
    0x93,   // FREQ1     Frequency control word, middle byte.
    0xB1,   // FREQ0     Frequency control word, low byte.
    0x78,   // MDMCFG4   Modem configuration.
    0x93,   // MDMCFG3   Modem configuration.
    0x08,   // MDMCFG2   Modem configuration.
    0x22,   // MDMCFG1   Modem configuration.
    0xF8,   // MDMCFG0   Modem configuration.
    0x00,   // CHANNR    Channel number.
    0x44,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    0x56,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Front end TX configuration.
    0x18,   // MCSM0     Main Radio Control State Machine configuration.
    0x16,   // FOCCFG    Frequency Offset Compensation Configuration.
    0x6C,   // BSCFG     Bit synchronization Configuration.
    0x43,   // AGCCTRL2  AGC control.
    0x40,   // AGCCTRL1  AGC control.
    0x91,   // AGCCTRL0  AGC control.
    0xA9,   // FSCAL3    Frequency synthesizer calibration.
    0x0A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x11,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.
    0x88,   // TEST2     Various test settings.
    0x31,   // TEST1     Various test settings.
    0x0B,   // TEST0     Various test settings.
    0x07,   // FIFOTHR   RXFIFO and TXFIFO thresholds.
    0x0B,   // IOCFG2    GDO2 output pin configuration.
    0x0C,   // IOCFG0D   GDO0 output pin configuration. 
    0x04,   // PKTCTRL1  Packet automation control.
    0x12,   // PKTCTRL0  Packet automation control.
    0x00,   // ADDR      Device address.
    0xFF    // PKTLEN    Packet length.
};
#endif

/* ************************************************************ */
/* ************************************************************ */

#if defined(TX)
#define ID 3

void tx(void)
{
  int i;
  char msg[MSG_SIZE];

  for(i=0; i<MSG_SIZE; i++)
    {
      /* msg[0] == id */
      msg[i] = ID + i;
    }

  led_green_on();

  spi_init();
  cc2500_init();
#if defined(USER_RFCONFIG)
  cc2500_configure(& USER_RFCONFIG );
#endif
  timerA_init();
  timerA_set_wakeup(1);
  timerA_start_milliseconds(1000);

  __enable_interrupt();

  for(;;)
    {
      LPM(1);
      led_green_switch();
      cc2500_utx(msg,MSG_SIZE);
    }
}

#endif

/* ************************************************************ */
/* ************************************************************ */

#if defined(RX)

static uint8_t buffer_rx_msg [MSG_SIZE];
static int     buffer_rx_rssi;
static char    buffer_rx_flag;

void radio_cb(uint8_t* buffer, int size, int8_t rssi)
{
  led_green_switch();
  switch (size)
    {
    case 0:
      //DBG_PRINTF("msg size 0\n");
      break;
    case -EEMPTY:
      //DBG_PRINTF("msg empty\n");
      break;
    case -ERXFLOW:
      //DBG_PRINTF("msg rx overflow\n");
      break;
    case -ERXBADCRC:
      //DBG_PRINTF("msg rx bad CRC\n");
      break;
    case -ETXFLOW:
      //DBG_PRINTF("msg tx overflow\n");
      break;
    default:
      if (size > 0)
	{
	  // memcpy(buffer_rx_msg, buffer, MSG_SIZE);
	  buffer_rx_rssi = rssi;
	  buffer_rx_flag = 1;
	}
      else
	{
	  /* packet error, drop */
	  //DBG_PRINTF("msg packet error size=%d\n",size);
	}
      break;
    }
  cc2500_idle();
  cc2500_rx_enter();
  led_green_switch();
}

void rx(void)
{
  int i = 0;
  int tstart = 0;

  uart_init(UART_9600_SMCLK_8MHZ);
  printf("rx node\n");

  timerA_init();
  timerA_set_wakeup(1);
  timerA_start_milliseconds(1100);

  buffer_rx_flag = 0;

  spi_init();
  cc2500_init();

#if defined(USER_RFCONFIG)
  printf(" -- user config\n");
  cc2500_configure(& USER_RFCONFIG );
#endif
  cc2500_rx_register_buffer(buffer_rx_msg, MSG_SIZE);
  cc2500_rx_register_cb(radio_cb);

  cc2500_rx_enter();
  printf(" -- start\n");

  printf("start\n");
  __enable_interrupt();
  
  for(;;)
    {
      LPM(1);
      led_red_switch();
      if (buffer_rx_flag == 1)
	{
	  printf(" -- paquet %d, emetteur %d, %d dBm\n",i++, buffer_rx_msg[0], buffer_rx_rssi);
	  buffer_rx_flag = 0;
	  if (tstart == 0)
	    {
	      // set next IRQ
	      timerA_start_milliseconds(1100);
	    }
	}
      else
	{
	  cc2500_idle();
	  cc2500_rx_enter();
	  timerA_start_milliseconds(1100);
	}
    }
}

#endif

/* ************************************************************ */
/* ************************************************************ */

int main(void)
{
    watchdog_stop();
    set_mcu_speed_dco_mclk_16MHz_smclk_8MHz();
    leds_init();

#if defined TX
    tx();
#elif defined(RX)
    rx();
#else
    #error "must define TX or RX"
#endif
    return 0;
}

/* ************************************************************ */
/* ************************************************************ */
 
