
/**
 *  \file   cc2420_dev.h
 *  \brief  CC2420 device model entry point
 *  \author Nicolas Boulicault, Antoine Fraboulet
 *  \date   2006
 **/

/*
 *  cc2420_dev.h
 *  
 *
 *  Created by Nicolas Boulicault
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *  Modified Antoine Fraboulet march 2008
 */

#ifndef CC2420_DEVICES_H
#define CC2420_DEVICES_H


/***************************************************/
/***************************************************/
/***************************************************/

/* 
 * . . | . . . .  . . . . | . . . .  . . . . 
 *                          x x x x  x x x x  DATA     ...ff
 *
 *                       X                    CSn      ..1..
 *                     X                      FIFO     ..2..
 *                   X                        FIFOP    ..4..
 *                 X                          CCA      ..8..
 * 
 *              X                             SFD      .1...
 *            X                               RESET    .2...
 *          X                                 VREG_EN  .4...
 *        X                                   GIO0     .8...
 * 
 *   X                                        GIO1     1....
 * X                                          PKT_INT  2....
 *
 */

#define CC2420_BIT_SPI       0
#define CC2420_BIT_CSn       8
#define CC2420_BIT_FIFO      9
#define CC2420_BIT_FIFOP    10
#define CC2420_BIT_CCA      11
#define CC2420_BIT_SFD      12
#define CC2420_BIT_RESET    13
#define CC2420_BIT_VREG_EN  14
#define CC2420_BIT_GDO0     15
#define CC2420_BIT_GDO2     16
#define CC2420_BIT_PKT_INT  17

#define CC2420_DATA_MASK    0x000000ff
#define CC2420_CSn_MASK     (1 << CC2420_BIT_CSn)
#define CC2420_FIFO_MASK    (1 << CC2420_BIT_FIFO)
#define CC2420_FIFOP_MASK   (1 << CC2420_BIT_FIFOP)
#define CC2420_CCA_MASK     (1 << CC2420_BIT_CCA)
#define CC2420_SFD_MASK     (1 << CC2420_BIT_SFD)
#define CC2420_RESET_MASK   (1 << CC2420_BIT_RESET)
#define CC2420_VREG_EN_MASK (1 << CC2420_BIT_VREG_EN)
#define CC2420_GDO0_MASK    (1 << CC2420_BIT_GDO0)
#define CC2420_GDO2_MASK    (1 << CC2420_BIT_GDO2)
#define CC2420_PKT_INT_MASK (1 << CC2420_BIT_PKT_INT)

/***************************************************/
/***************************************************/
/***************************************************/

int cc2420_device_create  (int dev_num, int fxocs_mhz);
int cc2420_device_size    (void);

/***************************************************/
/***************************************************/
/***************************************************/

#endif
