
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

#define MONITOR_DEFAULT_SIZE         1
#define MONITOR_MAX_VARIABLE_NAME  100
#define MONITOR_MAX_WATCHPOINT     512

struct watchpoint_t {
  char        name[MONITOR_MAX_VARIABLE_NAME];
  uint32_t    addr;
  int         size;                      /* bytes                            */
  int         mode;                      /* MAC_WATCH_WRITE | MAC_WATCH_READ */
  /*int       modify_on_first_write;*/   /* backtracked */ 
  int         modify_value; 
  /* trace */
  tracer_id_t trc_id_val;
  tracer_id_t trc_id_rw;
};

struct machine_state_t {
  wsimtime_t    nanotime;
  wsimtime_t    nanotime_incr;
  char          watchpoint_modify_on_first_write[MONITOR_MAX_WATCHPOINT];
  /* devices_state must be last */
  uint8_t       devices_state[0];
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 * machine description
 **/
struct machine_t
{
  /**
   * machine+devices state with backup
   **/
  struct machine_state_t*  state;
  struct machine_state_t*  state_backup;
  int                      state_size;

  /**
   * peripherals connected to the mcu
   **/
  struct device_t          device     [DEVICE_MAX];
  int                      device_size[DEVICE_MAX];
  int                      device_max;

  /**
   *
   **/
  int                      nanotime_incr;
  int                      run_limit;
  int                      run_realtime;
  wsimtime_t               run_time;
  uint64_t                 run_insn;

  /**
   * trace log
   **/
  uint32_t                 backtrack;
  tracer_id_t              backtrack_trc;
  tracer_id_t              realtime_trc;
  tracer_id_t              logwrite_trc;

  /**
   * ui
   **/
  struct ui_t              ui;
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

void     machine_exit      (int arg);

#define MACHINE_TIME_GET_NANO()  machine.state->nanotime
#define MACHINE_TIME_GET_INCR()  machine.nanotime_incr
#define MACHINE_TIME_SET_INCR(n)				       \
  do {								       \
    machine.nanotime_incr        = n;                                  \
    machine.state->nanotime     += n;                                  \
  } while (0)

#define MACHINE_TIME_CLR_INCR()  do { machine.nanotime_incr = 0; } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
