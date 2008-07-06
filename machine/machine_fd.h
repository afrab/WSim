
/**
 *  \file   machine_fd.h
 *  \brief  Platform Machine forward definitions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef HW_MACHINE_H
#define HW_MACHINE_H

#define DEVICE_MAX 20

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
  the mcu id defined in the associated lib, it should
  not be accessed outside of it interface.
  The interface is defined thanks to functions with
  only known type parameters.
 */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * machine description
 **/
struct machine_t
{
  uint64_t        nanotime;
  int             nanotime_incr;
  uint64_t        nanotime_backup;

  /**
   * devices state with backup
   **/
  uint8_t*        devices_state;
  uint8_t*        devices_state_backup;
  int             devices_state_size;
  
  /**
   * peripherals connected to the mcu
   **/
  struct device_t device     [DEVICE_MAX];
  int             device_size[DEVICE_MAX];
  int             device_max;

  /**
   * trace log
   **/
  uint32_t        backtrack;

  /**
   * ui
   **/
  struct ui_t   ui;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * global variable storing the emulator machine state
 **/
extern struct machine_t machine;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define MACHINE_TIME_GET_NANO()  machine.nanotime
#define MACHINE_TIME_GET_INCR()  machine.nanotime_incr
#define MACHINE_TIME_SET_INCR(n)                                \
  do {                                                          \
    machine.nanotime_incr = n;                                  \
    machine.nanotime     += n;                                  \
    HW_DMSG_TIME("msp430:time: %"PRId64"\n", machine.nanotime);	\
  } while (0)

#define MACHINE_TIME_CLR_INCR()  do { machine.nanotime_incr = 0; } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
