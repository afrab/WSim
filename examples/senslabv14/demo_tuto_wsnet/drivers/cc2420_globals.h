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
 * \file cc2420_globals.h
 * \brief Configuration registers addresses and default values
 * \author Clément Burin des Roziers <clement.burin-des-roziers@inria.fr>
 * \date 28/01/2009
 * 
 */

#ifndef _CC2420_GLOBALS_H
#define _CC2420_GLOBALS_H

/* Strobe command addresses */
#define CC2420_STROBE_NOP       0x00
#define CC2420_STROBE_XOSCON    0x01
#define CC2420_STROBE_TXCAL     0x02
#define CC2420_STROBE_RXON      0x03
#define CC2420_STROBE_TXON      0x04
#define CC2420_STROBE_TXONCCA   0x05
#define CC2420_STROBE_RFOFF     0x06
#define CC2420_STROBE_XOSCOFF   0x07
#define CC2420_STROBE_FLUSHRX   0x08
#define CC2420_STROBE_FLUSHTX   0x09
#define CC2420_STROBE_ACK       0x0A
#define CC2420_STROBE_ACKPEND   0x0B
#define CC2420_STROBE_RXDEC     0x0C
#define CC2420_STROBE_TXENC     0x0D
#define CC2420_STROBE_AES       0x0E

/* RAM/Register bit */
#define CC2420_RAM_ACCESS       0x80
#define CC2420_RAM_WRITE_ACCESS 0x00
#define CC2420_RAM_READ_ACCESS  0x20
#define CC2420_REG_ACCESS       0x00
#define CC2420_WRITE_ACCESS     0x00
#define CC2420_READ_ACCESS      0x40

/* RAM addresses */
#define CC2420_RAM_SHORTADR     0x16A
#define CC2420_RAM_PANID        0x168
#define CC2420_RAM_IEEEADR      0x160

/* Register addresses */
#define CC2420_REG_MAIN         0x10
#define CC2420_REG_MDMCTRL0     0x11
    #define ADR_DECODE  (1<<11)
    #define AUTOCRC     (1<<5)
    #define AUTOACK     (1<<4)
#define CC2420_REG_MDMCTRL1     0x12
#define CC2420_REG_RSSI         0x13
    #define CCATHR_MASK (0xFF<<8)
    #define RSSI_MASK   (0xFF)
#define CC2420_REG_SYNCWORD     0x14
#define CC2420_REG_TXCTRL       0x15
    #define PALEVEL_MASK (0x1F)
#define CC2420_REG_RXCTRL0      0x16
#define CC2420_REG_RXCTRL1      0x17
    #define RXBPF_LOCUR (1<<13)
#define CC2420_REG_FSCTRL       0x18
    #define FREQ_MASK   (0x3FF)
#define CC2420_REG_SECCTRL0     0x19
    #define RXFIFO_PROTECTION (1<<9)
#define CC2420_REG_SECCTRL1     0x1A
#define CC2420_REG_BATTMON      0x1B
#define CC2420_REG_IOCFG0       0x1C
    #define FIFOPTHR_MASK (0x7F)
#define CC2420_REG_IOCFG1       0x1D
#define CC2420_REG_MANFIDL      0x1E
#define CC2420_REG_MANFIDH      0x1F
#define CC2420_REG_FSMTC        0x20
#define CC2420_REG_MANAND       0x21
#define CC2420_REG_MANOR        0x22
#define CC2420_REG_AGCCTRL      0x23
#define CC2420_REG_AGCTST0      0x24
#define CC2420_REG_AGCTST1      0x25
#define CC2420_REG_AGCTST2      0x26
#define CC2420_REG_FSTST0       0x27
#define CC2420_REG_FSTST1       0x28
#define CC2420_REG_FSTST2       0x29
#define CC2420_REG_FSTST3       0x2A
#define CC2420_REG_RXBPFTST     0x2B
#define CC2420_REG_FSMSTATE     0x2C
#define CC2420_REG_ADCTST       0x2D
#define CC2420_REG_DACTST       0x2E
#define CC2420_REG_TOPTST       0x2F

#define CC2420_REG_TXFIFO       0x3E
#define CC2420_REG_RXFIFO       0x3F

#endif
