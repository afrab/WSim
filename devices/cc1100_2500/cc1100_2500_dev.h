
/**
 *  \file   cc1100_2500_dev.h
 *  \brief  CC1100/CC2500 device model entry point
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_2500_dev.h
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *  Modified Antoine Fraboulet Jan 2008
 */

#ifndef _CC1100_DEV_H
#define _CC1100_DEV_H


/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_DATA_SHIFT         0
#define CC1100_CSn_SHIFT          8
#define CC1100_SO_SHIFT           9
#define CC1100_SI_SHIFT          10
#define CC1100_CLK_SHIFT         11
#define CC1100_GDO0_SHIFT        12
#define CC1100_GDO1_SHIFT        13
#define CC1100_GDO2_SHIFT        14

#define CC1100_DATA_MASK         0xff
#define CC1100_CSn_MASK          (1 << CC1100_CSn_SHIFT)
#define CC1100_SO_MASK           (1 << CC1100_SO_SHIFT)
#define CC1100_SI_MASK           (1 << CC1100_SI_SHIFT)
#define CC1100_CLK_MASK          (1 << CC1100_CLK_SHIFT)
#define CC1100_GDO0_MASK         (1 << CC1100_GDO0_SHIFT)
#define CC1100_GDO1_MASK         (1 << CC1100_GDO1_SHIFT)
#define CC1100_GDO2_MASK         (1 << CC1100_GDO2_SHIFT)


#if defined(CC2500)

#define CC2500_DATA_SHIFT        CC1100_DATA_SHIFT
#define CC2500_CSn_SHIFT         CC1100_CSn_SHIFT
#define CC2500_SO_SHIFT          CC1100_SO_SHIFT
#define CC2500_SI_SHIFT          CC1100_SI_SHIFT
#define CC2500_CLK_SHIFT         CC1100_CLK_SHIFT
#define CC2500_GDO0_SHIFT        CC1100_GDO0_SHIFT
#define CC2500_GDO1_SHIFT        CC1100_GDO1_SHIFT
#define CC2500_GDO2_SHIFT        CC1100_GDO2_SHIFT

#define CC2500_DATA_MASK         CC1100_DATA_MASK
#define CC2500_CSn_MASK          CC1100_CSn_MASK
#define CC2500_SO_MASK           CC1100_SO_MASK
#define CC2500_SI_MASK           CC1100_SI_MASK
#define CC2500_CLK_MASK          CC1100_CLK_MASK
#define CC2500_GDO0_MASK         CC1100_GDO0_MASK
#define CC2500_GDO1_MASK         CC1100_GDO1_MASK
#define CC2500_GDO2_MASK         CC1100_GDO2_MASK

#endif

/***************************************************/
/***************************************************/
/***************************************************/

#if defined(CC1100)
int cc1100_device_create (int dev_num, int fxosc_mhz, char *antenna);
int cc1100_device_size   (void);
#elif defined(CC2500)
int cc2500_device_create (int dev_num, int fxosc_mhz, char *antenna);
int cc2500_device_size   (void);
#else
#error "you must define CC1100 or CC2500 model"
#endif

/***************************************************/
/***************************************************/
/***************************************************/

#endif //_CC1100_DEV_H
