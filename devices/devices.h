
/**
 *  \file   devices.h
 *  \brief  Platform devices and handling functions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef DEVICES_H
#define DEVICES_H

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define MAP_OVER_DEVICES(FUNC)                   \
  {                                              \
      int i;                                     \
      for(i=0; i < machine.device_max; i++)      \
        {                                        \
             res += machine.device[i].FUNC (i);  \
        }                                        \
  }

#define UPDATE(d)  machine.device[ d ].update( d )

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(GUI)
#  define REFRESH(d) do { refresh |= machine.device[ d ].ui_draw( d ); } while (0)
#else
#  define REFRESH(d) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
 
#define BIT0_MASK  0x01
#define BIT1_MASK  0x02
#define BIT2_MASK  0x04
#define BIT3_MASK  0x08
#define BIT4_MASK  0x10
#define BIT5_MASK  0x20
#define BIT6_MASK  0x40
#define BIT7_MASK  0x80

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add(void);

/**
 * create all devices
 **/
int devices_create(void); 
int devices_delete(void); 
int devices_memory_allocate(void);

/**
 * devices_reset
 * reset all devices
 **/
int devices_reset(void);
int devices_reset_post(void);

/**
 * devices_update
 * update all devices
 * done after writes but before read
 **/
int devices_update(void);

/**
 * devices_print
 * print a summary of all connected devices
 **/
void devices_print(void);

/**
 * devices_dump_stats
 * print summary stats at end of simulation
 */
void devices_dump_stats(int64_t user_time);

#endif
