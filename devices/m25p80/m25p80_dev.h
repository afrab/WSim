
/**
 *  \file   m25p80_dev.h
 *  \brief  STmicro m25p flash memory modules
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef M25P_DEVICES_H
#define M25P_DEVICES_H

#define M25P_W_SHIFT  8 /* write protect */
#define M25P_S_SHIFT  9 /* select        */
#define M25P_H_SHIFT 10 /* hold          */
#define M25P_C_SHIFT 11 /* clock         */

/** data 8 bits           **/
#define M25P_D  0x00ff

/** write protect negated **/
#define M25P_W  (1 << M25P_W_SHIFT)

/** chip select negated   **/
#define M25P_S  (1 << M25P_S_SHIFT)

/** hold negated          **/
#define M25P_H  (1 << M25P_H_SHIFT)

/** clock                 **/
#define M25P_C  (1 << M25P_C_SHIFT)


int  m25p_device_create (int dev_num, const char *init, const char *dump);
int  m25p_device_size   ();

#endif
