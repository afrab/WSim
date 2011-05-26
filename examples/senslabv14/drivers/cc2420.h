/*
 * Copyright  2008-2009 INRIA/SensTools
 * 
 * <dev-team@sentools.info>
 * 
 * This software is a set of libraries designed to develop applications
 * for the WSN430 embedded hardware platform.
 * 
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */

/**
 * \file cc2420.h
 * \brief CC2420 driver header
 * \author ClÃ©ment Burin des Roziers <clement.burin-des-roziers@inria.fr>
 * \date January 09
 */
#ifndef _CC2420_H
#define _CC2420_H

/**
 * The callback function type, used for all interrupts.
 * The registered function should return 0 if the CPU must remain
 * in its low power state, or anything else in order to wake the CPU up.
 */
typedef uint16_t (*cc2420_cb_t) (void);

extern uint8_t cc2420_received_frame[128];
extern uint16_t cc2420_received_length;

/**
 * Initialize the CC2420, by setting the default register values.
 * \return 1
 */
uint16_t cc2420_init(void);

/* Configuration */
/**
 * Set the operating frequency in MHz.
 * \param freq the frequency, should be between 2048 and 3072
 * \return 1 if the frequency is set, 0 if not
 */
uint16_t cc2420_set_frequency(uint16_t freq);

/**
 * Set the TX power.
 * Here is the corresponding table
 * output power     : 31 | 27 | 23 | 19 | 15 |  11 |  7  |  3
 * param value (dBm): 0  | -1 | -3 | -5 | -7 | -10 | -15 | -25
 * \param power the power value
 * \return 1
 */
uint16_t cc2420_set_txpower(uint16_t power);

/**
 * Set the FIFO threshold for setting the FIFOP pin.
 * \param thr the number of bytes in RXFIFO needed by the FIFOP pin
 * \return 1
 */
uint16_t cc2420_set_fifopthr(uint16_t thr);

/**
 * Set the CCA threshold. If the RSSI measure is below this value,
 * CCA is asserted. Offset is the same as the RSSI measures.
 * \param thr the threshold value (0x00-0xFF)
 * \return 1
 */
uint16_t cc2420_set_ccathr(uint16_t thr);

/**
 * Set in RAM the PANID value
 * \param *panid the pan id value (0x0000-0xFFFF)
 * \return 1
 */
uint16_t cc2420_set_panid(uint8_t *panid);

/**
 * Set in RAM the SHORTADR value
 * \param *shortadr the short addess value (0x0000-0xFFFF)
 * \return 1
 */
uint16_t cc2420_set_shortadr(uint8_t *shortadr);

/**
 * Set in RAM the IEEEADR value
 * \param *ieeeadr the extended address value (0x0000000000000000-0xFFFFFFFFFFFFFFFF)
 * \return 1
 */
uint16_t cc2420_set_ieeeadr(uint8_t *ieeeadr);


/* Info */
#define CC2420_STATUS_XOSC_STABLE  (1<<6)
#define CC2420_STATUS_TX_UNDERFLOW (1<<5)
#define CC2420_STATUS_ENC_BUSY     (1<<4)
#define CC2420_STATUS_TX_ACTIVE    (1<<3)
#define CC2420_STATUS_LOCK         (1<<2)
#define CC2420_STATUS_RSSI_VALID   (1<<1)
/**
 * Get the CC2420 chip status. The bits of interest are described above.
 * \return the chip status value
 */

uint16_t cc2420_get_status(void);

/**
 * Read the RSSI value. Measure validity can be checked
 * with the RSSI_VALID bit of the Status byte.
 * \return the rssi readout.
 */
uint16_t cc2420_get_rssi(void);

/* Commands */
/**
 * Stop the CC2420 oscillator.
 * \return 1
 */
uint16_t cc2420_cmd_xoscoff(void);

/**
 * Start the CC2420 oscillator.
 * Check the XOSC_STABLE bit of the Status byte to know when it's stable.
 * \return 1
 */
uint16_t cc2420_cmd_xoscon(void);

/**
 * Stop RX or TX and go to the idle state.
 * \return 1
 */
uint16_t cc2420_cmd_idle(void);

/**
 * Start RX.
 * \return 1
 */
uint16_t cc2420_cmd_rx(void);

/**
 * Start TX. A complete frame should be present in the TX FIFO.
 * \return 1
 */
uint16_t cc2420_cmd_tx(void);

/**
 * Flush the RX FIFO.
 * \return 1
 */
uint16_t cc2420_cmd_flushrx(void);

/**
 * Flush the TX FIFO.
 * \return 1
 */
uint16_t cc2420_cmd_flushtx(void);

/* FIFOs */
/**
 * Write data to the TX FIFO. Take care to respect the frame format.
 * A complete frame put in FIFO should look like this:
 * length of the following bytes (1 byte) | data (n bytes) | FCS (2 bytes)
 * The FCS bytes are filled by the chip and need not to be written.
 * The length byte value must be (n+3)
 * \param data a pointer to the data to write.
 * \param data_length the number of bytes to write.
 * \return 1
 */
uint16_t cc2420_fifo_put(uint8_t* data, uint16_t data_length);
/**
 * Read data from the RX FIFO. Respect the frame format.
 * If a frame has been received, the first byte read is the length of the frame,
 * then the data is read, and finally an extra 2byte is read containing
 * the RSSI measure, and the CRC/LQI byte. The CRC bit of this later byte
 * is set to 1 if the CRC was correct, or 0 if not. It should be checked.
 * \param data a pointer to a buffer to store the data.
 * \param data_length the number of bytes to read.
 * \return 1
 */
uint16_t cc2420_fifo_get(uint8_t* data, uint16_t data_length);

/* Interrupts & IOs */
/**
 * Register a callback function to be called when an interrupt
 * happens on the FIFO pin.
 * \param cb a callback function pointer.
 * \return 1
 */
uint16_t cc2420_io_fifo_register_cb(cc2420_cb_t cb);
/**
 * Register a callback function to be called when an interrupt
 * happens on the FIFOP pin.
 * \param cb a callback function pointer.
 * \return 1
 */
uint16_t cc2420_io_fifop_register_cb(cc2420_cb_t cb);
/**
 * Register a callback function to be called when an interrupt
 * happens on the SFD pin.
 * \param cb a callback function pointer.
 * \return 1
 */
uint16_t cc2420_io_sfd_register_cb(cc2420_cb_t cb);
/**
 * Register a callback function to be called when an interrupt
 * happens on the CCA pin.
 * \param cb a callback function pointer.
 * \return 1
 */
uint16_t cc2420_io_cca_register_cb(cc2420_cb_t cb);

uint16_t cc2420_io_fifo_int_enable(void);
uint16_t cc2420_io_fifo_int_disable(void);
uint16_t cc2420_io_fifo_int_clear(void);
uint16_t cc2420_io_fifo_int_set_falling(void);
uint16_t cc2420_io_fifo_int_set_rising(void);
uint16_t cc2420_io_fifo_read(void);

uint16_t cc2420_io_fifop_int_enable(void);
uint16_t cc2420_io_fifop_int_disable(void);
uint16_t cc2420_io_fifop_int_clear(void);
uint16_t cc2420_io_fifop_int_set_falling(void);
uint16_t cc2420_io_fifop_int_set_rising(void);
uint16_t cc2420_io_fifop_read(void);

uint16_t cc2420_io_sfd_int_enable(void);
uint16_t cc2420_io_sfd_int_disable(void);
uint16_t cc2420_io_sfd_int_clear(void);
uint16_t cc2420_io_sfd_int_set_falling(void);
uint16_t cc2420_io_sfd_int_set_rising(void);
uint16_t cc2420_io_sfd_read(void);

uint16_t cc2420_io_cca_int_enable(void);
uint16_t cc2420_io_cca_int_disable(void);
uint16_t cc2420_io_cca_int_clear(void);
uint16_t cc2420_io_cca_int_set_falling(void);
uint16_t cc2420_io_cca_int_set_rising(void);
uint16_t cc2420_io_cca_read(void);

/* Other */
void micro_delay(unsigned int n);

#endif
