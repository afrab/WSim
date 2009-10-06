
/**
 *  \file   cc2420_registers.h
 *  \brief  CC2420 registers
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_registers.h
 *  
 *
 *  Created by Nicolas Boulicault on 05/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_REGISTERS_H
#define _CC2420_REGISTERS_H

#include "cc2420_internals.h"


/*
 * CC2420 reset function
 */

void cc2420_reset_registers(struct _cc2420_t * cc2420);

/*
 * CC2420 write register functions
 */

void cc2420_write_register  (struct _cc2420_t * cc2420, uint8_t addr, uint16_t val);
//void cc2420_write_register_h(struct _cc2420_t * cc2420, uint8_t addr, uint8_t valh);
//void cc2420_write_register_l(struct _cc2420_t * cc2420, uint8_t addr, uint8_t vall);

/*
 * CC2420 read register functions
 */

uint16_t cc2420_read_register  (struct _cc2420_t * cc2420, uint8_t addr);
uint8_t  cc2420_read_register_h(struct _cc2420_t * cc2420, uint8_t addr);
uint8_t  cc2420_read_register_l(struct _cc2420_t * cc2420, uint8_t addr);

/*
 * CC2420 update register function
 */
void	cc2420_pll_register_update(struct _cc2420_t * cc2420);


/*
 * CC2420 registers addresses and default values
 */

#define CC2420_REG_MAIN                          0x10   //main control register
#define CC2420_REG_MAIN_DEFAULT                  0xF800
#define CC2420_REG_MDMCTRL0                      0x11   //modem control register 0
#define CC2420_REG_MDMCTRL0_DEFAULT              0x0AE2
#define CC2420_REG_MDMCTRL1                      0x12   //model control register 1
#define CC2420_REG_MDMCTRL1_DEFAULT              0x0000
#define CC2420_REG_RSSI                          0x13   //RSSI and CCA status
#define CC2420_REG_RSSI_DEFAULT                  0xE080
#define CC2420_REG_SYNCWORD                      0x14   //synchronisation word register
#define CC2420_REG_SYNCWORD_DEFAULT              0xA70F
#define CC2420_REG_TXCTRL                        0x15   //transmit control register
#define CC2420_REG_TXCTRL_DEFAULT                0xA0FF
#define CC2420_REG_RXCTRL0                       0x16   //receive control register 0
#define CC2420_REG_RXCTRL0_DEFAULT               0x12E5
#define CC2420_REG_RXCTRL1                       0x17   //receive control register 1
#define CC2420_REG_RXCTRL1_DEFAULT               0x0A56
#define CC2420_REG_FSCTRL                        0x18   //freq synthetizer control / status
#define CC2420_REG_FSCTRL_DEFAULT                0x4165
#define CC2420_REG_SECCTRL0                      0x19   //security control register 0
#define CC2420_REG_SECCTRL0_DEFAULT              0x03C4
#define CC2420_REG_SECCTRL1                      0x1A   //security control register 1
#define CC2420_REG_SECCTRL1_DEFAULT              0x0000
#define CC2420_REG_BATTMON                       0x1B   //battery monitor control register
#define CC2420_REG_BATTMON_DEFAULT               0x0000
#define CC2420_REG_IOCFG0                        0x1C   //IO configuration register 0
#define CC2420_REG_IOCFG0_DEFAULT                0x0040
#define CC2420_REG_IOCFG1                        0x1D   //IO configuration register 1
#define CC2420_REG_IOCFG1_DEFAULT                0x0000
#define CC2420_REG_MANFIDL                       0x1E   //manufacturer ID, low part
#define CC2420_REG_MANFIDL_DEFAULT               0x233D
#define CC2420_REG_MANFIDH                       0x1F   //manufacturer ID, high part
#define CC2420_REG_MANFIDH_DEFAULT               0x2000
#define CC2420_REG_FSMTC                         0x20   //finite state machine constants
#define CC2420_REG_FSMTC_DEFAULT                 0x7A94
#define CC2420_REG_MANAND                        0x21   //manual signal AND override register
#define CC2420_REG_MANAND_DEFAULT                0xFFFF
#define CC2420_REG_MANOR                         0x22   //manual signal OR override register
#define CC2420_REG_MANOR_DEFAULT                 0x0000
#define CC2420_REG_AGCCTRL                       0x23   //AGC control register
#define CC2420_REG_AGCCTRL_DEFAULT               0x07F0
#define CC2420_REG_AGCTST0                       0x24   //AGC test register 0
#define CC2420_REG_AGCTST0_DEFAULT               0x3649
#define CC2420_REG_AGCTST1                       0x25   //AGC test register 1
#define CC2420_REG_AGCTST1_DEFAULT               0x0860
#define CC2420_REG_AGCTST2                       0x26   //AGC test register 2
#define CC2420_REG_AGCTST2_DEFAULT               0x012A
#define CC2420_REG_FSTST0                        0x27   //freq synthetizer test register 0
#define CC2420_REG_FSTST0_DEFAULT                0x0200
#define CC2420_REG_FSTST1                        0x28   //freq synthetizer test register 1
#define CC2420_REG_FSTST1_DEFAULT                0x5002
#define CC2420_REG_FSTST2                        0x29   //freq synthetizer test register 2
#define CC2420_REG_FSTST2_DEFAULT                0x0600
#define CC2420_REG_FSTST3                        0x2A   //freq synthetizer test register 3
#define CC2420_REG_FSTST3_DEFAULT                0x81DD
#define CC2420_REG_RXBPFTST                      0x2B   //receiver bandpass filters test register
#define CC2420_REG_RXBPFTST_DEFAULT              0x0000
#define CC2420_REG_FSMSTATE                      0x2C   //finite state machine information
#define CC2420_REG_FSMSTATE_DEFAULT              0x0000
#define CC2420_REG_ADCTST                        0x2D   //ADC test register
#define CC2420_REG_ADCTST_DEFAULT                0x0000
#define CC2420_REG_DACTST                        0x2E   //DAC test register
#define CC2420_REG_DACTST_DEFAULT                0x0000
#define CC2420_REG_TOPTST                        0x2F   //top level test register
#define CC2420_REG_TOPTST_DEFAULT                0x0010
#define CC2420_REG_RESERVED                      0x30   //reserved, contains spare control and status bits
#define CC2420_REG_RESERVED_DEFAULT              0x0000
#define CC2420_REG_TXFIFO                        0x3E   //transmit FIFO byte register
#define CC2420_REG_TXFIFO_DEFAULT                0x0000
#define CC2420_REG_RXFIFO                        0x3F   //receive FIFO byte register
#define CC2420_REG_RXFIFO_DEFAULT                0x0000


/*
 * CC2420 configuration registers bits
 */

/* CC2420 Main Control Register (cf [1] p64) */
#define CC2420_REG_MAIN_RESETn(x) (x & 0x8000)
#define CC2420_REG_MAIN_ENC_RESETn(x) (x & 0x4000)
#define CC2420_REG_MAIN_DEMOD_RESETn(x) (x & 0x2000)
#define CC2420_REG_MAIN_MOD_RESETn(x) (x & 0x1000)
#define CC2420_REG_MAIN_FS_RESETn(x) (x & 0x0800)
#define CC2420_REG_MAIN_XOSC16M_BYPASS(x) (x & 0x0001)

/* CC2420 Modem Control Register 0 (cf [1] p65) */
#define CC2420_REG_MDMCTRL0_RESERVED_FRAME_MODE(x) (x & 0x2000)
#define CC2420_REG_MDMCTRL0_PAN_COORDINATOR(x) (x & 0x1000)
#define CC2420_REG_MDMCTRL0_ADR_DECODE(x) (x & 0x0800)
#define CC2420_REG_MDMCTRL0_CCA_HYST(x) (x & (0x0400 | 0x0200 | 0x0100))
#define CC2420_REG_MDMCTRL0_CCA_MODE(x) (x & (0x0080 | 0x0040))
#define CC2420_REG_MDMCTRL0_AUTOCRC(x) (x & 0x0020)
#define CC2420_REG_MDMCTRL0_AUTOACK(x) (x & 0x0010)
#define CC2420_REG_MDMCTRL0_PREAMBLE_LENGTH(x) (x & (0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Modem Control Register 1 (cf [1] p66) */
#define CC2420_REG_MDMCTRL1_CDDR_THR(x) (x & (0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0040))
#define CC2420_REG_MDMCTRL1_DEMOD_AVG_MODE(x) (x & 0x0020)
#define CC2420_REG_MDMCTRL1_MODULATION_MODE(x) (x & 0x0010)
#define CC2420_REG_MDMCTRL1_TX_MODE(x) (x & (0x0008 | 0x0004))
#define CC2420_REG_MDMCTRL1_RX_MODE(x) (x & (0x0002 | 0x0001))

/* CC2420 RSSI & CCA Register 1 (cf [1] p66) */
#define CC2420_REG_RSSI_CCA_THR(x) (x & (0x8000 | 0x4000 | 0x2000 | 0x1000 | 0x0800 | 0x0400 | 0x0200 | 0x0100))
#define CC2420_REG_RSSI_RSSI_VAL(x) (x & (0x0080 | 0x0040 | 0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))
/* RSSI offset value (ie RSSI_VAL = P - RSSI_OFFSET) */
#define CC2420_RSSI_OFFSET (-45)

/* CC2420 CRC OK, cf fig 21 p39 */
#define CC2420_CRC_OK 0x80

/* CC2420 Sync Word Register (cf [1] p67) */
#define CC2420_REG_SYNC_WORD_SYNCWORD(x) (x & 0xFFFF)

/* CC2420 Transmit Control Register (cf [1] p67) */
#define CC2420_REG_TXCTRL_TXMIXBUF_CUR(x) (x & (0x8000 | 0x4000))
#define CC2420_REG_TXCTRL_TX_TURNAROUND(x) (x & 0x2000)
#define CC2420_REG_TXCTRL_TXMIX_CAP_ARRAY(x) (x & (0x1000 | 0x0800))
#define CC2420_REG_TXCTRL_TXMIX_CURRENT(x) (x & (0x0400 | 0x0200))
#define CC2420_REG_TXCTRL_PA_CURRENT(x) (x & (0x0100 | 0x0080 | 0x0040))
#define CC2420_REG_TXCTRL_PA_LEVEL(x) (x & (0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Receive Control 0 Register (cf [1] p68) */
#define CC2420_REG_RXCTRL0_RXMIXBUF_CUR(x) (x & (0x2000 | 0x1000))
#define CC2420_REG_RXCTRL0_HIGH_LNA_GAIN(x) (x & (0x0800 | 0x0400))
#define CC2420_REG_RXCTRL0_MED_LNA_GAIN(x) (x & (0x0200 | 0x0100))
#define CC2420_REG_RXCTRL0_LOW_LNA_GAIN(x) (x & (0x0080 | 0x0040))
#define CC2420_REG_RXCTRL0_HIGH_LNA_CURRENT(x) (x & (0x0020 | 0x0010))
#define CC2420_REG_RXCTRL0_MED_LNA_CURRENT(x) (x & (0x0008 | 0x0004))
#define CC2420_REG_RXCTRL0_LOW_LNA_CURRENT(x) (x & (0x0002 | 0x0001))

/* CC2420 Receive Control 1 Register (cf [1] p69) */
#define CC2420_REG_RXCTRL1_RXBPF_LOCUR(x) (x & 0x2000)
#define CC2420_REG_RXCTRL1_RXBPF_MIDCUR(x) (x & 0x1000)
#define CC2420_REG_RXCTRL1_LOW_LOWGAIN(x) (x & 0x0800)
#define CC2420_REG_RXCTRL1_MED_LOWGAIN(x) (x & 0x0400)
#define CC2420_REG_RXCTRL1_HIGH_HGM(x) (x & 0x0200)
#define CC2420_REG_RXCTRL1_MED_HGM(x) (x & 0x0100)
#define CC2420_REG_RXCTRL1_LNA_CAP_ARRAY(x) (x & (0x0080 | 0x0040))
#define CC2420_REG_RXCTRL1_RXMIX_TAIL(x) (x & (0x0020 | 0x0010))
#define CC2420_REG_RXCTRL1_RXMIX_VCM(x) (x & (0x0008 | 0x0004))
#define CC2420_REG_RXCTRL1_RXMIX_CURRENT(x) (x & (0x0002 | 0x0001))

/* CC2420 Frequency Synthesizer Control Register (cf [1] p70) */
#define CC2420_REG_FSCTRL_LOCK_THR(x) (x & (0x8000 | 0x4000))
#define CC2420_REG_FSCTRL_CAL_DONE(x) (x & (0x2000))
#define CC2420_REG_FSCTRL_CAL_RUNNING(x) (x & (0x1000))
#define CC2420_REG_FSCTRL_LOCK_LENGTH(x) (x & (0x0800))
#define CC2420_REG_FSCTRL_LOCK_STATUS(x) (x & (0x0400))
#define CC2420_REG_FSCTRL_FREQ(x) (x & (0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))
#define CC2420_SET_REG_FSCTRL_CAL_DONE    (1 << 13)
#define CC2420_SET_REG_FSCTRL_CAL_RUNNING (1 << 12)
#define CC2420_SET_REG_FSCTRL_LOCK_STATUS (1 << 10)

/* CC2420 Security Control 0 Register (cf [1] p71) */
#define CC2420_REG_SECCTRL0_RXFIFO_PROTECTION(x) (x & (0x0200))
#define CC2420_REG_SECCTRL0_SEC_CBC_HEAD(x) (x & (0x0100))
#define CC2420_REG_SECCTRL0_SEC_SAKEYSEL(x) (x & (0x0080))
#define CC2420_REG_SECCTRL0_SEC_TXKEYSEL(x) (x & (0x0040))
#define CC2420_REG_SECCTRL0_SEC_RXKEYSEL(x) (x & (0x0020))
#define CC2420_REG_SECCTRL0_SEC_M(x) (x & (0x0010 | 0x0008 | 0x0004))
#define CC2420_REG_SECCTRL0_SEC_MODE(x) (x & (0x0002 | 0x0001))

/* CC2420 Security Control 1 Register (cf [1] p72) */
#define CC2420_REG_SECCTRL0_SEC_TXL(x) (x & (0x4000 | 0x2000 | 0x1000 | 0x0800 | 0x0400 | 0x0200 | 0x0100))
#define CC2420_REG_SECCTRL0_SEC_RXL(x) (x & (0x0040 | 0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Battery Monitor Control Register (cf [1] p72) */
#define CC2420_REG_BATTMON_BATTMON_OK(x) (x & (0x0040))
#define CC2420_REG_BATTMON_BATTMON_EN(x) (x & (0x0020))
#define CC2420_REG_BATTMON_BATTMON_VOL(x) (x & (0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 I/O Configuration 0 Register (cf [1] p73) */
#define CC2420_REG_IOCFG0_BCN_ACCEPT(x) (x & (0x0800))
#define CC2420_REG_IOCFG0_FIFO_POLARITY(x) (x & (0x0400))
#define CC2420_REG_IOCFG0_FIFOP_POLARITY(x) (x & (0x0200))
#define CC2420_REG_IOCFG0_SFD_POLARITY(x) (x & (0x0100))
#define CC2420_REG_IOCFG0_CCA_POLARITY(x) (x & (0x0080))
#define CC2420_REG_IOCFG0_FIFOP_THR(x) (x & (0x0040 | 0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 I/O Configuration 1 Register (cf [1] p73) */
#define CC2420_REG_IOCFG1_HSSD_SRC(x) (x & (0x1000 | 0x0800 | 0x0400))
#define CC2420_REG_IOCFG1_SFIMUX(x) (x & (0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020))
#define CC2420_REG_IOCFG1_CCAMUX(x) (x & (0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Manufacturer ID Low Register (cf [1] p73) */
#define CC2420_REG_MANFIDL_PARTNUM(x) (x & (0x8000 | 0x4000 | 0x2000 | 0x1000))
#define CC2420_REG_MANFIDL_MANFID(x) (x & (0x0800 | 0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Manufacturer ID High Register (cf [1] p74) */
#define CC2420_REG_MANFIDH_VERSION(x) (x & (0x8000 | 0x4000 | 0x2000 | 0x1000))
#define CC2420_REG_MANFIDH_PARTNUM(x) (x & (0x0800 | 0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Finite State Machine Time Constant Register (cf [1] p74) */
#define CC2420_REG_FSMTC_RXCHAIN2RX(x) (x & (0x8000 | 0x4000 | 0x2000))
#define CC2420_REG_FSMTC_SWITCH2TX(x) (x & (0x1000 | 0x0800 | 0x0400))
#define CC2420_REG_FSMTC_PAON2TX(x) (x & (0x0200 | 0x0100 | 0x0080 | 0x0040))
#define CC2420_REG_FSMTC_TXEND2SWITCH(x) (x & (0x0020 | 0x0010 | 0x0008))
#define CC2420_REG_FSMTC_TXEND2PAOFF(x) (x & (0x0004 | 0x0002 | 0x0001))

/* CC2420 Manual signal AND Override Register (cf [1] p75) */
#define CC2420_REG_MANAND_VGA_RESET_N(x) (x & (0x8000))
#define CC2420_REG_MANAND_BIAS_PD(x) (x & (0x4000))
#define CC2420_REG_MANAND_BALUN_CTRL(x) (x & (0x2000))
#define CC2420_REG_MANAND_RXTX(x) (x & (0x1000))
#define CC2420_REG_MANAND_PRE_PD(x) (x & (0x0800))
#define CC2420_REG_MANAND_PA_N_PD(x) (x & (0x0400))
#define CC2420_REG_MANAND_PA_P_PD(x) (x & (0x0200))
#define CC2420_REG_MANAND_DAC_LPF_PD(x) (x & (0x0100))
#define CC2420_REG_MANAND_XOSC16M_PD(x) (x & (0x0080))
#define CC2420_REG_MANAND_RXBPF_CAL_PD(x) (x & (0x0040))
#define CC2420_REG_MANAND_CHP_PD(x) (x & (0x0020))
#define CC2420_REG_MANAND_FS_PD(x) (x & (0x0010))
#define CC2420_REG_MANAND_ADC_PD(x) (x & (0x0008))
#define CC2420_REG_MANAND_VGA_PD(x) (x & (0x0004))
#define CC2420_REG_MANAND_RXBPF_PD(x) (x & (0x0002))
#define CC2420_REG_MANAND_LNAMIX_PD(x) (x & (0x0001))

/* CC2420 Manual signal OR Override Register (cf [1] p76) */
#define CC2420_REG_MANOR_VGA_RESET_N(x) (x & (0x8000))
#define CC2420_REG_MANOR_BIAS_PD(x) (x & (0x4000))
#define CC2420_REG_MANOR_BALUN_CTRL(x) (x & (0x2000))
#define CC2420_REG_MANOR_RXTX(x) (x & (0x1000))
#define CC2420_REG_MANOR_PRE_PD(x) (x & (0x0800))
#define CC2420_REG_MANOR_PA_N_PD(x) (x & (0x0400))
#define CC2420_REG_MANOR_PA_P_PD(x) (x & (0x0200))
#define CC2420_REG_MANOR_DAC_LPF_PD(x) (x & (0x0100))
#define CC2420_REG_MANOR_XOSC16M_PD(x) (x & (0x0080))
#define CC2420_REG_MANOR_RXBPF_CAL_PD(x) (x & (0x0040))
#define CC2420_REG_MANOR_CHP_PD(x) (x & (0x0020))
#define CC2420_REG_MANOR_FS_PD(x) (x & (0x0010))
#define CC2420_REG_MANOR_ADC_PD(x) (x & (0x0008))
#define CC2420_REG_MANOR_VGA_PD(x) (x & (0x0004))
#define CC2420_REG_MANOR_RXBPF_PD(x) (x & (0x0002))
#define CC2420_REG_MANOR_LNAMIX_PD(x) (x & (0x0001))

/* CC2420 AGC Control Register (cf [1] p76) */
#define CC2420_REG_AGCCTRL_VGA_GAIN_OE(x) (x & (0x0800))
#define CC2420_REG_AGCCTRL_VGA_GAIN(x) (x & (0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020 | 0x0010))
#define CC2420_REG_AGCCTRL_LNAMIX_GAINMODE_0(x) (x & (0x0008 | 0x0004))
#define CC2420_REG_AGCCTRL_LNAMIX_GAINMODE(x) (x & (0x0002 | 0x0001))

/* CC2420 AGC Test 0 Register (cf [1] p76) */
#define CC2420_REG_AGTST0_LNAMIX_HYST(x) (x & ( 0x8000 | 0x4000 | 0x2000 | 0x1000))
#define CC2420_REG_AGTST0_LNAMIX_THR_H(x) (x & (0x0800 | 0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0040))
#define CC2420_REG_AGTST0_LNAMIX_THR_L(x) (x & (0x0020| 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 AGC Test 1 Register (cf [1] p77) */
#define CC2420_REG_AGTST1_AGC_BLANK_MODE(x) (x & (0x4000))
#define CC2420_REG_AGTST1_PEAKDET_CUR_BOOST(x) (x & (0x2000))
#define CC2420_REG_AGTST1_AGC_SETTLE_WAIT(x) (x & (0x1000| 0x0800))
#define CC2420_REG_AGTST1_AGC_PEAK_DET_NODE(x) (x & (0x0400 | 0x0200 | 0x0100))
#define CC2420_REG_AGTST1_AGC_WIN_SIZE(x) (x & (0x0080 | 0x0040))
#define CC2420_REG_AGTST1_AGC_REF(x) (x & (0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 AGC Test 2 Register (cf [1] p77) */
#define CC2420_REG_AGTST2_LOW2MEDGAIN(x) (x & (0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020))
/* #define CC2420_REG_AGTST2_LOW2MEDGAIN(x) (x & (0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001)) */

/* CC2420 Frequency Synthsizer Test 0 Register (cf [1] p78) */
#define CC2420_REG_FSTST0_VCO_ARRAY_SETTLE_LONG(x) (x & (0x0800))
#define CC2420_REG_FSTST0_VCO_ARRAY_OE(x) (x & (0x0400))
#define CC2420_REG_FSTST0_VCO_ARRAY_O(x) (x & (0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020))
#define CC2420_REG_FSTST0_VCO_ARRAY_RES(x) (x & (0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))


/* CC2420 Frequency Synthsizer Test 1 Register (cf [1] p78) */
#define CC2420_REG_FSTST1_VCO_TX_NOCAL(x) (x & (0x8000))
#define CC2420_REG_FSTST1_VCO_ARRAY_CAL_LONG(x) (x & (0x4000))
#define CC2420_REG_FSTST1_VCO_CURRENT_REF(x) (x & (0x2000 | 0x1000 | 0x0800 | 0x0400))
#define CC2420_REG_FSTST1_VCO_CURRENT_K(x) (x & (0x0200 | 0x0100 | 0x0080 | 0x0040 | 0x0020 | 0x0010))
#define CC2420_REG_FSTST1_VCA_DAC_EN(x) (x & (0x0008))
#define CC2420_REG_FSTST1_VCA_DAC_VAL(x) (x & (0x0004 | 0x0002 | 0x0001))

/* CC2420 Frequency Synthsizer Test 2 Register (cf [1] p78) */
#define CC2420_REG_FSTST2_VCO_CURCAL_SPEED(x) (x & (0x4000 | 0x2000))
#define CC2420_REG_FSTST2_VCO_CURRENT_OE(x) (x & (0x1000))
#define CC2420_REG_FSTST2_VCO_CURRENT(x) (x & (0x0800 | 0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0040))
#define CC2420_REG_FSTST2_VCO_CURRENT_RES(x) (x & (0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Frequency Synthsizer Test 3 Register (cf [1] p79) */
#define CC2420_REG_FSTST3_CHP_CAL_DISABLE(x) (x & (0x8000))
#define CC2420_REG_FSTST3_CHP_CURRENT_OE(x) (x & (0x4000))
#define CC2420_REG_FSTST3_CHP_TEST_UP(x) (x & (0x2000))
#define CC2420_REG_FSTST3_CHP_TEST_DN(x) (x & (0x1000))
#define CC2420_REG_FSTST3_CHP_DISABLE(x) (x & (0x0800))
#define CC2420_REG_FSTST3_PD_DELAY(x) (x & (0x0400))
#define CC2420_REG_FSTST3_CHP_STEP_PERIOD(x) (x & (0x0200 | 0x0100))
#define CC2420_REG_FSTST3_STOP_CHP_CURRENT(x) (x & (0x0080 | 0x0040 | 0x0020 | 0x0010))
#define CC2420_REG_FSTST3_START_CHP_CURRENT(x) (x & (0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Receiver Bandpass Filters Test Register (cf [1] p79) */
#define CC2420_REG_RXBPFTST_RXBPF_CAP_OE(x) (x & (0x1000))
#define CC2420_REG_RXBPFTST_RXBPF_CAP_O(x) (x & (0x0800 | 0x0400 | 0x0200 | 0x01000 | 0x0080 | 0x0040))
#define CC2420_REG_RXBPFTST_RXBPF_CAP_RES(x) (x & (0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Finite State Machine Information (cf [1] p79) */
#define CC2420_REG_FSMSTATE_FSM_CUR_STATE(x) (x & (0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 ADC Test Register (cf [1] p80) */
#define CC2420_REG_ADCTST_ADC_CLOCK_DISABLE(x) (x & (0x8000))
#define CC2420_REG_ADCTST_ADC_I(x) (x & (0x4000 |0x2000 | 0x1000 | 0x0800 | 0x0400 | 0x0200 | 0x0100))
#define CC2420_REG_ADCTST_ADC_Q(x) (x & (0x0040 |0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 DAC Test Register (cf [1] p80) */
#define CC2420_REG_DACTST_DAC_SRC(x) (x & (0x4000 | 0x2000 | 0x1000))
#define CC2420_REG_DACTST_DAC_I_O(x) (x & (0x0800 | 0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0040))
#define CC2420_REG_DACTST_DAC_Q_O(x) (x & (0x0020 | 0x0010 | 0x0008 | 0x0004 | 0x0002 | 0x0001))

/* CC2420 Top Level Test Register (cf [1] p81) */
#define CC2420_REG_TOPTST_RAM_BIST_RUN(x) (x & (0x0080))
#define CC2420_REG_TOPTST_TEST_BATTMON_EN(x) (x & (0x0040))
#define CC2420_REG_TOPTST_VC_IN_TEST_EN(x) (x & (0x0020))
#define CC2420_REG_TOPTST_ATESTMOD_PD(x) (x & (0x0010))
#define CC2420_REG_TOPTST_ATESTMOD_MODE(x) (x & (0x0008 | 0x0004 | 0x0002 | 0x0001))



#endif
