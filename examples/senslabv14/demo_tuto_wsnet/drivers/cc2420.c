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
 * \file cc2420.c
 * \brief CC2420 driver implementation
 * \author Clément Burin des Roziers <clement.burin-des-roziers@inria.fr>
 * \date January 09
 */
#include <io.h>
#include <signal.h>

#include "cc2420.h"

#include "spi1.h"

#include "cc2420_io.h"
#include "cc2420_spi.h"
#include "cc2420_globals.h"


static cc2420_cb_t fifo_cb = 0x0,
                   fifop_cb = 0x0,
                   sfd_cb = 0x0,
                   cca_cb = 0x0;

void inline micro_delay(register unsigned int n)
{
    __asm__ __volatile__ (
  "1: \n"
  " dec %[n] \n"
  " jne 1b \n"
        : [n] "+r"(n));
}

uint16_t cc2420_init(void)
{
    uint16_t reg;

    /* Initialize IO pins */
    CC2420_IO_INIT();

    /* Initialize SPI */
    SPI1_INIT();
 
    /* Power on the cc2420 and reset it */
    VREGEN_SET(1);
    RESETn_SET(0);
    micro_delay(200);
    RESETn_SET(1);
    
    /* Turn on the crystal oscillator. */
    CC2420_STROBE_CMD(CC2420_STROBE_XOSCON);

    CC2420_READ_REG(CC2420_REG_MDMCTRL0, reg);  
#ifdef CC2420_ENABLE_ADR_DECODE
    /* Turn on automatic packet acknowledgment and address decoding. */
    reg |= ADR_DECODE;
#elif CC2420_ENABLE_AUTOACK
    reg |= ADR_DECODE;
    reg |= AUTOACK;
#else
    /* Turn off automatic packet acknowledgment and address decoding. */
    reg &= ~AUTOACK;
    reg &= ~ADR_DECODE;
#endif
    CC2420_WRITE_REG(CC2420_REG_MDMCTRL0, reg);

    /* Change default values as recomended in the data sheet, */
    /* RX bandpass filter = 1.3uA. */
    CC2420_READ_REG(CC2420_REG_RXCTRL1, reg);
    reg |= RXBPF_LOCUR;
    CC2420_WRITE_REG(CC2420_REG_RXCTRL1, reg);

    /* Set output power */
    cc2420_set_txpower(0xa0e3);

    /* Set the FIFOP threshold to maximum. */
    CC2420_WRITE_REG(CC2420_REG_IOCFG0, 127);

    /* Turn off "Security enable" (page 32). */
    CC2420_READ_REG(CC2420_REG_SECCTRL0, reg);
    reg &= ~RXFIFO_PROTECTION;
    CC2420_WRITE_REG(CC2420_REG_SECCTRL0, reg);

    return 1;
}

/* Configuration */
uint16_t cc2420_set_frequency(uint16_t freq)
{
    uint16_t reg;
    
    if (freq < 2048)
    {
        return 0;
    }
    
    freq -= 2048;
    
    if (freq >= (1<<10))
    {
        return 0;
    }
    
    CC2420_READ_REG(CC2420_REG_FSCTRL, reg);
    reg &= ~FREQ_MASK;
    reg |= freq;
    CC2420_WRITE_REG(CC2420_REG_FSCTRL, reg);
    
    return 1;
}

uint16_t cc2420_set_txpower(uint16_t power)
{
    uint16_t reg;
    
    CC2420_READ_REG(CC2420_REG_TXCTRL, reg);
    reg &= ~PALEVEL_MASK;
    reg |= (power&PALEVEL_MASK);
    CC2420_WRITE_REG(CC2420_REG_TXCTRL, reg);
    
    return 1;
}

uint16_t cc2420_set_fifopthr(uint16_t thr)
{
    uint16_t reg;
    
    CC2420_READ_REG(CC2420_REG_IOCFG0, reg);
    reg &= ~FIFOPTHR_MASK;
    reg |= (thr & FIFOPTHR_MASK);
    CC2420_WRITE_REG(CC2420_REG_IOCFG0, reg);
    
    return 1;
}

uint16_t cc2420_set_ccathr(uint16_t thr)
{
    uint16_t reg;
    
    CC2420_READ_REG(CC2420_REG_RSSI, reg);
    reg &= ~CCATHR_MASK;
    reg |= (thr << 8);
    CC2420_WRITE_REG(CC2420_REG_RSSI, reg);
    
    return 1;
}

uint16_t cc2420_set_panid(uint8_t *panid)
{
    CC2420_WRITE_RAM(CC2420_RAM_PANID, panid, 2);

    return 1;
}

uint16_t cc2420_set_shortadr(uint8_t *shortadr)
{
    CC2420_WRITE_RAM(CC2420_RAM_SHORTADR, shortadr, 2);

    return 1;
}

uint16_t cc2420_set_ieeeadr(uint8_t *ieeeadr)
{
    CC2420_WRITE_RAM(CC2420_RAM_IEEEADR, ieeeadr, 8);

    return 1;
}


/* Infos */
uint16_t cc2420_get_status(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_NOP );
    return spi1_tx_return_value;
}

uint16_t cc2420_get_rssi(void)
{
    uint16_t reg;
    
    CC2420_READ_REG(CC2420_REG_RSSI, reg);
    
    return (reg & 0x00FF);
}

/* Commands */
uint16_t cc2420_cmd_xoscoff(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_XOSCOFF );
    return 1;
}
uint16_t cc2420_cmd_xoscon(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_XOSCON );
    return 1;
}
uint16_t cc2420_cmd_idle(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_RFOFF );
    return 1;
}
uint16_t cc2420_cmd_rx(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_RXON );
    return 1;
}
uint16_t cc2420_cmd_tx(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_TXON );
    return 1;
}
uint16_t cc2420_cmd_flushrx(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_FLUSHRX );
    return 1;
}
uint16_t cc2420_cmd_flushtx(void)
{
    CC2420_STROBE_CMD( CC2420_STROBE_FLUSHTX );
    return 1;
}

/* FIFOs */
uint16_t cc2420_fifo_put(uint8_t* data, uint16_t data_length)
{
    CC2420_WRITE_FIFO(data, data_length);
    return 1;
}

uint16_t cc2420_fifo_get(uint8_t* data, uint16_t data_length)
{
    CC2420_READ_FIFO(data, data_length);
    return 1;
}

/* Interrupts */

// FIFO
uint16_t cc2420_io_fifo_register_cb(cc2420_cb_t cb)
{
    fifo_cb = cb;
    return 1;
}
uint16_t cc2420_io_fifo_int_enable(void)
{
    FIFO_INT_ENABLE();
    return 1;
}
uint16_t cc2420_io_fifo_int_disable(void)
{
    FIFO_INT_DISABLE();
    return 1;
}
uint16_t cc2420_io_fifo_int_clear(void)
{
    FIFO_INT_CLEAR();
    return 1;
}
uint16_t cc2420_io_fifo_int_set_falling(void)
{
    FIFO_INT_SET_FALLING();
    return 1;
}
uint16_t cc2420_io_fifo_int_set_rising(void)
{
    FIFO_INT_SET_RISING();
    return 1;
}
uint16_t cc2420_io_fifo_read(void)
{
    return FIFO_READ();
}

// FIFOP
uint16_t cc2420_io_fifop_register_cb(cc2420_cb_t cb)
{
    fifop_cb = cb;
    return 1;
}
uint16_t cc2420_io_fifop_int_enable(void)
{
    FIFOP_INT_ENABLE();
    return 1;
}
uint16_t cc2420_io_fifop_int_disable(void)
{
    FIFOP_INT_DISABLE();
    return 1;
}
uint16_t cc2420_io_fifop_int_clear(void)
{
    FIFOP_INT_CLEAR();
    return 1;
}
uint16_t cc2420_io_fifop_int_set_falling(void)
{
    FIFOP_INT_SET_FALLING();
    return 1;
}
uint16_t cc2420_io_fifop_int_set_rising(void)
{
    FIFOP_INT_SET_RISING();
    return 1;
}
uint16_t cc2420_io_fifop_read(void)
{
    return FIFOP_READ();
}

// SFD
uint16_t cc2420_io_sfd_register_cb(cc2420_cb_t cb)
{
    sfd_cb = cb;
    return 1;
}
uint16_t cc2420_io_sfd_int_enable(void)
{
    SFD_INT_ENABLE();
    return 1;
}
uint16_t cc2420_io_sfd_int_disable(void)
{
    SFD_INT_DISABLE();
    return 1;
}
uint16_t cc2420_io_sfd_int_clear(void)
{
    SFD_INT_CLEAR();
    return 1;
}
uint16_t cc2420_io_sfd_int_set_falling(void)
{
    SFD_INT_SET_FALLING();
    return 1;
}
uint16_t cc2420_io_sfd_int_set_rising(void)
{
    SFD_INT_SET_RISING();
    return 1;
}
uint16_t cc2420_io_sfd_read(void)
{
    return SFD_READ();
}

// CCA
uint16_t cc2420_io_cca_register_cb(cc2420_cb_t cb)
{
    cca_cb = cb;
    return 1;
}
uint16_t cc2420_io_cca_int_enable(void)
{
    CCA_INT_ENABLE();
    return 1;
}
uint16_t cc2420_io_cca_int_disable(void)
{
    CCA_INT_DISABLE();
    return 1;
}
uint16_t cc2420_io_cca_int_clear(void)
{
    CCA_INT_CLEAR();
    return 1;
}
uint16_t cc2420_io_cca_int_set_falling(void)
{
    CCA_INT_SET_FALLING();
    return 1;
}
uint16_t cc2420_io_cca_int_set_rising(void)
{
    CCA_INT_SET_RISING();
    return 1;
}
uint16_t cc2420_io_cca_read(void)
{
    return CCA_READ();
}

/**
 * Interrupt service routine for PORT1.
 * Used for handling CC2420 interrupts triggered on
 * the IO pins.
 */
interrupt(PORT1_VECTOR) port1irq(void)
{
    if (P1IFG & FIFO_PIN)
    {
        FIFO_INT_CLEAR();
        if (fifo_cb != 0x0)
        {
            if ( fifo_cb() )
            {
                LPM4_EXIT;
            }
        }
        
    }

    if (P1IFG & FIFOP_PIN)
    {
        FIFOP_INT_CLEAR();
        if (fifop_cb != 0x0)
        {
            if ( fifop_cb() )
            {
                LPM4_EXIT;
            }
        }
        
    }

    if (P1IFG & SFD_PIN)
    {
        SFD_INT_CLEAR();
        if (sfd_cb != 0x0)
        {
            if ( sfd_cb() )
            {
                LPM4_EXIT;
            }
        }
        
    }
}
