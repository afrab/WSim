/*
 *  cc1100_spi.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Modified by Antoine Fraboulet on 04/04/07
 *  Copyright 2005,2006,2007 __WorldSens__. All rights reserved.
 *
 */
#ifndef _CC1100_SPI_H
#define _CC1100_SPI_H

#include "cc1100_globals.h"

extern volatile uint8_t cc1100_status_register;
extern volatile uint8_t cc1100_intr; 

#define CC1100_SPI_STROBE(s)                    \
do {                                            \
	CC1100_HW_GLOBAL_DINT();                \
	CC1100_SPI_ENABLE();                    \
	CC1100_SPI_TX(s | CC1100_REG_ACCESS_OP_WRITE | CC1100_REG_ACCESS_NOBURST);\
	CC1100_SPI_DISABLE();                   \
	CC1100_HW_GLOBAL_EINT();                \
} while (0)

#define CC1100_SPI_WREG(a,v)                    \
do {                                            \
	CC1100_HW_GLOBAL_DINT();                \
	CC1100_SPI_ENABLE();                    \
	CC1100_SPI_TX(a | CC1100_REG_ACCESS_OP_WRITE | CC1100_REG_ACCESS_NOBURST);\
	CC1100_SPI_TX(v);                       \
	CC1100_SPI_DISABLE();                   \
	CC1100_HW_GLOBAL_EINT();                \
} while (0)

#define CC1100_SPI_RREG(a,v)                    \
do {                                            \
	CC1100_HW_GLOBAL_DINT();                \
	CC1100_SPI_ENABLE();                    \
	CC1100_SPI_TX(a | CC1100_REG_ACCESS_OP_READ | CC1100_REG_ACCESS_NOBURST);\
	CC1100_SPI_RX(v);                       \
	CC1100_SPI_DISABLE();                   \
	CC1100_HW_GLOBAL_EINT();                \
} while (0)

#define CC1100_SPI_ROREG(a,v)                   \
do {                                            \
	CC1100_HW_GLOBAL_DINT();                \
	CC1100_SPI_ENABLE();                    \
	CC1100_SPI_TX(a | CC1100_REG_ACCESS_OP_READ | CC1100_REG_ACCESS_BURST);\
	CC1100_SPI_RX(v);                       \
	CC1100_SPI_DISABLE();                   \
	CC1100_HW_GLOBAL_EINT();                \
} while (0)




#define CC1100_SPI_TX_BYTE(addr, val)                    \
do {                                                     \
	CC1100_HW_GLOBAL_DINT();                         \
	CC1100_SPI_ENABLE();                             \
	CC1100_SPI_TX(addr);                             \
	CC1100_SPI_TX(val);                              \
	CC1100_SPI_DISABLE();                            \
	CC1100_HW_GLOBAL_EINT();                         \
} while (0)

#define CC1100_SPI_RX_BYTE(addr, val)                    \
do {                                                     \
	CC1100_HW_GLOBAL_DINT();                         \
	CC1100_SPI_ENABLE();                             \
	CC1100_SPI_TX(addr | CC1100_REG_ACCESS_OP_READ); \
	CC1100_SPI_RX(val);                              \
	CC1100_SPI_DISABLE();                            \
	CC1100_HW_GLOBAL_EINT();                         \
} while (0)




#define CC1100_SPI_TX_BURST(addr, val, len)              \
do {                                                     \
	uint8_t cnt = 0;                                 \
	CC1100_HW_GLOBAL_DINT();                         \
	CC1100_SPI_ENABLE();                             \
	CC1100_SPI_TX(addr | CC1100_REG_ACCESS_BURST);   \
	for (cnt = 0; cnt < (len); cnt++)                \
          {                                              \
	    CC1100_SPI_TX(val [cnt]);                    \
	  }                                              \
	CC1100_SPI_DISABLE();                            \
	CC1100_HW_GLOBAL_EINT();                         \
} while (0)

#define CC1100_SPI_RX_BURST(addr,val,len)                \
do {                                                     \
	uint8_t cnt = 0;                                 \
	CC1100_HW_GLOBAL_DINT();                         \
	CC1100_SPI_ENABLE();                             \
	CC1100_SPI_TX(addr | CC1100_REG_ACCESS_OP_READ | CC1100_REG_ACCESS_BURST); \
	for (cnt = 0; cnt < (len); cnt++)                \
          {                                              \
	    CC1100_SPI_RX(val [cnt]);                    \
          }                                              \
	CC1100_SPI_DISABLE();                            \
	CC1100_HW_GLOBAL_EINT();                         \
} while (0)


#define CC1100_SPI_TX_FIFO_BYTE(val)      CC1100_SPI_TX_BYTE(CC1100_DATA_FIFO_ADDR,val)
#define CC1100_SPI_TX_FIFO_BURST(val,len) CC1100_SPI_TX_BURST(CC1100_DATA_FIFO_ADDR,val,len)

#define CC1100_SPI_RX_FIFO_BYTE(val)      CC1100_SPI_RX_BYTE(CC1100_DATA_FIFO_ADDR,val)
#define CC1100_SPI_RX_FIFO_BURST(val,len) CC1100_SPI_RX_BURST(CC1100_DATA_FIFO_ADDR,val,len)

/****************************************************************************/
//                                                                          
//                 min 40 us
//                 <------------------>
// CSn      |--|  |--------------------|            |-----
//          |  |  |                    |            |
//              --                      ------------
//
// MISO                                     |----|
//          -----------------------------|  |    |
//                                        --      ---------
//               Unknown / don't care
//                                       SRES     done
//
/****************************************************************************/

#define CC1100_SPI_POWER_UP_RESET()               \
do {                                              \
/* low */   CC1100_SPI_ENABLE();                  \
/* high */  CC1100_SPI_DISABLE();                 \
            CC1100_HW_MICRO_WAIT(40);             \
/* low */   CC1100_SPI_ENABLE();                  \
	    while (CC1100_HW_CHECK_MISO_HIGH());  \
	    CC1100_SPI_RESET();                   \
	    while (CC1100_HW_CHECK_MISO_HIGH());  \
} while (0)



#endif
