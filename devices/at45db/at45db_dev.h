
/**
 *  \file   at45db_dev.h
 *  \brief  at45db flash memory module
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef AT45DB_DEVICES_H
#define AT45DB_DEVICES_H

#define AT45DB_W_SHIFT    8 /* write protect */
#define AT45DB_S_SHIFT    9 /* select        */
#define AT45DB_R_SHIFT   10 /* reset         */
#define AT45DB_C_SHIFT   11 /* clock         */
#define AT45DB_RDY_SHIFT 12 /* READY/~BUSY   */

#define AT45DB_D   0x00ff                   /** data 8 bits           **/
#define AT45DB_W   (1 << AT45DB_W_SHIFT)    /** write protect negated **/
#define AT45DB_S   (1 << AT45DB_S_SHIFT)    /** chip select negated   **/
#define AT45DB_R   (1 << AT45DB_R_SHIFT)    /** reset negated         **/
#define AT45DB_C   (1 << AT45DB_C_SHIFT)    /** clock                 **/
#define AT45DB_RDY (1 << AT45DB_RDY_SHIFT)  /** READY/~BUSY           **/

int  at45db_add_options   (int dev_num, int dev_id, const char* dev_name);
int  at45db_device_size   ();
int  at45db_device_create (int dev_num, int dev_id);

#endif
