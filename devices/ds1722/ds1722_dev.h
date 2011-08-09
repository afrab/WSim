
/**
 *  \file   ds1722_dev.h
 *  \brief  Dallas Maxim DS1722 digital thermometer
 *  \author Antoine Fraboulet
 *  \date   2011
 **/

#ifndef DS1722_DEVICES_H
#define DS1722_DEVICES_H

#define DS1722_CS_SHIFT  8 /* select        */
#define DS1722_SER_SHIFT 9 /* sermode       */

#define DS1722_D_MASK   0x00ff                  /** data 8 bits    **/
#define DS1722_CS_MASK  (1 << DS1722_CS_SHIFT)  /** chip select    **/
#define DS1722_SER_MASK (1 << DS1722_SER_SHIFT) /** sermode select **/

int  ds1722_add_options   (int dev_num, int dev_id, const char* dev_name);
int  ds1722_device_size   ();
int  ds1722_device_create (int dev_num, int dev_id);

#endif
