
/**
 *  \file   cc1100_internals.h
 *  \brief  CC1100 internal definitions and structures
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_internals.h
 *  
 *
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *  Modified by Antoine Fraboulet, 2007
 *
 */

#ifndef _CC1100_INTERNALS_H
#define _CC1100_INTERNALS_H

#include "libwsnet/libwsnet.h"
#include "machine/machine.h"

#include "cc1100_globals.h"
#include "cc1100_dev.h"
#include "cc1100_debug.h"
#include "cc1100_macros.h"

/***************************************************/
/***************************************************/
/***************************************************/

/* Private cc1100 time constants in MHz and ns (cf [1] p9) */
// #define CC1100_INTERNAL_XOSC_FREQ         26 /* MHz      */
// #define CC1100_INTERNAL_XOSC_PERIOD       38 /* 1/26 MHz */

extern int CC1100_XOSC_FREQ_MHz;
extern int CC1100_XOSC_PERIOD_NS;

/***************************************************/
/***************************************************/
/***************************************************/

/* Private states */
#define CC1100_STATE_POWER_DOWN		0x24
#define CC1100_STATE_POWER_UP		0x25

/***************************************************/
/***************************************************/
/***************************************************/

/* Private type of next operation */
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_ADDR                   0
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGREAD_BURST          1
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGREAD_NOBURST        2
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGWRITE_BURST         3
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_REGWRITE_NOBURST       4
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_TXFIFO_BURST           5
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_TXFIFO_NOBURST         6
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_RXFIFO_BURST           7
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_RXFIFO_NOBURST         8
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_READ_BURST     9
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_READ_NOBURST  10
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_WRITE_BURST   11
#define CC1100_INTERNAL_NEXT_ACCESS_TYPE_PATABLE_WRITE_NOBURST 12

/***************************************************/
/***************************************************/
/***************************************************/

/* Private lengths of internal FIFOs */
#define CC1100_TXFIFO_LENGTH              64
#define CC1100_RXFIFO_LENGTH              64

/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_PIN_ASSERT                  1
#define CC1100_PIN_DEASSERT                0

/***************************************************/
/***************************************************/
/***************************************************/

struct _cc1100_t {
	
	uint8_t		fsm_state;
	int             fsm_ustate;
	uint8_t		fsm_pending;
	uint64_t	fsm_timer;

	int	        fs_cal;
	
	uint64_t	clk_timeref;
	uint64_t	clk_tick;
	
	uint64_t	tx_io_timer;
	uint64_t	rx_io_timer;

	uint8_t		CSn_pin;
	uint8_t		SI_pin;
	uint8_t		GO0_pin;
	uint8_t		GO1_pin;
	uint8_t		GO2_pin;
	int		SI_set;
	int		GO0_set;
	int		GO1_set;
	int		GO2_set;
	
	uint8_t		SIType;
	uint8_t		addr;
	uint8_t		read;
	
	uint8_t		txfifo[CC1100_TXFIFO_LENGTH];
	int		txBytes;
	int		txOffset;
	int		txUnderflow;
	
	uint8_t		rxfifo[CC1100_RXFIFO_LENGTH];
	int		rxBytes;
	int		rxOffset;
	int		rxOverflow;
	
	int		ioOffset;
	int		ioLength;
	uint16_t	ioCrc;

	double		lqi;
	uint8_t		lqi_cnt;
	int		pqt;
	uint8_t		addressChk;

	uint8_t		registers[0x3F];

	uint8_t		patable[8];
	uint8_t		patable_cnt;
};


/***************************************
* Device functions (cc1100_dev.c)
***************************************/
void	 cc1100_write_pin		(struct _cc1100_t *cc1100, uint8_t pin, uint8_t val);
uint8_t  cc1100_read_pin                (struct _cc1100_t *cc1100, uint8_t pin);
int	 cc1100_power_up                (int dev_num);
int	 cc1100_power_down		(int dev_num);
int	 cc1100_delete		        (int dev_num);
int	 cc1100_update			(int dev_num);
int	 cc1100_reset                   (int dev_num);
void	 cc1100_reset_internal          (struct _cc1100_t *cc1100);
int	 cc1100_io_pins	                (struct _cc1100_t *cc1100);
void	 cc1100_write_status		(struct _cc1100_t *cc1100);


/***************************************
* Register functions (cc1100_registers.c)
***************************************/
void	 cc1100_reset_registers	        (struct _cc1100_t *cc1100);
void	 cc1100_write_register	        (struct _cc1100_t *cc1100, uint8_t addr, uint8_t data);
uint8_t  cc1100_read_register	        (struct _cc1100_t *cc1100, uint8_t addr);
void	 cc1100_state_register	        (struct _cc1100_t *cc1100, uint8_t state);
void	 cc1100_crc_register		(struct _cc1100_t *cc1100, uint8_t ok);
void	 cc1100_lqi_register		(struct _cc1100_t *cc1100, uint8_t lqi);


/***************************************
* Pin functions (cc1100_gdo.c)
***************************************/
void	 cc1100_spi_output		(struct _cc1100_t *cc1100, uint8_t val);
void	 cc1100_assert_gdo		(struct _cc1100_t *cc1100, int event, int assert);
void	 cc1100_update_xosc		(struct _cc1100_t *cc1100);
void	 cc1100_update_gdo		(struct _cc1100_t *cc1100, uint8_t val);


/***************************************
* Fifo functions (cc1100_fifo.c)
***************************************/
void	 cc1100_put_tx_fifo		(struct _cc1100_t *cc1100, uint8_t val);
uint8_t  cc1100_get_tx_fifo		(struct _cc1100_t *cc1100);
void	 cc1100_flush_tx_fifo	        (struct _cc1100_t *cc1100);
int	 cc1100_put_rx_fifo		(struct _cc1100_t *cc1100, uint8_t val);
uint8_t  cc1100_get_rx_fifo		(struct _cc1100_t *cc1100);
void	 cc1100_flush_rx_fifo	        (struct _cc1100_t *cc1100);


/***************************************
* Tx functions (cc1100_tx.c)
***************************************/
double	 cc1100_get_frequency_mhz       (struct _cc1100_t *cc1100);
int	 cc1100_get_modulation	        (struct _cc1100_t *cc1100);
double	 cc1100_get_power_dbm		(struct _cc1100_t *cc1100);
uint64_t cc1100_get_tx_byte_duration    (struct _cc1100_t *cc1100);
uint8_t  cc1100_get_preamble_length     (struct _cc1100_t *cc1100);

void	 cc1100_tx                      (struct _cc1100_t *cc1100);


/***************************************
* Rx functions (cc1100_rx.c)
***************************************/

uint64_t cc1100_callback_rx		(void *, struct wsnet_rx_info*);
void	 cc1100_compute_cca		(struct _cc1100_t *cc1100);
void	 cc1100_rx_state		(struct _cc1100_t *cc1100);


/***************************************
* Strobe functions (cc1100_strobe.c)
***************************************/
void     cc1100_strobe_command          (struct _cc1100_t *cc1100);


#endif ///_CC1100_INTERNALS_H
