
/**
 *  \file   at45db_dev.h
 *  \brief  at45db flash memory module
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef AT45DB_DEVICES_H
#define AT45DB_DEVICES_H

#define AT45DB_W_SHIFT  8 /* write protect */
#define AT45DB_S_SHIFT  9 /* select        */
#define AT45DB_R_SHIFT 10 /* reset         */
#define AT45DB_C_SHIFT 11 /* clock         */

/** data 8 bits           **/
#define AT45DB_D  0x00ff

/** write protect negated **/
#define AT45DB_W  (1 << AT45DB_W_SHIFT)

/** chip select negated   **/
#define AT45DB_S  (1 << AT45DB_S_SHIFT)

/** reset negated         **/
#define AT45DB_R  (1 << AT45DB_R_SHIFT)

/** clock                 **/
#define AT45DB_C  (1 << AT45DB_C_SHIFT)


int  at45db_device_create (int dev_num, const char *init, const char *dump);
int  at45db_device_size   ();

#endif
