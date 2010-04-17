
/**
 *  \file   spidev_dsp_dev.h
 *  \brief  SPI DSP device example
 *  \author Loic Lemaitre, Antoine Fraboulet
 *  \date   2010
 **/

#ifndef SPIDEV_DSP_DEVICES_H
#define SPIDEV_DSP_DEVICES_H

#define SPIDEV_DSP_W_SHIFT  8 /* write protect */
#define SPIDEV_DSP_S_SHIFT  9 /* select        */
#define SPIDEV_DSP_M_SHIFT 10 /* SPI mode      */
#define SPIDEV_DSP_C_SHIFT 11 /* clock         */

#define SPIDEV_DSP_D  0x00ff                     /** data 8 bits           **/
#define SPIDEV_DSP_W  (1 << SPIDEV_DSP_W_SHIFT)  /** write protect negated **/
#define SPIDEV_DSP_S  (1 << SPIDEV_DSP_S_SHIFT)  /** chip select negated   **/
#define SPIDEV_DSP_M  (1 << SPIDEV_DSP_M_SHIFT)  /** spi mode              **/
#define SPIDEV_DSP_C  (1 << SPIDEV_DSP_C_SHIFT)  /** clock                 **/

int  spidev_dsp_add_options   (int dev_num, int dev_id, const char *name);
int  spidev_dsp_device_size   ();
int  spidev_dsp_device_create (int dev_num, int dev_id);

#endif
