
/**
 *  \file   options.h
 *  \brief  WSim command line options
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef WSIM_PARAMS_H
#define WSIM_PARAMS_H

#include <stdint.h>

enum sim_mode_t {
  SIM_MODE_CONS  = 0,
  SIM_MODE_GDB   = 1,
  SIM_MODE_RUN   = 2,
  SIM_MODE_INSN  = 3,
  SIM_MODE_TIME  = 4
};

enum wsens_mode_t {
  WS_MODE_WSNET0 = 0,
  WS_MODE_WSNET1 = 1,
  WS_MODE_WSNET2 = 2
};

#include "mgetopt.h"

#define MAX_FILENAME  256

/**
 * WSim command line options
 **/
struct options_t {
  int                verbose;

  char               logfilename   [MAX_FILENAME];
  char               logpktfilename[MAX_FILENAME];
  char               progname      [MAX_FILENAME]; 
  char               dumpfile      [MAX_FILENAME];
  char               tracefile     [MAX_FILENAME];
  char               etracefile    [MAX_FILENAME];
  char               preload       [MAX_FILENAME];

  int                do_dump;
  int                do_trace;
  int                do_etrace;
  int                do_preload;
  int                do_elfload;
  int                do_etrace_at_begin;

  int                do_logpkt;
  char              *logpkt;

  int                do_monitor;
  char              *monitor;
  int                do_modify;
  char              *modify;

  uint32_t           node_id;
  char               server_ip   [MAX_FILENAME];
  uint16_t           server_port;
  char               multicast_ip[MAX_FILENAME];
  uint16_t           multicast_port;

  /**
   * bool
   */
  enum sim_mode_t    sim_mode;
  enum wsens_mode_t  wsens_mode;
  unsigned short     gdb_port;
  uint64_t           sim_insn;
  uint64_t           sim_time;
  int                realtime;
};


void options_start         ();
void options_add           (struct moption_t *o);

void options_read_cmdline  (struct options_t* s, int *argc, char *argv[]);
void options_print_params  (struct options_t *s);

#endif
