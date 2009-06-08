/*
 *  cc1100.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Modified by Antoine Fraboulet on 04/04/07
 *  Copyright 2005,2006,2007 __WorldSens__. All rights reserved.
 *
 */
#ifndef _CC1100_H
#define _CC1100_H


/************************************************/
/* Init                                         */
/************************************************/

/* calls pin settings and set power */
void    cc1100_init                  (void);

/* reset to idle state */
void    cc1100_reset                 (void);

/************************************************/
/* IRQ Handler                                  */
/************************************************/

#define CC1100_GDO0 0
#define CC1100_GDO2 2

void  cc1100_interrupt_handler(uint8_t pin);

// extern volatile uint8_t  cc1100_rx_received; /* set by driver, reset by application */
// extern volatile uint8_t  cc1100_tx_sent;     /* set by driver, reset by application */
// extern volatile uint8_t  cc1100_tx_ongoing;  /* set/reset by driver                 */
// extern volatile uint8_t  cc1100_rx_ongoing;  /* set/reset by driver                 */

/************************************************/
/* Freq / Channel / Tx Power / Rx Gain          */
/************************************************/

#define CC1100_FREQ_315 0
#define CC1100_FREQ_433 1
#define CC1100_FREQ_868 2
#define CC1100_FREQ_915 3

void    cc1100_set_freq_mhz          (uint8_t config);
void    cc1100_set_channel           (uint8_t chan);

#define CC1100_TX_POWER_CONFIG_00DBM  0 /*   0dBm */
#define CC1100_TX_POWER_CONFIG_05DBM  1 /* -05dBm */
#define CC1100_TX_POWER_CONFIG_10DBM  2 /* -10dBm */
#define CC1100_TX_POWER_CONFIG_15DBM  3 /* -15dBm */
#define CC1100_TX_POWER_CONFIG_20DBM  4 /* -20dBm */
#define CC1100_TX_POWER_CONFIG_30DBM  5 /* -30bBm */
#define CC1100_TX_POWER_CONFIG_P05DBM 6 /* + 5dBm */
#define CC1100_TX_POWER_CONFIG_P10DBM 7 /* +10dBm */

void    cc1100_set_tx_power          (uint8_t config);

/************************************************/
/* Rx/Tx                                        */
/************************************************/


/* 
 * Tx
 * < 63B packet limitation, fixed size
 *   _utx         synchronous sending
 */
void    cc1100_fixed_utx             (char *buffer);

/* 
 * Tx
 * < 63B packet limitation,
 *   _utx         synchronous sending
 *   _utx_asynch  asynchronous End-Of-Packet interrupt
 */
void    cc1100_utx                   (char *buffer, uint8_t length);
void    cc1100_utx_async             (char *buffer, uint8_t length);

/* 
 * Tx
 * < 255B packet limitation
 */
void    cc1100_tx                    (char *buffer, uint8_t length);

/*
 * Rx mode
 * max lenght 254 bytes
 * first buffer byte is received length  
 */

#define CC1100_RX_MODE_IRQ            0
#define CC1100_RX_MODE_POLLING        1
int     cc1100_rx_switch_mode        (int mode); /* switch mode       */
// int     cc1100_rx_poll_pkt           (void);     /* poll for 1 packet */


/*
 * Rx packet callbacks
 */

#define EEMPTY  1
#define ERXFLOW 2
#define ETXFLOW 3

typedef void (*cc1100_rx_cb_t)(uint8_t *data, int size);
void    cc1100_rx_register_rx_cb     (cc1100_rx_cb_t f);
void    cc1100_rx_register_buffer    (char *buffer, uint8_t length);


/* Tx/Rx Fifo threshold, page 46
 * value   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
 * TX     61 57 53 49 45 41 37 33 29 25 21 17 13  9  5  1
 * RX      4  8 12 16 20 24 28 32 36 40 44 48 52 56 60 64
 */
void    cc1100_set_fifo_threshold    (uint8_t thr);

/************************************************/
/* Major modes                                  */
/************************************************/

/* puts the CC1100 in idle mode = tx/rx ready */
void    cc1100_calibrate             (void);

/* puts the CC1100 in idle mode = tx/rx ready */
void    cc1100_idle                  (void);

/* enter xoff / registers are saved, crystal is off */
int     cc1100_xoff                  (void);

/* enter sleep with wake on radio */
int     cc1100_sleep_wor             (void);

/* enter sleep mode */
int     cc1100_sleep                 (void);

/* Start Rx mode */
void    cc1100_rx_enter              (void);

/************************************************/
/* Options                                      */
/************************************************/

/* 
 * clear channel assesment 
 *   0 : busy
 *   1 : clear
 */
int     cc1100_cca                   (void);

/*
 * RSSI
 */
uint8_t cc1100_get_rssi              (void);


#define CC1100_MODULATION_2FSK 0x00
#define CC1100_MODULATION_GFSK 0x01
#define CC1100_MODULATION_ASK  0x03
#define CC1100_MODULATION_MSK  0x07

void cc1100_set_modulation(uint8_t modulation);

/* transition mode after a packet transmit, page 54 */
#define CC1100_TXOFF_IDLE     0x00
#define CC1100_TXOFF_FSTXON   0x01 /* freq synth on, ready to Tx */
#define CC1100_TXOFF_STAY_TX  0x02
#define CC1100_TXOFF_RX       0x03

void cc1100_set_txoff_mode(uint8_t policy);

/* transition mode after a packet receive, page 54 */
#define CC1100_RXOFF_IDLE     0x00
#define CC1100_RXOFF_FSTXON   0x01 /* freq synth on, ready to Tx */
#define CC1100_RXOFF_TX       0x02 
#define CC1100_RXOFF_STAY_RX  0x03

void cc1100_set_rxoff_mode(uint8_t policy);

#define CC1100_CALIBRATION_NEVER      0x00
#define CC1100_CALIBRATION_IDLE_TX_RX 0x01
#define CC1100_CALIBRATION_TX_RX_IDLE 0x02
#define CC1100_CALIBRATION_ALL        0x03

void cc1100_set_calibration_policy(uint8_t policy);

#define CC1100_FEC_ENABLE       0x01
#define CC1100_FEC_DISABLE      0x00

void cc1100_set_fec_policy(uint8_t policy);

#define CC1100_CRC_ENABLE       0x01
#define CC1100_CRC_DISABLE      0x00

void cc1100_set_crc_policy(uint8_t policy);

#define CC1100_WHITEDATA_DISABLE  0x0
#define CC1100_WHITEDATA_ENABLE   0x1

void cc1100_set_data_whitening(uint8_t policy);

#define CC1100_MANCHESTER_ENABLE  0x1
#define CC1100_MANCHESTER_DISABLE 0x0

void cc1100_set_manchester(uint8_t coding);

#define CC1100_STATUS_DISABLE  0x0
#define CC1100_STATUS_ENABLE   0x1

void cc1100_set_rx_packet_status(uint8_t policy);

#endif
