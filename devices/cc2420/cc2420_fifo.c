
/**
 *  \file   cc2420_fifo.c
 *  \brief  CC2420 Data RX/TX fifo 
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_fifo.c
 *  
 *
 *  Created by Nicolas Boulicault on 06/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *  Modified by Loic Lemaitre 2009
 *
 */

#include "cc2420_internals.h"
#include "cc2420_registers.h"
#include "cc2420_fifo.h"
#include "cc2420_ram.h"
#include "cc2420_macros.h"
#include "cc2420_debug.h"
#include "cc2420_tx.h"


/**
 * write a byte to the TX FIFO
 */

void cc2420_tx_fifo_write(struct _cc2420_t * cc2420, uint8_t val) {
    
    /* the address where we have to write data */
    uint8_t addr;

    uint16_t autocrc = CC2420_REG_MDMCTRL0_AUTOCRC(cc2420->registers[CC2420_REG_MDMCTRL0]);

    if (cc2420->tx_fifo_len >= CC2420_RAM_TXFIFO_LEN) {
	CC2420_DEBUG("cc2420_tx_fifo_write : TX FIFO is FULL\n");
	return;
    }

    /* if FIFO is empty, it means that the written byte is the frame length */
    if (cc2420->tx_fifo_len == 0) {
	/* just use the first 7 bits */
	cc2420->tx_frame_len = CC2420_TX_LEN_FIELD(val);

	/* check if frame length is valid */
	if (cc2420->tx_frame_len > CC2420_MAX_TX_LEN) {
	    CC2420_DEBUG("cc2420_tx_fifo_write : frame length is too long, dropping byte\n");
	    cc2420->tx_frame_len = 0;
	    return;
	}
	else if ( (cc2420->tx_frame_len < 2 && autocrc) || ( cc2420->tx_frame_len < 1 && !autocrc ) ) {
	    CC2420_DEBUG("cc2420_tx_fifo_write : frame length is too low, dropping byte\n");
	    cc2420->tx_frame_len = 0;
	    return;
	    }

	/* calculate the real number of bytes we need before sending */
	if (autocrc) {
	  /* add 1 for length (length field is not included in length value) remove 2 for FCS */
	  cc2420->tx_needed_bytes = 1 + cc2420->tx_frame_len - 2;
	}
	else cc2420->tx_needed_bytes = 1 + cc2420->tx_frame_len;

	cc2420->tx_available_bytes = 0;
	cc2420->tx_fifo_len ++;
	cc2420->ram[0] = val;
	CC2420_DEBUG("cc2420_tx_fifo_write : val 0x%02x written in fifo at address 0x%02x\n", val, 0);
	return;
    }

    if (cc2420->tx_available_bytes == cc2420->tx_needed_bytes) {
	/* too many data in TX FIFO */
	CC2420_DEBUG("cc2420_tx_fifo_write : two many data (%d)\n", cc2420->tx_available_bytes);
	return;
    }

    addr = CC2420_RAM_TXFIFO_START + cc2420->tx_fifo_len;
    cc2420->ram[addr] = val;
    CC2420_DEBUG("cc2420_tx_fifo_write : val 0x%02x written in fifo at address 0x%02x\n", val, addr);
    cc2420->tx_fifo_len++;
    cc2420->tx_available_bytes ++;

    return;
}


/**
 * read a byte from RX FIFO, but don't remove it from fifo
 * don't touch read pointer
 * returns 0 if OK, -1 if no data in fifo
 */

int cc2420_rx_fifo_read(struct _cc2420_t * cc2420, uint8_t * val) {

   if (cc2420->rx_fifo_read == cc2420->rx_fifo_write) {
	CC2420_DEBUG("cc2420_rx_fifo_read : fifo is empty\n");
	return -1;
    }

    *val = cc2420->ram[CC2420_RAM_RXFIFO_START + cc2420->rx_fifo_read];

    //CC2420_DEBUG("RX_FIFO : reading byte %c at %d\n", *val, cc2420->rx_fifo_read);

    return 0;
}


/**
 * pop a byte from fifo
 * update read pointer
 * returns -1 if fifo is empty, 0 else
 */

int cc2420_rx_fifo_pop(struct _cc2420_t * cc2420, uint8_t * val) {

    uint8_t len_val;
    uint8_t calculate_length = 0;
    
    if (cc2420_rx_fifo_read(cc2420, val) < 0)
	return -1;

    /* if this is the first data byte AND data bytes are < rx_threshold, unset FIFOP pin */
    /* todo deal with address recog : FIFOP is not dealt the same way */
    uint8_t rx_threshold = CC2420_REG_IOCFG0_FIFOP_THR(cc2420->registers[CC2420_REG_MDMCTRL0]);
    if ( (cc2420->rx_fifo_read == cc2420->rx_first_data_byte) && (cc2420->rx_data_bytes < rx_threshold) ) {
	CC2420_DEBUG("cc2420_rx_fifo_pop : reading first data byte, unsetting FIFOP !!\n");
	cc2420->FIFOP_pin = 0x00;
	cc2420->FIFOP_set = 1;
	cc2420->rx_first_data_byte = -1;
    }
    
    /* if this is the end of the current frame,
       update the number of frames in RX FIFO */
    if (cc2420->rx_fifo_read == cc2420->rx_frame_end) {
	cc2420->nb_rx_frames --;
	CC2420_DEBUG("cc2420_rx_fifo_pop : nb frames in rx fifo decremented from %d to %d\n", 
		     cc2420->nb_rx_frames + 1, cc2420->nb_rx_frames);
	if (cc2420->nb_rx_frames > 0)
	    calculate_length = 1;
    }

    /* update the read pointer */
    cc2420->rx_fifo_read++;

    /* modulo */
    if (cc2420->rx_fifo_read == CC2420_RAM_RXFIFO_LEN) {
	cc2420->rx_fifo_read = 0;
    }
 
    /* if there is another frame in FIFO calculate where it finishes, and set FIFOP */
    if (calculate_length) {
	/* read len val from FIFO */
	len_val = cc2420->ram[CC2420_RAM_RXFIFO_START + cc2420->rx_fifo_read];
        /* set first data byte position of next frame */
	cc2420->rx_first_data_byte = cc2420->rx_fifo_read;
	if (cc2420->rx_first_data_byte >= CC2420_RAM_RXFIFO_LEN)
	    cc2420->rx_first_data_byte = cc2420->rx_first_data_byte - CC2420_RAM_RXFIFO_LEN;

	/* calculate where the last byte of the frame is */
	cc2420->rx_frame_end = cc2420->rx_fifo_read + len_val;
	if (cc2420->rx_frame_end >= CC2420_RAM_RXFIFO_LEN)
	    cc2420->rx_frame_end -= CC2420_RAM_RXFIFO_LEN;

	cc2420->FIFOP_set = 1;
	cc2420->FIFOP_pin = 0xFF;
    }

    return 0;
}


/**
 * write a byte to rx fifo 
 * update write pointer
 * return 0 if OK, -1 if overflow
 */

int cc2420_rx_fifo_push(struct _cc2420_t * cc2420, uint8_t val) {

    uint8_t threshold UNUSED;
    
    /* if fifo is full */
    if ( ( (cc2420->rx_fifo_write + 1) % CC2420_RAM_RXFIFO_LEN) == cc2420->rx_fifo_read ) {
	/* RW overflow state will be set by caller */
	CC2420_DEBUG("cc2420_rx_fifo_push : fifo overflow\n");
	return -1;
    }

    cc2420->ram[CC2420_RAM_RXFIFO_START + cc2420->rx_fifo_write] = val;

    CC2420_DEBUG("cc2420_rx_fifo_push: data 0x%02x written in fifo at addr 0x%02x\n", 
		 val, CC2420_RAM_RXFIFO_START + cc2420->rx_fifo_write);

    /* update write pointer */
    cc2420->rx_fifo_write ++;

    /* modulo */
    if (cc2420->rx_fifo_write == CC2420_RAM_RXFIFO_LEN) {
	cc2420->rx_fifo_write = 0;
    }

    
    /* todo : implement and test threshold */

    /*
    if (cc2420->rx_data_bytes >= threshold) {
	//cc2420->FIFOP_pin = 0xFF;
	//cc2420->FIFOP_set = 1;
	cc2420->nb_FIFOP ++;
	}*/

    return 0;
}


/**
 * read a buffer in rx fifo from specified address with specified len
 * 'buffer' must be allocated by caller and must have a length of at least 'len'
 * mainly used to calculate crc when an incoming frame has been received
 */

int cc2420_rx_fifo_get_buffer(struct _cc2420_t * cc2420, uint8_t start_address, uint8_t * buffer, uint8_t len) {
    uint8_t available_bytes;
    uint8_t addr;
    uint8_t i;
    
    /* no data in fifo */
    if (cc2420->rx_fifo_write == cc2420->rx_fifo_read) {
	CC2420_DEBUG("cc2420_rx_fifo_get_buffer : no data in rx fifo\n");
	return -1;
    }

    /* calculate the number of available bytes in RX FIFO */
    if (cc2420->rx_fifo_write > cc2420->rx_fifo_read) {
	available_bytes = cc2420->rx_fifo_write - cc2420->rx_fifo_read;
    }
    else {
	available_bytes = CC2420_RAM_RXFIFO_LEN - (cc2420->rx_fifo_read - cc2420->rx_fifo_write);
    }

    /* there are not enough data to fill buffer */
    if (available_bytes < len) {
	CC2420_DEBUG("cc2420_rx_fifo_get_buffer : not enough data in rx fifo\n");
	return -1;
    }

    addr = start_address;

    /* copy the buffer */
    for (i = 0; i < len; i++) {
	buffer[i] = cc2420->ram[CC2420_RAM_RXFIFO_START + ((addr + i) % CC2420_RAM_RXFIFO_LEN)];
    }

    return 0;
}
