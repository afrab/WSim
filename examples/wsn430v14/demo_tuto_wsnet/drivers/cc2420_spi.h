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
 * \file cc2420_spi.h
 * \brief CC2420 spi macros
 * \author Clément Burin des Roziers <clement.burin-des-roziers@inria.fr>
 * \date January 09
 */
#ifndef _CC2420_SPI_H
#define _CC2420_SPI_H

#include "spi1.h"
#include "cc2420_globals.h"

/**
 * Read a CC2420 register via SPI
 * \param addr register address
 * \param val variable to store the read value
 */
#define CC2420_READ_REG(addr, val) do \
{ \
    uint8_t value; \
    SPI1_CC2420_ENABLE(); \
    SPI1_TX((addr) | CC2420_READ_ACCESS); \
    SPI1_RX((value)); \
    val = value; \
    val <<= 8; \
    SPI1_RX((value)); \
    val += value; \
    SPI1_CC2420_DISABLE(); \
} while (0)
 

/**
 * Write a value to a CC2420 register via SPI
 * \param addr register address
 * \param val value to write
 */
#define CC2420_WRITE_REG(addr, val) do \
{ \
    SPI1_CC2420_ENABLE(); \
    SPI1_TX((addr) | CC2420_WRITE_ACCESS); \
    SPI1_TX(((char)(val>>8))); \
    SPI1_TX(((char)(val&0xFF))); \
    SPI1_CC2420_DISABLE(); \
} while (0)

/**
 * Send a command to CC2420 via SPI
 * \param cmd command to execute
 */
#define CC2420_STROBE_CMD(cmd) do \
{ \
    SPI1_CC2420_ENABLE(); \
    SPI1_TX((cmd)); \
    SPI1_CC2420_DISABLE(); \
} while (0) 

#define CC2420_READ_FIFO(val, len) do \
{ \
  uint16_t i; \
  SPI1_CC2420_ENABLE(); \
  SPI1_TX( CC2420_REG_RXFIFO | CC2420_READ_ACCESS); \
  for (i=0;i<(len);i++)  \
  { \
    SPI1_RX((val)[i]); \
  } \
  SPI1_CC2420_DISABLE(); \
} while (0)

#define CC2420_WRITE_FIFO(val, len) do \
{ \
  uint16_t i; \
  SPI1_CC2420_ENABLE(); \
  SPI1_TX( CC2420_REG_TXFIFO | CC2420_WRITE_ACCESS); \
  for (i=0;i<(len);i++)  \
  { \
    SPI1_TX((val)[i]); \
  } \
  SPI1_CC2420_DISABLE(); \
} while (0)


#define CC2420_READ_RAM(addr, val, len) do \
{ \
  uint16_t i; \
  SPI1_CC2420_ENABLE(); \
  SPI1_TX( CC2420_RAM_ACCESS | (addr & 0x7F) ); \
  SPI1_TX( ( (addr >> 1) & 0xC0 ) | CC2420_RAM_READ_ACCESS );\
  for (i=0;i<(len);i++)  \
  { \
    SPI1_RX((val)[i]); \
  } \
  SPI1_CC2420_DISABLE(); \
} while (0)


#define CC2420_WRITE_RAM(addr, val, len) do \
{ \
  uint16_t i; \
  SPI1_CC2420_ENABLE(); \
  SPI1_TX( CC2420_RAM_ACCESS | (addr & 0x7F) ); \
  SPI1_TX( ( (addr >> 1) & 0xC0 ) | CC2420_RAM_WRITE_ACCESS );\
  for (i=0;i<(len);i++)  \
  { \
    SPI1_TX((val)[i]); \
  } \
  SPI1_CC2420_DISABLE(); \
} while (0)

#endif
