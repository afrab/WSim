
/**
 *  \file   cc1100_dev.h
 *  \brief  CC1100 device model entry point
 *  \author Guillaume Chelius
 *  \date   2006
 **/

/*
 *  cc1100_dev.h
 *  Created by Guillaume Chelius on 16/02/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *  Modified Antoine Fraboulet Jan 2008
 */

#ifndef _CC1100_DEV_H
#define _CC1100_DEV_H


/***************************************************/
/***************************************************/
/***************************************************/

#define CC1100_INTERNAL_CSn_PIN   0
#define CC1100_INTERNAL_SI_PIN    1
#define CC1100_INTERNAL_GO0_PIN   2
#define CC1100_INTERNAL_GO1_PIN   3
#define CC1100_INTERNAL_GO2_PIN   4

#define CC1100_DATA_SHIFT         0
#define CC1100_CSn_SHIFT          8
#define CC1100_SI_SHIFT           9
#define CC1100_GDO0_SHIFT        10
#define CC1100_GDO1_SHIFT        11
#define CC1100_GDO2_SHIFT        12

#define CC1100_DATA_MASK         0xff
#define CC1100_CSn_MASK          (1 << CC1100_CSn_SHIFT)
#define CC1100_SI_MASK           (1 << CC1100_SI_SHIFT)
#define CC1100_GDO0_MASK         (1 << CC1100_GDO0_SHIFT)
#define CC1100_GDO1_MASK         (1 << CC1100_GDO1_SHIFT)
#define CC1100_GDO2_MASK         (1 << CC1100_GDO2_SHIFT)

/***************************************************/
/***************************************************/
/***************************************************/

int cc1100_device_create (int dev_num, int fxosc_mhz);
int cc1100_device_size   (void);

/***************************************************/
/***************************************************/
/***************************************************/

#endif //_CC1100_DEV_H
