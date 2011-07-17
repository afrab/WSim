/*
 *  simulation.c
 *  
 *
 *  Created by Guillaume Chelius on 01/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#include <signal.h>

#include <private/core_private.h>
#include <private/sim_conf.h>
#include <private/command_line.h>
#include <private/log_private.h>

#include "libtracer/tracer.h"
#include "liblogger/logger.h"

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int g_m_nodes = -1;
int g_c_nodes = 0;
	
double g_x    = -1.0;
double g_y    = -1.0;
double g_z    = -1.0;

struct _worldsens_s worldsens;



uint64_t g_sim_time = 0;

uint64_t get_global_time()               
{ 
  return g_sim_time; 
}

void set_global_time(uint64_t time)  
{ 
  g_sim_time = time; 
  WSNET_S_DBG_TIME ("WSNET:: ===\n");
  OUTPUT ("WSNET:: === TIME  %"PRId64" (%.2fms, seq: %d)\n", time, (float)time / 1000000.0, worldsens.rp_seq);
  WSNET_S_DBG_TIME ("WSNET:: ===\n");
  tracer_state_save();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#define SIG_NAME_MAX 30
char* unix_signal_to_str(int sig)
{
  static char sig_unknown[SIG_NAME_MAX]; 

  switch (sig)
    {
    case 0       : return "";
    case SIGINT  : return "SIGINT";
    case SIGILL  : return "SIGILL";
    case SIGABRT : return "SIGABRT";
    case SIGFPE  : return "SIGFPE";
    case SIGSEGV : return "SIGSEGV";
    case SIGTERM : return "SIGTERM";
#if defined(SIGHUP)
    case SIGHUP  : return "SIGHUP";
    case SIGQUIT : return "SIGQUIT";
    case SIGKILL : return "SIGKILL";
    case SIGPIPE : return "SIGPIPE";
    case SIGUSR1 : return "SIGUSR1";      
    case SIGUSR2 : return "SIGUSR2";      
    case SIGALRM : return "SIGALRM";
    case SIGSTOP : return "SIGSTOP";
    case SIGTSTP : return "SIGTSTP";
    case SIGCONT : return "SIGCONT";
#endif
    default      : 
      sprintf(sig_unknown,"unknown %d",sig);
      return sig_unknown;
    }
}

char* host_signal_str(int sig)
{
  static char buff[SIG_NAME_MAX];

#if  0 /* defined(FUNC_STRSIGNAL_DEFINED) */
  strcpy(buff,strsignal(sig));
#else
  strcpy(buff,unix_signal_to_str(sig));
#endif

  return buff;
}

void signal_quit(int signum)
{
  WARNING("wsnet1: simulation stopped, received Unix signal %d (%s)\n",signum,host_signal_str(signum));
  simulation_keeps_going = 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

int simulation_start(int argc, char* argv[]) 
{
#if !defined(__MINGW32__)
  signal(SIGINT ,signal_quit);
  signal(SIGQUIT,signal_quit);
  signal(SIGUSR1,signal_quit);
  signal(SIGUSR2,signal_quit);
  signal(SIGPIPE,signal_quit); 
#endif

  logger_init("stdout",4);
  tracer_init("wsnet1.trc", 0);
  tracer_set_timeref(get_global_time);
  tracer_start();

  if (command_line (argc, argv)) 
    {
      return -1;
    }
  
  if (sim_config ()) 
    {
      return -1;
    }

  if (worldsens_s_initialize(&worldsens))
    {
      return -1;
    }

  core_start(&worldsens);

  tracer_close(); 
  logger_close();

  if (g_log_to_file) 
    {
      fclose(g_log_file);
    }

  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
