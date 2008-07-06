
/**
 *  \file   cc1100_registers.c
 *  \brief  CC1100 registers
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100->registers.c
 *  
 *
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */

#include "cc1100_internals.h"


/***************************************************/
/***************************************************/
/***************************************************/
uint8_t cc1100_read_ro_register				(struct _cc1100_t *cc1100, uint8_t addr);
uint8_t cc1100_compute_pktstatus_register	(struct _cc1100_t *cc1100);


/***************************************************/
/***************************************************/
/***************************************************/
void cc1100_reset_registers(struct _cc1100_t *cc1100) {
	
	cc1100->registers[CC1100_REG_IOCFG2] = CC1100_REG_IOCFG2_DEFAULT;
	cc1100->registers[CC1100_REG_IOCFG1] = CC1100_REG_IOCFG1_DEFAULT;
	cc1100->registers[CC1100_REG_IOCFG0] = CC1100_REG_IOCFG0_DEFAULT;
	cc1100->registers[CC1100_REG_FIFOTHR] = CC1100_REG_FIFOTHR_DEFAULT;
	cc1100->registers[CC1100_REG_SYNC1] = CC1100_REG_SYNC1_DEFAULT;
	cc1100->registers[CC1100_REG_SYNC0] = CC1100_REG_SYNC0_DEFAULT;
	cc1100->registers[CC1100_REG_PKTLEN] = CC1100_REG_PKTLEN_DEFAULT;
	cc1100->registers[CC1100_REG_PKTCTRL1] = CC1100_REG_PKTCTRL1_DEFAULT;
	cc1100->registers[CC1100_REG_PKTCTRL0] = CC1100_REG_PKTCTRL0_DEFAULT;
	cc1100->registers[CC1100_REG_ADDR] = CC1100_REG_ADDR_DEFAULT;
	cc1100->registers[CC1100_REG_CHANNR] = CC1100_REG_CHANNR_DEFAULT;
	cc1100->registers[CC1100_REG_FSCTRL1] = CC1100_REG_FSCTRL1_DEFAULT;
	cc1100->registers[CC1100_REG_FSCTRL0] = CC1100_REG_FSCTRL0_DEFAULT;
	cc1100->registers[CC1100_REG_FREQ2] = CC1100_REG_FREQ2_DEFAULT;
	cc1100->registers[CC1100_REG_FREQ1] = CC1100_REG_FREQ1_DEFAULT;
	cc1100->registers[CC1100_REG_FREQ0] = CC1100_REG_FREQ0_DEFAULT;
	CC1100_UNCALIBRATE(cc1100);
	cc1100->registers[CC1100_REG_MDMCFG4] = CC1100_REG_MDMCFG4_DEFAULT;
	cc1100->registers[CC1100_REG_MDMCFG3] = CC1100_REG_MDMCFG3_DEFAULT;
	cc1100->registers[CC1100_REG_MDMCFG2] = CC1100_REG_MDMCFG2_DEFAULT;
	cc1100->registers[CC1100_REG_MDMCFG1] = CC1100_REG_MDMCFG1_DEFAULT;
	cc1100->registers[CC1100_REG_MDMCFG0] = CC1100_REG_MDMCFG0_DEFAULT;
	cc1100->registers[CC1100_REG_DEVIATN] = CC1100_REG_DEVIATN_DEFAULT;
	cc1100->registers[CC1100_REG_MCSM2] = CC1100_REG_MCSM2_DEFAULT;
	cc1100->registers[CC1100_REG_MCSM1] = CC1100_REG_MCSM1_DEFAULT;
	cc1100->registers[CC1100_REG_MCSM0] = CC1100_REG_MCSM0_DEFAULT;
	cc1100->registers[CC1100_REG_FOCCFG] = CC1100_REG_FOCCFG_DEFAULT;
	cc1100->registers[CC1100_REG_BSCFG] = CC1100_REG_BSCFG_DEFAULT;
	cc1100->registers[CC1100_REG_AGCCTRL2] = CC1100_REG_AGCCTRL2_DEFAULT;
	cc1100->registers[CC1100_REG_AGCCTRL1] = CC1100_REG_AGCCTRL1_DEFAULT;
	cc1100->registers[CC1100_REG_AGCCTRL0] = CC1100_REG_AGCCTRL0_DEFAULT;
	cc1100->registers[CC1100_REG_WOREVT1] = CC1100_REG_WOREVT1_DEFAULT;
	cc1100->registers[CC1100_REG_WOREVT0] = CC1100_REG_WOREVT0_DEFAULT;
	cc1100->registers[CC1100_REG_WORCTRL] = CC1100_REG_WORCTRL_DEFAULT;
	cc1100->registers[CC1100_REG_FREND1] = CC1100_REG_FREND1_DEFAULT;
	cc1100->registers[CC1100_REG_FREND0] = CC1100_REG_FREND0_DEFAULT;
	cc1100->registers[CC1100_REG_FSCAL3] = CC1100_REG_FSCAL3_DEFAULT;
	cc1100->registers[CC1100_REG_FSCAL2] = CC1100_REG_FSCAL2_DEFAULT;
	cc1100->registers[CC1100_REG_FSCAL1] = CC1100_REG_FSCAL1_DEFAULT;
	cc1100->registers[CC1100_REG_FSCAL0] = CC1100_REG_FSCAL0_DEFAULT;
	cc1100->registers[CC1100_REG_RCCTRL1] = CC1100_REG_RCCTRL1_DEFAULT;
	cc1100->registers[CC1100_REG_RCCTRL1] = CC1100_REG_RCCTRL1_DEFAULT;
	
	/* Configuration registers  (cf [1] p60) */
	cc1100->registers[CC1100_REG_FSTEST] = CC1100_REG_FSTEST_DEFAULT;
	cc1100->registers[CC1100_REG_PTEST] = CC1100_REG_PTEST_DEFAULT;
	cc1100->registers[CC1100_REG_AGCTEST] = CC1100_REG_AGCTEST_DEFAULT;
	cc1100->registers[CC1100_REG_TEST2] = CC1100_REG_TEST2_DEFAULT;
	cc1100->registers[CC1100_REG_TEST1] = CC1100_REG_TEST1_DEFAULT;
	cc1100->registers[CC1100_REG_TEST0] = CC1100_REG_TEST0_DEFAULT;
	
	/* PA Table */
	cc1100->patable[0] = 0xC6;
	cc1100->patable[1] = 0;
	cc1100->patable[2] = 0;
	cc1100->patable[3] = 0;
	cc1100->patable[4] = 0;
	cc1100->patable[5] = 0;
	cc1100->patable[6] = 0;
	cc1100->patable[7] = 0;
	
	/* Read only registers  (cf [1] p60) */
	cc1100->registers[CC1100_REG_PARTNUM] = CC1100_REG_PARTNUM_DEFAULT;
	cc1100->registers[CC1100_REG_VERSION] = CC1100_REG_VERSION_DEFAULT;
	cc1100->registers[CC1100_REG_FREQEST] = CC1100_REG_FREQEST_DEFAULT;
	cc1100->registers[CC1100_REG_LQI] = CC1100_REG_LQI_DEFAULT; 
	cc1100->registers[CC1100_REG_RSSI] = CC1100_REG_RSSI_DEFAULT;
	cc1100->registers[CC1100_REG_MARCSTATE] = CC1100_REG_MARCSTATE_DEFAULT;
	cc1100->registers[CC1100_REG_WORTIME1] = CC1100_REG_WORTIME1_DEFAULT;
	cc1100->registers[CC1100_REG_WORTIME0] = CC1100_REG_WORTIME0_DEFAULT;
	cc1100->registers[CC1100_REG_PKTSTATUS] = CC1100_REG_PKTSTATUS_DEFAULT;
	cc1100->registers[CC1100_REG_VCO_VC_DAC] = CC1100_REG_VCO_VC_DAC_DEFAULT; 
	cc1100->registers[CC1100_REG_TXBYTES] = CC1100_REG_TXBYTES_DEFAULT;
	cc1100->registers[CC1100_REG_RXBYTES] = CC1100_REG_RXBYTES_DEFAULT;
	
	return;
}	


/***************************************************/
/***************************************************/
/***************************************************/
void cc1100_write_register(struct _cc1100_t *cc1100, uint8_t addr, uint8_t val) {
	uint8_t old_val;
	
	if (addr >= 0x30)
		return;
	
	old_val = cc1100->registers[addr];
	cc1100->registers[addr] = val;
	
	if ((addr >= CC1100_REG_FREQ2) && (addr <= CC1100_REG_FREQ0) && (val != old_val)) {
		/* Need to calibrate if the frequency registers are modified */
		CC1100_UNCALIBRATE(cc1100);
	} else if ((addr <= CC1100_REG_IOCFG0) && (val != old_val)) {
		/* Need to update pins if modified */
		cc1100_update_gdo(cc1100, val);
	}		
	
	CC1100_DBG_REG("cc1100: (register DEBUG): register 0x%x written with 0x%x\n", addr, val);		
	return;
}


/***************************************************/
/***************************************************/
/***************************************************/
uint8_t cc1100_read_register(struct _cc1100_t *cc1100, uint8_t addr) {

	if (addr >= 0x30) {
		return cc1100_read_ro_register(cc1100, addr);
	} else {
		CC1100_DBG_REG("cc1100: (register DEBUG): register 0x%x read 0x%x\n", addr, cc1100->registers[addr]);		
		return cc1100->registers[addr];
	}
}

/***************************************************/
/***************************************************/
/***************************************************/
uint8_t cc1100_read_ro_register(struct _cc1100_t *cc1100, uint8_t addr) {
	uint8_t val;
	
	switch (addr) {
		case CC1100_REG_PARTNUM:
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_VERSION:
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_FREQEST:
			CC1100_DBG_IMPL("cc1100: (register IMPLEMENTATION): read-only register 0x%x not implemented yet\n", addr);		
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_LQI:
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_RSSI:
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_MARCSTATE:
			val = cc1100->fsm_state;
			break;
		case CC1100_REG_WORTIME1:
			CC1100_DBG_IMPL("cc1100: (register IMPLEMENTATION): read-only register 0x%x not implemented yet\n", addr);		
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_WORTIME0:
			CC1100_DBG_IMPL("cc1100: (register IMPLEMENTATION): read-only register 0x%x not implemented yet\n", addr);		
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_PKTSTATUS:
			val = cc1100_compute_pktstatus_register(cc1100);
			break;
		case CC1100_REG_VCO_VC_DAC:
			val = cc1100->registers[addr];
			break;
		case CC1100_REG_TXBYTES:
			if (cc1100->txUnderflow)
				val = 0x80;
			else
				val = cc1100->txBytes;
			break;
		case CC1100_REG_RXBYTES:
			if (cc1100->rxOverflow)
				val = 0x80;
			else
				val = cc1100->rxBytes;
			break;
		default: 
			CC1100_DBG_EXC("cc1100: (register EXCEPTION): unknown register address 0x%x \n", addr);		
			val = 0x00;
			break;
	}	

	CC1100_DBG_REG("cc1100: (register DEBUG): register 0x%x read 0x%x\n", addr, cc1100->registers[addr]);		
	return val;
}


/***************************************************/
/***************************************************/
/***************************************************/
uint8_t cc1100_compute_pktstatus_register(struct _cc1100_t *cc1100) {

	/* CRC value */
	cc1100->registers[CC1100_REG_PKTSTATUS] = cc1100->registers[CC1100_REG_LQI] & 0x80;
	
	/* CS */
	if (CC1100_GET_CS(cc1100)) {
		cc1100->registers[CC1100_REG_PKTSTATUS] |= 0x40;
	}
	
	/* PQT */
	if (CC1100_GET_PQT(cc1100)) {
		cc1100->registers[CC1100_REG_PKTSTATUS] |= 0x20;
	}
	
	/* CCA */
	CC1100_COMPUTE_CCA(cc1100);
	
	/* Sync word found */
	if ((cc1100->fsm_state == CC1100_STATE_RX) 
		&& ((cc1100->fsm_ustate == 3) || (cc1100->fsm_ustate == 4))) {
		cc1100->registers[CC1100_REG_PKTSTATUS] |= 0x08;
	}
	
	return cc1100->registers[CC1100_REG_PKTSTATUS];
}

