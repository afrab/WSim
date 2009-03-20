
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

#include "mgetopt.h"

#define MAX_FILENAME 256

/**
 * WSim command line options
 **/
struct options_t {
  int                verbose;

  char               logfilename[MAX_FILENAME];
  char               progname   [MAX_FILENAME]; 
  char               dumpfile   [MAX_FILENAME];
  char               tracefile  [MAX_FILENAME];
  char               etracefile [MAX_FILENAME];
  char               precharge  [MAX_FILENAME];

  int                do_dump;
  int                do_trace;
  int                do_etrace;
  int                do_precharge;
  int                do_etrace_at_begin;

  /**
   * bool
   */
  enum sim_mode_t    sim_mode;
  unsigned short     gdb_port;
  uint64_t           sim_insn;
  uint64_t           sim_time;
};


void options_start         ();
void options_add           (struct moption_t *o);

void options_read_cmdline  (struct options_t* s, int *argc, char *argv[]);
void options_print_version (struct options_t *s);
void options_print_params  (struct options_t *s);

#endif
