
/**
 *  \file   cc2420_internals.h
 *  \brief  CC2420 internal definitions
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_internals.h
 *  
 *
 *  Created by Nicolas Boulicault on 31/05/07.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_INTERNALS_H
#define _CC2420_INTERNALS_H

#include "libwsnet/libwsnet.h"
#include "machine/machine.h"

#include "cc2420_state.h"


/***************************************************/
/***************************************************/
/***************************************************/
/*
 * RAM and registers size
 */

#define CC2420_REG_SIZE   0x40   /* last register is 0x3F */
#define CC2420_RAM_SIZE   0x16F

extern tracer_id_t TRACER_CC2420_STATE;
extern tracer_id_t TRACER_CC2420_STROBE;
extern tracer_id_t TRACER_CC2420_CS;
extern tracer_id_t TRACER_CC2420_FIFOP;
extern tracer_id_t TRACER_CC2420_FIFO;
extern tracer_id_t TRACER_CC2420_CCA;
extern tracer_id_t TRACER_CC2420_SFD;
extern tracer_id_t TRACER_CC2420_VREG_EN;
extern tracer_id_t TRACER_CC2420_RESET;

/***************************************************/
/***************************************************/
/***************************************************/

/* voltage regulator startup time, 6ms, cf p13 */
#define CC2420_VREG_STARTUP_TIME 6000000

/* oscillator strartup time, cf p12 (depends on the crystal used) */
#define CC2420_XOSC_STARTUP_TIME 1000000

/* TODO:: calculated from bitrate and sb/chip */
#define CC2420_SYMBOL_PERIOD       16000


/* cf p77 */
#define CC2420_PLL_CALIBRATION_TIME_SHORT 37000
#define CC2420_PLL_CALIBRATION_TIME_LONG  57000

/*
 * CC2420 SPI access types
 * used in cc2420_dev.c, cc2420_read() to store the SPI access type
 */

/* address byte */
#define CC2420_SPI_NEXT_ACCESS_TYPE_ADDR                 0
/* register access */
#define CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE1      1
#define CC2420_SPI_NEXT_ACCESS_TYPE_REG_WRITE_BYTE2      2
#define CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE1       3
#define CC2420_SPI_NEXT_ACCESS_TYPE_REG_READ_BYTE2       4
/* TX RX FIFO access */
#define CC2420_SPI_NEXT_ACCESS_TYPE_TXFIFO_WRITE         5
#define CC2420_SPI_NEXT_ACCESS_TYPE_RXFIFO_READ          6
/* RAM access */
#define CC2420_SPI_NEXT_ACCESS_TYPE_RAM_BANK_SELECT      7
#define CC2420_SPI_NEXT_ACCESS_TYPE_RAM_READ_BYTE        8
#define CC2420_SPI_NEXT_ACCESS_TYPE_RAM_WRITE_BYTE       9


/* CC2420 SPI communications */
#define CC2420_SPI_ACCESS_ZONE(x)         ((x) & 0x80)
#define CC2420_SPI_ACCESS_RW(x)           ((x) & 0x40)
#define CC2420_SPI_ACCESS_REG_ADDR(x)     ((x) & 0x3F)
#define CC2420_SPI_ACCESS_RAM_ADDR(x)     ((x) & 0x7F)


/*
 * CC2420 internal pins access
 */

#define CC2420_INTERNAL_FIFO_PIN    0
#define CC2420_INTERNAL_FIFOP_PIN   1
#define CC2420_INTERNAL_CCA_PIN     2
#define CC2420_INTERNAL_SFD_PIN     3
#define CC2420_INTERNAL_SI_PIN      4
#define CC2420_INTERNAL_SO_PIN      5
#define CC2420_INTERNAL_CSn_PIN     6
#define CC2420_INTERNAL_VREG_EN_PIN 7
#define CC2420_INTERNAL_RESET_PIN   8


/*
 * Authorized memory zones (depend on fsm state)
 */
#define CC2420_ACCESS_NONE            0  /* no memory is available */
#define CC2420_ACCESS_MAIN_ONLY       1  /* only main register is available */
#define CC2420_ACCESS_REGISTERS_ONLY  2  /* only registers are available */
#define CC2420_ACCESS_ALL             3  /* registers and RAM (including TX and RX FIFOs) are available */


/*
 * CCA modes, cf [1] p. 65
 */
#define CC2420_CCA_MODE_RESERVED      0 /* reserved mode */
#define CC2420_CCA_MODE_THRESHOLD     1 /* threshold mode */
#define CC2420_CCA_MODE_DATA          2 /* valid IEEE 802.15.4 data mode */
#define CC2420_CCA_MODE_BOTH          3 /* both 1 and 2 modes */


/*
 * Used for test output signals pins (CCA and SFD)
 */
#define CC2420_PIN_ASSERT             1
#define CC2420_PIN_DEASSERT           0

/***************************************************/
/***************************************************/
/***************************************************/

struct _cc2420_t {
    uint8_t  fsm_state;
    uint8_t  fsm_ustate;
    uint64_t fsm_timer;

    uint8_t  FIFO_pin;
    uint8_t  FIFOP_pin;
    uint8_t  CCA_pin;
    uint8_t  SFD_pin;
    uint8_t  SI_pin;
    uint8_t  SO_pin;
    uint8_t  CSn_pin;
    uint8_t  VREG_EN_pin;
    uint8_t  RESET_pin;

    int      FIFO_set;
    int      FIFOP_set;
    int      CCA_set;
    int      SFD_set;
    int      SI_set;
    int      SO_set;
    int      VREG_EN_set;
    int      RESET_set;

    uint8_t  SI_type;  /* type of next SPI access */
    uint8_t  SPI_addr; /* address read on SPI bus */

    uint8_t  SI_byte1; /* store the first byte received when writing to a register */

    uint8_t  status_byte;

    uint16_t registers[CC2420_REG_SIZE];
    uint8_t  ram      [CC2420_RAM_SIZE];

    uint8_t  ram_bank;

    /*
     * allowed memory access. Upon FSM state, can be
     *  - CC2420_ACCESS_MAIN_ONLY
     *  - CC2420_ACCESS_REGISTERS_ONLY
     *  - CC2420_ACCESS_ALL
     */
    
    uint8_t  mem_access;

    /* cca mode, threshold and hysteresis */
    uint8_t cca_mode;
    int8_t  cca_threshold;
    uint8_t cca_hyst;

    /* cca value storage when cca pin isn't used to return cca value */
    uint8_t cca_internal_value;

    /* state of XOSC */
    uint8_t xosc_stable;

    /* state of PLL */
    uint8_t pll_locked;

    /* time to reach for PLL to become locked */
    uint64_t pll_lock_timer;

    /* state of encoding module */
    uint8_t encoding_busy;

    /* is tx active ? */
    uint8_t tx_active;

    /* number of user bytes in FIFO */
    uint8_t tx_fifo_len;

    /* number of available bytes in TX FIFO */
    uint8_t tx_available_bytes;

    /* number of needed bytes in TX FIFO to avoid TX underflow */
    uint8_t tx_needed_bytes;

    /* length of the frame we have to transmit */
    uint8_t tx_frame_len;

    /* tx frame pending */
    uint8_t tx_frame_pending;

    /* tx frame completed */
    uint8_t tx_frame_completed;

    /* store the time when we began to transmit a frame, including preamble */
    uint64_t tx_start_time;

    /* number of symbols in preamble, including SFD */
    uint8_t tx_preamble_symbols;

    /* flag indicating tx underflow */
    uint8_t tx_underflow;

    /* number of bytes sent within current frame */
    uint8_t tx_bytes;

    uint64_t tx_timer;

    /* the FCS of the current frame */
    uint16_t tx_fcs;

    /* next RX symbol time */
    uint64_t rx_sync_timer;

    /* rssi value */
    int8_t rx_rssi_value;

    /* 8 samples to compute rssi */
    int8_t rx_rssi_samples [8];

    /* next index of rssi samples tab to be write */
    int rx_rssi_sample_index;
  
    /* rssi value to replace the FCS first byte when CRC is ok (only the 8 first symbols following SFD) */
    int8_t rx_rssi_value_for_fcs;
    
    /* number of values used to compute rssi */
    uint8_t rx_rssi_values;

    /* rssi validity */
    uint8_t rx_rssi_valid;

    /* correlation/LQI byte count */
    uint8_t corr_lqi_count;

    /* correlation/LQI value */
    uint16_t corr_lqi_value;

    /* rssi validity timer */
    uint64_t rx_rssi_timer;

    /* number of received zero_symbols */
    uint8_t rx_zero_symbols;

    /* number of received data bytes within currently received frame */
    uint8_t rx_data_bytes;

    /* rx frame length field of currently received frame */
    uint8_t rx_len;

    /* rx fifo indexes, range is 0 - CC2420_RAM_RXFIFO_LEN */
    uint8_t rx_fifo_read;  
    uint8_t rx_fifo_write;

    /* flag for rx overflow */
    uint8_t rx_overflow;

    /* position of the beginning of current frame in rx fifo */
    /* useful if the frame is rejected (address recognition or bad CRC)   */
    uint8_t rx_frame_start;

    /* position of 1st data byte av in FIFO */
    int16_t rx_first_data_byte;

    /* the frame control field */
    uint16_t rx_fcf;

    /* store the sequence number */
    uint8_t  rx_sequence;

    /* frame type */
    uint8_t  rx_frame_type;

    /* secutiry enabled */
    uint8_t  rx_sec_enabled;

    /* data pending */
    uint8_t  rx_frame_pending;

    /* ack request */
    uint8_t  rx_ack_req;

    /* intra pan */
    uint8_t  rx_intra_pan;

    /* a flag to indicate that address recog failed and frame has to be flushed */
    uint8_t  rx_addr_decode_failed;

    /* dest addressing mode */
    uint8_t  rx_dst_addr_mode;

    /* src addressing mode */
    uint8_t  rx_src_addr_mode;

    /* dest PAN id */
    uint8_t  rx_dst_pan_offset;
    uint8_t  rx_dst_pan_len;

    /* src PAN id */
    uint8_t  rx_src_pan_offset;
    uint8_t  rx_src_pan_len;

    /* RX frame dst address */
    uint8_t  rx_dst_addr_offset;
    uint8_t  rx_dst_addr_len;
    
    /* RX frame src address */
    uint8_t  rx_src_addr_offset;
    uint8_t  rx_src_addr_len;

    /* number of complete frames in RX FIFO */
    int      nb_rx_frames;

    /* position in RX FIFO of the last byte of oldest RX frame */
    int16_t  rx_frame_end;

    /* radio id in libwsnet2 */
    int worldsens_radio_id;
};

uint64_t cc2420_callback_rx    (void *arg, struct wsnet_rx_info *);
void     cc2420_reset_internal (struct _cc2420_t * cc2420);
uint8_t  cc2420_read_pin       (struct _cc2420_t * cc2420, uint8_t pin);
int      cc2420_io_pins        (struct _cc2420_t * cc2420);


int      cc2420_reset          (int dev_num);
int      cc2420_delete         (int dev_num);
int      cc2420_update         (int dev_num);
int      cc2420_power_up       (int dev_num);
int      cc2420_power_down     (int dev_num);

void     cc2420_read           (int dev_num, uint32_t *mask, uint32_t *value);
void     cc2420_write          (int dev_num, uint32_t  mask, uint32_t  value);

#endif
