
/**
 *  \file   cc1100_2500_globals.h
 *  \brief  CC1100/CC2500 global definitions
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_2500_globals.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *  Modified by Loic Lemaitre 2010
 */
#ifndef _CC1100_GLOBALS_H
#define _CC1100_GLOBALS_H

/* TODO: this must be parameterized according to bandwith and MCU speed */
#define CC1100_SYNCHRO_DELAY_THRESHOLD          5000  /* ns */

/* Preamble pattern byte (alternating sequence of 1 and 0) */
#define CC1100_PREAMBLE_PATTERN 0x55

/* CC1100 time constants (cf [1] p33) */
#define CC1100_POWER_UP_DELAY_NS                40000
#define CC1100_POWER_UP_DELAY_US                40
#define CC1100_MANCAL_DELAY_NS                  721000
#define CC1100_MANCAL_DELAY_US                  721
#define CC1100_FS_WAKEUP_DELAY_NS               44200
#define CC1100_FS_WAKEUP_DELAY_US               45
#define CC1100_SETTLING_DELAY_NS                44200
#define CC1100_SETTLING_DELAY_US                44
#define CC1100_CALIBRATE_DELAY_NS               720600
#define CC1100_CALIBRATE_DELAY_US               720
/* RC oscillator calibration time, cf AN047 p8 */
#define CC1100_RC_CALIBRATE_DELAY_NS            2000000
#define CC1100_RC_CALIBRATE_DELAY_US            2000
/* IDLE->SLEEP in WOR mode when RC calibration over or no RC calibration  (cf AN047 p9) */
#define CC1100_WOR_IDLE_SLEEP_NS                150000
#define CC1100_WOR_IDLE_SLEEP_US                150
#define CC1100_IDLE_NOCAL_DELAY_NS              100
#define CC1100_IDLE_NOCAL_DELAY_US              1
#define CC1100_TX_RX_DELAY_NS                   21500
#define CC1100_TX_RX_DELAY_US                   22
#define CC1100_RX_TX_DELAY_NS                   9600
#define CC1100_RX_TX_DELAY_US                   10


/* CC1100 State Machine in status byte (cf [1] p21) */
#define CC1100_STATUS_IDLE                      0x00
#define CC1100_STATUS_RX                        0x01
#define CC1100_STATUS_TX                        0x02
#define CC1100_STATUS_FSTXON                    0x03
#define CC1100_STATUS_CALIBRATE                 0x04
#define CC1100_STATUS_SETTLING                  0x05
#define CC1100_STATUS_RXFIFO_OVERFLOW           0x06
#define CC1100_STATUS_TXFIFO_UNDERFLOW          0x07


/* CC1100 internal states (cf[1] p30) */
#define CC1100_STATE_SLEEP                      0x00
#define CC1100_STATE_IDLE                       0x01
#define CC1100_STATE_XOFF                       0x02
#define CC1100_STATE_MANCAL                     0x03
#define CC1100_STATE_FS_WAKEUP                  0x06
#define CC1100_STATE_FS_CALIBRATE               0x08
#define CC1100_STATE_SETTLING                   0x09
#define CC1100_STATE_CALIBRATE                  0x0C
#define CC1100_STATE_RX                         0x0D
#define CC1100_STATE_TXRX_SETTLING              0x10
#define CC1100_STATE_RX_OVERFLOW                0x11
#define CC1100_STATE_FSTXON                     0x12
#define CC1100_STATE_TX                         0x13
#define CC1100_STATE_RXTX_SETTLING              0x15
#define CC1100_STATE_TX_UNDERFLOW               0x16
#define CC1100_STATE_IDLING                     0x17

#define CC1100_NO_STATE_PENDING                 -1

/* CC1100 RAM & register Access (cf [1] p18) */
#define CC1100_REG_ACCESS_OP(x)                 (x & 0x80) 
#define CC1100_REG_ACCESS_OP_READ               0x80 
#define CC1100_REG_ACCESS_OP_WRITE              0x00 
#define CC1100_REG_ACCESS_TYPE(x)               (x & 0x40) 
#define CC1100_REG_ACCESS_BURST                 0x40 
#define CC1100_REG_ACCESS_NOBURST               0x00 
#define CC1100_REG_ACCESS_ADDRESS(x)            (x & 0x3F)


/* C1100 Strobe commands (cf [1] p41) */
#define CC1100_STROBE_SRES                      0x30 /* reset                                       */
#define CC1100_STROBE_SFSTXON                   0x31 /* enable and calibrate                        */
#define CC1100_STROBE_SXOFF                     0x32 /* crystall off                                */
#define CC1100_STROBE_SCAL                      0x33 /* calibrate                                   */
#define CC1100_STROBE_SRX                       0x34 /* enable rx                                   */
#define CC1100_STROBE_STX                       0x35 /* enable tx                                   */
#define CC1100_STROBE_SIDLE                     0x36 /* go idle                                     */
#define CC1100_STROBE_SAFC                      0x37 /* AFC adjustment, removed in datasheet 1.1    */
#define CC1100_STROBE_SWOR                      0x38 /* wake on radio                               */
#define CC1100_STROBE_SPWD                      0x39 /* power down                                  */
#define CC1100_STROBE_SFRX                      0x3A /* flush Rx fifo                               */
#define CC1100_STROBE_SFTX                      0x3B /* flush Tx fifo                               */
#define CC1100_STROBE_SWORRST                   0x3C /* Reset real time clock to Event1 value.      */
#define CC1100_STROBE_SNOP                      0x3D /* no operation                                */


/* C1100 Registers (cf [1] p42) */
#define CC1100_REG_IOCFG2                       0x00
#define CC1100_REG_IOCFG2_DEFAULT               0x29
#define CC1100_REG_IOCFG1                       0x01
#define CC1100_REG_IOCFG1_DEFAULT               0x2E
#define CC1100_REG_IOCFG0                       0x02
#define CC1100_REG_IOCFG0_DEFAULT               0x3F
#define CC1100_REG_FIFOTHR                      0x03
#define CC1100_REG_FIFOTHR_DEFAULT              0x07
#define CC1100_REG_SYNC1                        0x04
#define CC1100_REG_SYNC1_DEFAULT                0xD3
#define CC1100_REG_SYNC0                        0x05
#define CC1100_REG_SYNC0_DEFAULT                0x91
#define CC1100_REG_PKTLEN                       0x06
#define CC1100_REG_PKTLEN_DEFAULT               0xFF
#define CC1100_REG_PKTCTRL1                     0x07
#define CC1100_REG_PKTCTRL1_DEFAULT             0x04
#define CC1100_REG_PKTCTRL0                     0x08
#define CC1100_REG_PKTCTRL0_DEFAULT             0x45
#define CC1100_REG_ADDR                         0x09
#define CC1100_REG_ADDR_DEFAULT                 0x00
#define CC1100_REG_CHANNR                       0x0A
#define CC1100_REG_CHANNR_DEFAULT               0x00
#define CC1100_REG_FSCTRL1                      0x0B
#define CC1100_REG_FSCTRL1_DEFAULT              0x0F
#define CC1100_REG_FSCTRL0                      0x0C
#define CC1100_REG_FSCTRL0_DEFAULT              0x00
#define CC1100_REG_FREQ2                        0x0D
#if defined(CC1100)
#define CC1100_REG_FREQ2_DEFAULT                0x1E
#elif defined(CC2500)
#define CC1100_REG_FREQ2_DEFAULT                0x5E
#else
#error "you must define CC1100 or CC2500 model"
#endif
#define CC1100_REG_FREQ1                        0x0E
#define CC1100_REG_FREQ1_DEFAULT                0xC4
#define CC1100_REG_FREQ0                        0x0F
#define CC1100_REG_FREQ0_DEFAULT                0xEC
#define CC1100_REG_MDMCFG4                      0x10
#define CC1100_REG_MDMCFG4_DEFAULT              0x8C
#define CC1100_REG_MDMCFG3                      0x11
#define CC1100_REG_MDMCFG3_DEFAULT              0x22
#define CC1100_REG_MDMCFG2                      0x12
#define CC1100_REG_MDMCFG2_DEFAULT              0x02
#define CC1100_REG_MDMCFG1                      0x13
#define CC1100_REG_MDMCFG1_DEFAULT              0x22
#define CC1100_REG_MDMCFG0                      0x14
#define CC1100_REG_MDMCFG0_DEFAULT              0xF8
#define CC1100_REG_DEVIATN                      0x15
#define CC1100_REG_DEVIATN_DEFAULT              0x47
#define CC1100_REG_MCSM2                        0x16
#define CC1100_REG_MCSM2_DEFAULT                0x07
#define CC1100_REG_MCSM1                        0x17
#define CC1100_REG_MCSM1_DEFAULT                0x30
#define CC1100_REG_MCSM0                        0x18
#define CC1100_REG_MCSM0_DEFAULT                0x04
#define CC1100_REG_FOCCFG                       0x19
#define CC1100_REG_FOCCFG_DEFAULT               0x36
#define CC1100_REG_BSCFG                        0x1A
#define CC1100_REG_BSCFG_DEFAULT                0x6C
#define CC1100_REG_AGCCTRL2                     0x1B
#define CC1100_REG_AGCCTRL2_DEFAULT             0x03
#define CC1100_REG_AGCCTRL1                     0x1C
#define CC1100_REG_AGCCTRL1_DEFAULT             0x40
#define CC1100_REG_AGCCTRL0                     0x1D
#define CC1100_REG_AGCCTRL0_DEFAULT             0x91
#define CC1100_REG_WOREVT1                      0x1E
#define CC1100_REG_WOREVT1_DEFAULT              0x87
#define CC1100_REG_WOREVT0                      0x1F
#define CC1100_REG_WOREVT0_DEFAULT              0x6B
#define CC1100_REG_WORCTRL                      0x20
#define CC1100_REG_WORCTRL_DEFAULT              0xF8
#define CC1100_REG_FREND1                       0x21
#define CC1100_REG_FREND1_DEFAULT               0x56 /* A6 in datasheet 1.0 */
#define CC1100_REG_FREND0                       0x22
#define CC1100_REG_FREND0_DEFAULT               0x10
#define CC1100_REG_FSCAL3                       0x23
#define CC1100_REG_FSCAL3_DEFAULT               0xA9
#define CC1100_REG_FSCAL2                       0x24
#define CC1100_REG_FSCAL2_DEFAULT               0x0A
#define CC1100_REG_FSCAL1                       0x25
#define CC1100_REG_FSCAL1_DEFAULT               0x20
#define CC1100_REG_FSCAL0                       0x26
#define CC1100_REG_FSCAL0_DEFAULT               0x0D
#define CC1100_REG_RCCTRL1                      0x27
#define CC1100_REG_RCCTRL1_DEFAULT              0x41
#define CC1100_REG_RCCTRL0                      0x28
#define CC1100_REG_RCCTRL0_DEFAULT              0x00


/* Configuration registers  (cf [1] p60) */
#define CC1100_REG_FSTEST                       0x29
#define CC1100_REG_FSTEST_DEFAULT               0x59 /* 57 in datasheet 1.0 */
#define CC1100_REG_PTEST                        0x2A
#define CC1100_REG_PTEST_DEFAULT                0x7F
#define CC1100_REG_AGCTEST                      0x2B
#define CC1100_REG_AGCTEST_DEFAULT              0x3F
#define CC1100_REG_TEST2                        0x2C
#define CC1100_REG_TEST2_DEFAULT                0x88
#define CC1100_REG_TEST1                        0x2D
#define CC1100_REG_TEST1_DEFAULT                0x31
#define CC1100_REG_TEST0                        0x2E
#define CC1100_REG_TEST0_DEFAULT                0x0B


/* Read only registers  (cf [1] p60) */
#define CC1100_REG_PARTNUM                      0x30
#if defined(CC1100)
#define CC1100_REG_PARTNUM_DEFAULT              0x00
#elif defined(CC2500)
#define CC1100_REG_PARTNUM_DEFAULT              0x80
#else
#error "you must define CC1100 or CC2500 model"
#endif
#define CC1100_REG_VERSION                      0x31
#define CC1100_REG_VERSION_DEFAULT              0x03  /* old version number 0x01 replaced by 0x03 according to current used hardware version */
#define CC1100_REG_FREQEST                      0x32
#define CC1100_REG_FREQEST_DEFAULT              0x00
#define CC1100_REG_LQI                          0x33
#define CC1100_REG_LQI_DEFAULT                  0x7F
#define CC1100_REG_RSSI                         0x34
#define CC1100_REG_RSSI_DEFAULT                 0x80
#define CC1100_REG_MARCSTATE                    0x35
#define CC1100_REG_MARCSTATE_DEFAULT            0x01
#define CC1100_REG_WORTIME1                     0x36
#define CC1100_REG_WORTIME1_DEFAULT             0x00
#define CC1100_REG_WORTIME0                     0x37
#define CC1100_REG_WORTIME0_DEFAULT             0x00
#define CC1100_REG_PKTSTATUS                    0x38
#define CC1100_REG_PKTSTATUS_DEFAULT            0x00
#define CC1100_REG_VCO_VC_DAC                   0x39
#define CC1100_REG_VCO_VC_DAC_DEFAULT           0x94
#define CC1100_REG_TXBYTES                      0x3A
#define CC1100_REG_TXBYTES_DEFAULT              0x00
#define CC1100_REG_RXBYTES                      0x3B
#define CC1100_REG_RXBYTES_DEFAULT              0x00


#endif
