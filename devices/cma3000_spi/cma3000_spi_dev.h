/**
 *  \file   cma3000_spi_dev.h
 *  \brief  CMA3000 Accel sensor in SPI Mode
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef CMA3000_SPI_DEV_H
#define CMA3000_SPI_DEV_H

#define CMA3000_SPI_W_SHIFT  8 /* write protect */
#define CMA3000_SPI_CSb_SHIFT  9 /* select        */
#define CMA3000_SPI_M_SHIFT 10 /* MISO          */
#define CMA3000_SPI_C_SHIFT 11 /* clock         */
#define CMA3000_SPI_INT_SHIFT 12 /* INT         */

#define CMA3000_SPI_D  0x00ff                 /** data 8 bits           **/
#define CMA3000_SPI_W  (1 << CMA3000_SPI_W_SHIFT)  /** write protect negated **/
#define CMA3000_SPI_CSb  (1 << CMA3000_SPI_CSb_SHIFT)  /** chip select negated   **/
#define CMA3000_SPI_M  (1 << CMA3000_SPI_M_SHIFT)  /** miso                  **/
#define CMA3000_SPI_C  (1 << CMA3000_SPI_C_SHIFT)  /** clock                 **/
#define CMA3000_SPI_INT_MASK  (1 << CMA3000_SPI_INT_SHIFT)  /** INT                   **/

/* register addresses */
#define CMA3000_SPI_WHO_AM_I    0x00
#define CMA3000_SPI_REVID       0x01
#define CMA3000_SPI_CTRL        0x02
#define CMA3000_SPI_STATUS      0x03
#define CMA3000_SPI_RSTR        0x04
#define CMA3000_SPI_INT_STATUS  0x05
#define CMA3000_SPI_DOUTX       0x06
#define CMA3000_SPI_DOUTY       0x07
#define CMA3000_SPI_DOUTZ       0x08
#define CMA3000_SPI_MDTHR       0x09
#define CMA3000_SPI_MDFFTMR     0x0A
#define CMA3000_SPI_FFTHR       0x0B
#define CMA3000_SPI_I2C_ADDR    0x0C

int cma3000_spi_add_options(int dev_num, int dev_id, const char *name);
int cma3000_spi_device_size();
int cma3000_spi_device_create(int dev_num, int dev_id);

#endif
