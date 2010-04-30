/**
 *  \file   logpkt.c
 *  \brief  Simulator packets logger
 *  \author Loic Lemaitre
 *  \date   2010
 **/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "logpkt.h"
#include "liblogger/logger.h"
#include "arch/common/hardware.h"


#undef UNUSED
#define UNUSED __attribute__((unused))

#define MAX_PKT_LENGTH 1000  /* maximum size of a packet           */
#define MAX_INTERFACES 5     /* maximum number of radio interfaces */

#define MAXINTERFACENAME 30
#define MAXERRORLENGTH   50

#define DEFAULT_LOGPKTFILE      stdout
#define DEFAULT_LOGPKTFILENAME "stdout"

#define LOG_RX_ONLY   0
#define LOG_TX_ONLY   1
#define LOG_RX_AND_TX 2

#define DEFAULT_LOG_MODE LOG_RX_AND_TX

#define MAXFILENAME 500

#undef DEBUG
#if defined(DEBUG)
#define LOGPKT_DBG(x...) VERBOSE(6,x) 
#else
#define LOGPKT_DBG(x...) do { } while (0)
#endif 


/* ************************************************** */
/* ************************************************** */
struct _logpkt_state_t {
  char     interface_name[MAXINTERFACENAME];
  uint8_t  current_rx_pkt[MAX_PKT_LENGTH];
  uint8_t  current_tx_pkt[MAX_PKT_LENGTH];
  int      rx_pkt_offset;
  int      tx_pkt_offset;
  int      rx_pkt_completed;
  int      tx_pkt_completed;
  char     rx_error_log[MAXERRORLENGTH];
  char     tx_error_log[MAXERRORLENGTH];
  uint64_t rx_start_time;
  uint64_t rx_end_time;
  uint64_t tx_start_time;
  uint64_t tx_end_time;
  uint32_t rx_pkt_count;
  uint32_t tx_pkt_count;
};

static struct _logpkt_state_t logpkt_tab      [MAX_INTERFACES]; /* current logpkt table             */
static struct _logpkt_state_t logpkt_tab_saved[MAX_INTERFACES]; /* saved logpkt table for backtrack */

static char  logpkt_filename[MAXFILENAME];
static FILE* logpkt_logfile;

static int nb_interfaces = 0;  /* number of different radio interfaces */

static int log_mode = DEFAULT_LOG_MODE;


/* ************************************************** */
/* ************************************************** */
static int logpkt_open_logfile(const char* filename)
{
  if (strcmp(filename, "stdout") == 0)
    {
      /* REAL_STDOUT("wsim:log:pipe:stdout\n"); */
      logpkt_logfile = stdout;
      return 0;
    }

  if (strcmp(filename, "stderr") == 0)
    {
      /* REAL_STDOUT("wsim:log:pipe:stderr\n"); */
      logpkt_logfile = stderr;
      return 0;
    }
  
  /* REAL_STDOUT("wsim:log:file:%s\n",filename); */
  return (logpkt_logfile = fopen(filename,"wb")) == NULL ;
}


/* ************************************************** */
/* ************************************************** */
void logpkt_init(int do_log_pkt, char* logpkt, const char* logpktfilename)
{
  LOGPKT_DBG("liblogpkt:init\n");

  if (do_log_pkt)
    {
      /* enable packet log */
      logpkt_init_interface  = logpkt_init_interface_op;
      logpkt_close           = logpkt_close_op;
      logpkt_rx_byte         = logpkt_rx_byte_op;
      logpkt_tx_byte         = logpkt_tx_byte_op;
      logpkt_rx_complete_pkt = logpkt_rx_complete_pkt_op;
      logpkt_tx_complete_pkt = logpkt_tx_complete_pkt_op;
      logpkt_rx_abort_pkt    = logpkt_rx_abort_pkt_op;
      logpkt_tx_abort_pkt    = logpkt_tx_abort_pkt_op;
      logpkt_state_save      = logpkt_state_save_op;
      logpkt_state_restore   = logpkt_state_restore_op;
    }
  else
    {
      /* disable packet log */
      logpkt_init_interface  = logpkt_init_interface_nop;
      logpkt_close           = logpkt_close_nop;
      logpkt_rx_byte         = logpkt_rx_byte_nop;
      logpkt_tx_byte         = logpkt_tx_byte_nop;
      logpkt_rx_complete_pkt = logpkt_rx_complete_pkt_nop;
      logpkt_tx_complete_pkt = logpkt_tx_complete_pkt_nop;
      logpkt_rx_abort_pkt    = logpkt_rx_abort_pkt_nop;
      logpkt_tx_abort_pkt    = logpkt_tx_abort_pkt_nop;
      logpkt_state_save      = logpkt_state_save_nop;
      logpkt_state_restore   = logpkt_state_restore_nop;

      return;
    }

  if (logpkt)
    {
      if (!strcmp(logpkt, "rx"))
	{
	  log_mode = LOG_RX_ONLY;
	}
      else if (!strcmp(logpkt, "tx"))
	{
	  log_mode = LOG_TX_ONLY;
	}
      else if (!strcmp(logpkt, "rxtx") || !strcmp(logpkt, "txrx"))
	{
	  log_mode = LOG_RX_AND_TX;
	}
      else
	{
	  ERROR("pktlog: wrong option, should be '--pktlog=tx', '--pktlog=rx' or '--pktlog=rxtx'\n");
	  ERROR("pktlog: log mode set to RX and TX\n");
	  log_mode = LOG_RX_AND_TX;
	}
    }

  if (logpkt_open_logfile(logpktfilename))
    {
      ERROR(" ** Cannot open logpktfile, defaulting to %s\n", DEFAULT_LOGPKTFILENAME);
      logpkt_logfile = DEFAULT_LOGPKTFILE;
      strcpy(logpkt_filename, DEFAULT_LOGPKTFILENAME);
    }
  else
    {
      strcpy(logpkt_filename,logpktfilename);
      /* REAL_STDOUT("wsim:log:%s\n",filename); */
    }
  
  fprintf(logpkt_logfile, "\n");
  fprintf(logpkt_logfile, "========================\n");
  fprintf(logpkt_logfile, "== Radios packets log ==\n");
  fprintf(logpkt_logfile, "========================\n");
  fprintf(logpkt_logfile, "\n");
  fprintf(logpkt_logfile, "Note : rx and tx packets contains preamble, sync word and data\n");
  fprintf(logpkt_logfile, "Note : rx packets are logged before crc replacement\n");
  fprintf(logpkt_logfile, "Note : time interval syntax -> [tx or rx start; tx or rx end]\n");
  fprintf(logpkt_logfile, "\n");

  switch(log_mode)
    {
    case LOG_RX_ONLY:
      fprintf(logpkt_logfile, "Log mode: RX only\n");
      break;
    case LOG_TX_ONLY:
      fprintf(logpkt_logfile, "Log mode: TX only\n");
      break;
    case LOG_RX_AND_TX:
      fprintf(logpkt_logfile, "Log mode: RX and TX\n");
      break;
    }
}


/* ************************************************** */
/* ************************************************** */
void logpkt_init_interface_op(int interface_id, const char* interface_name)
{
  LOGPKT_DBG("liblogpkt:init interface %s\n", interface_name);

  if (interface_id >= MAX_INTERFACES)
    {
      ERROR("logpkt:logpkt_interface_init: radio interface id (%d) too high\n", interface_id);
      return;
    }

  /* init variables */
  strcpy(logpkt_tab[interface_id].interface_name, interface_name);
  logpkt_tab[interface_id].rx_pkt_offset    = 0;
  logpkt_tab[interface_id].tx_pkt_offset    = 0;
  logpkt_tab[interface_id].rx_pkt_completed = 0;
  logpkt_tab[interface_id].tx_pkt_completed = 0;
  logpkt_tab[interface_id].rx_pkt_count     = 0;
  logpkt_tab[interface_id].tx_pkt_count     = 0;
  strcpy(logpkt_tab[interface_id].tx_error_log, "");

  strcpy(logpkt_tab_saved[interface_id].interface_name, interface_name);
  logpkt_tab_saved[interface_id].rx_pkt_offset    = 0;
  logpkt_tab_saved[interface_id].tx_pkt_offset    = 0;
  logpkt_tab_saved[interface_id].rx_pkt_completed = 0;
  logpkt_tab_saved[interface_id].tx_pkt_completed = 0;
  logpkt_tab_saved[interface_id].rx_pkt_count     = 0;
  logpkt_tab_saved[interface_id].tx_pkt_count     = 0;
  strcpy(logpkt_tab_saved[interface_id].tx_error_log, "");

  nb_interfaces++;

  fprintf(logpkt_logfile, "Interface %s registered\n", interface_name);
}


/* ************************************************** */
/* ************************************************** */
void logpkt_close_op(void)
{
  if ((logpkt_logfile != stdout) && (logpkt_logfile != stderr))
    {
      fclose(logpkt_logfile);
    }
}


/* ************************************************** */
/* ************************************************** */
void logpkt_rx_byte_op(int interface_id, uint8_t val)
{
  LOGPKT_DBG("liblogpkt:log rx byte : 0x%02x\n", val);

  if (logpkt_tab[interface_id].rx_pkt_offset == 0)
    {
      logpkt_tab[interface_id].rx_start_time = MACHINE_TIME_GET_NANO();
    }

  logpkt_tab[interface_id].rx_end_time = MACHINE_TIME_GET_NANO();
  logpkt_tab[interface_id].current_rx_pkt[logpkt_tab[interface_id].rx_pkt_offset] = val;
  logpkt_tab[interface_id].rx_pkt_offset++;
}


void logpkt_tx_byte_op(int interface_id, uint8_t val)
{
  LOGPKT_DBG("liblogpkt:log tx byte : 0x%02x\n", val);

  if (logpkt_tab[interface_id].tx_pkt_offset == 0)
    {
      logpkt_tab[interface_id].tx_start_time = MACHINE_TIME_GET_NANO();
    }

  logpkt_tab[interface_id].tx_end_time = MACHINE_TIME_GET_NANO();
  logpkt_tab[interface_id].current_tx_pkt[logpkt_tab[interface_id].tx_pkt_offset] = val;
  logpkt_tab[interface_id].tx_pkt_offset++;
}


/* ************************************************** */
/* ************************************************** */
void logpkt_rx_complete_pkt_op(int interface_id)
{
  LOGPKT_DBG("liblogpkt:rx complete pkt\n");
  logpkt_tab[interface_id].rx_pkt_completed = 1;
}


void logpkt_tx_complete_pkt_op(int interface_id)
{
  LOGPKT_DBG("liblogpkt:tx complete pkt\n");
  logpkt_tab[interface_id].tx_pkt_completed = 1;
}


void logpkt_rx_abort_pkt_op(int interface_id, const char* error)
{
  char error_str[MAXERRORLENGTH + 20] = " -> rx aborted: ";

  LOGPKT_DBG("liblogpkt:rx abort pkt\n");
  /* current rx frame not empty */
  if (logpkt_tab[interface_id].rx_pkt_offset)
    {
      logpkt_tab[interface_id].rx_pkt_completed = 1;
      strcpy(logpkt_tab[interface_id].rx_error_log, strcat(error_str, error));
    }
}


void logpkt_tx_abort_pkt_op(int interface_id, const char* error)
{
  char error_str[MAXERRORLENGTH + 20] = " -> tx aborted: ";

  LOGPKT_DBG("liblogpkt:tx abort pkt\n");
  /* current tx frame not empty */
  if (logpkt_tab[interface_id].tx_pkt_offset)
    {
      logpkt_tab[interface_id].tx_pkt_completed = 1;
      strcpy(logpkt_tab[interface_id].tx_error_log, strcat(error_str, error));
    }
}

/* ************************************************** */
/* ************************************************** */
static void logpkt_rx_dump_pkt(int interface_id)
{
  int i;

  /* current rx frame not empty */
  if (logpkt_tab[interface_id].rx_pkt_offset && log_mode != LOG_TX_ONLY)
    {
      fprintf(logpkt_logfile,"\n");
      fprintf(logpkt_logfile,"******* %s: RX packet (n°%d) [%"PRIu64"ns; %"PRIu64"ns] *******\n",
	      logpkt_tab[interface_id].interface_name,
	      logpkt_tab[interface_id].rx_pkt_count,
	      logpkt_tab[interface_id].rx_start_time,
	      logpkt_tab[interface_id].rx_end_time);

      for (i = 0; i < logpkt_tab[interface_id].rx_pkt_offset - 1; i++)
	{
	  fprintf(logpkt_logfile,"%02x:", logpkt_tab[interface_id].current_rx_pkt[i]);
	}
      fprintf(logpkt_logfile,"%02x", logpkt_tab[interface_id].current_rx_pkt[logpkt_tab[interface_id].rx_pkt_offset - 1]);
      fprintf(logpkt_logfile,"%s\n", logpkt_tab[interface_id].rx_error_log);

      logpkt_tab[interface_id].rx_pkt_count++;
    }
}


static void logpkt_tx_dump_pkt(int interface_id)
{
  int i;

  /* current tx frame not empty */
  if (logpkt_tab[interface_id].tx_pkt_offset && log_mode != LOG_RX_ONLY)
    {
      fprintf(logpkt_logfile,"\n");
      fprintf(logpkt_logfile,"******* %s: TX packet (n°%d) [%"PRIu64"ns; %"PRIu64"ns] *******\n",
	      logpkt_tab[interface_id].interface_name,
	      logpkt_tab[interface_id].tx_pkt_count,
	      logpkt_tab[interface_id].tx_start_time,
	      logpkt_tab[interface_id].tx_end_time);

      for (i = 0; i < logpkt_tab[interface_id].tx_pkt_offset - 1; i++)
	{
	  fprintf(logpkt_logfile,"%02x:", logpkt_tab[interface_id].current_tx_pkt[i]);
	}
      fprintf(logpkt_logfile,"%02x", logpkt_tab[interface_id].current_tx_pkt[logpkt_tab[interface_id].tx_pkt_offset - 1]);
      fprintf(logpkt_logfile,"%s\n", logpkt_tab[interface_id].tx_error_log);

      logpkt_tab[interface_id].tx_pkt_count++;
    }
}

/* ************************************************** */
/* ************************************************** */
void logpkt_state_save_op(void)
{
  int i;
  
  for (i = 0; i < nb_interfaces; i++)
    {
      if (logpkt_tab[i].rx_pkt_completed)
	{
	  logpkt_rx_dump_pkt(i);
	  logpkt_tab[i].rx_pkt_completed = 0;
	  logpkt_tab[i].rx_pkt_offset    = 0;
	  strcpy(logpkt_tab[i].rx_error_log, "");
	}
      if (logpkt_tab[i].tx_pkt_completed)
	{
	  logpkt_tx_dump_pkt(i);
	  logpkt_tab[i].tx_pkt_completed = 0;
	  logpkt_tab[i].tx_pkt_offset    = 0;
	  strcpy(logpkt_tab[i].tx_error_log, "");
	}
    }

  memcpy(logpkt_tab_saved, logpkt_tab, nb_interfaces * sizeof(struct _logpkt_state_t));
}


void logpkt_state_restore_op(void)
{
  memcpy(logpkt_tab, logpkt_tab_saved, nb_interfaces * sizeof(struct _logpkt_state_t));
}


/* ************************************************** */
/* ************************************************** */
/* Empty functions, when log packets disable          */
/* ************************************************** */
/* ************************************************** */
void logpkt_init_interface_nop(int UNUSED interface_id, const UNUSED char* interface_name)
{
}


/* ************************************************** */
/* ************************************************** */
void logpkt_close_nop(void)
{
}


/* ************************************************** */
/* ************************************************** */
void logpkt_rx_byte_nop(int UNUSED interface_id, uint8_t UNUSED val)
{
}


void logpkt_tx_byte_nop(int UNUSED interface_id, uint8_t UNUSED val)
{
}


/* ************************************************** */
/* ************************************************** */
void logpkt_rx_complete_pkt_nop(int UNUSED interface_id)
{
}


void logpkt_tx_complete_pkt_nop(int UNUSED interface_id)
{
}


void logpkt_rx_abort_pkt_nop(int UNUSED interface_id, const UNUSED char* error)
{
}


void logpkt_tx_abort_pkt_nop(int UNUSED interface_id, const UNUSED char* error)
{
}


/* ************************************************** */
/* ************************************************** */
void logpkt_state_save_nop(void)
{
}


void logpkt_state_restore_nop(void)
{
}
