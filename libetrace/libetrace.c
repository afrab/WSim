
/**
 *  \file   libetrace.c
 *  \brief  WSim energy tracer for eSimu
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "liblogger/logger.h"
#include "libetrace.h"
#include "src/options.h"

#if defined(ETRACE)

#include "etrace.h"

#define ETRACER_FILE_MODE   ETRACE_MODE_ZBINARY
#define ETRACER_WORD_SIZE                     2
#define ETRACER_MAX_MISS                      0  /* cache+tlb miss in 1 slot */
#define ETRACER_MAX_ACCESS                    9  /* memory access in 1 slot */
#define ETRACER_MAX_PER_EVTS                  5  /* peripheral events in 1 slot */
#define ETRACER_WRITE_BUFFER      (5*1024*1024)  /* bytes */

/****************************************
 * DEBUG
 * 
 * DMSG is used for general tracer messages
 * DMSG_TRACER is used for event recording messages
 * ERROR is used for ... errors
 ****************************************/

#if defined(DEBUG)
#define DMSG(x...) HW_DMSG(x)
#else
#define DMSG(x...) do {} while(0)
#endif

#if defined(DEBUG)
#define DMSG_ETRACER(x...) DMSG(x)
#else
#define DMSG_ETRACER(x...) do { } while (0)
#endif

#if defined(DEBUG)
#define DEBUG_MAX_VALUE_REACHED(val,max)                      \
do {                                                          \
  if (val >= max)                                             \
    {                                                         \
       ERROR("etracer: =================================\n"); \
       ERROR("etracer: max value reached for energy slot\n"); \
       ERROR("etracer: =================================\n"); \
    }                                                         \
} while (0)
#else
#define DEBUG_MAX_VALUE_REACHED(val,max) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ETRACER_MODE_IDLE   0
#define ETRACER_MODE_ACTIVE 1

struct libetracer_data_t {
 int                  mode;   /* used for idle time compression */
 int                  stopped;
 int                  next_must_be_NS;
};

static int                         libetracer_init = 0;

static trace_config_t              libetracer_conf;
static etrace_t                    libetracer_handler;
static trace_slot_t               *libetracer_slot;
static trace_slot_t               *libetracer_slot_bkp;
static struct libetracer_data_t    libetracer_current;
static struct libetracer_data_t    libetracer_backup;

static enum wsens_mode_t           libtracer_ws_mode;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

       void (*etracer_slot_insn_ptr)    (uint32_t pc, uint8_t class, uint16_t cycles, uint8_t non_seq, uint16_t ret_addr, uint8_t b_type, uint8_t except, uint8_t reti);
static void etracer_slot_insn_internal  (uint32_t pc, uint8_t class, uint16_t cycles, uint8_t non_seq, uint16_t ret_addr, uint8_t b_type, uint8_t except, uint8_t reti);

       void (*etracer_slot_access_ptr)  (uint32_t addr, uint8_t burst_size, uint8_t type, uint8_t width, uint8_t level, uint16_t timing);
static void etracer_slot_access_internal(uint32_t addr, uint8_t burst_size, uint8_t type, uint8_t width, uint8_t level, uint16_t timing);

       void (*etracer_slot_event_ptr)   (uint8_t periph_id, uint8_t event_id, uint8_t arg, uint8_t skew);
static void etracer_slot_event_internal (uint8_t periph_id, uint8_t event_id, uint8_t arg, uint8_t skew);

       void (*etracer_slot_set_pc_ptr)  (uint32_t pc);
static void etracer_slot_set_pc_internal(uint32_t pc);

       void (*etracer_slot_set_ns_ptr)  ();
static void etracer_slot_set_ns_internal();

       void (*etracer_slot_end_ptr)     (int timing);
static void etracer_slot_end_internal   (int timing);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void etracer_init(char *filename, int ws_mode)
{
  libetracer_conf.mode                = ETRACER_FILE_MODE;
  libetracer_conf.word_size           = ETRACER_WORD_SIZE;
  libetracer_conf.max_misses          = ETRACER_MAX_MISS;
  libetracer_conf.max_accesses        = ETRACER_MAX_ACCESS;
  libetracer_conf.max_per_evts        = ETRACER_MAX_PER_EVTS; 
  libetracer_conf.buffer_sz           = ETRACER_WRITE_BUFFER;    
  libetracer_handler                  = etrace_init(&libetracer_conf);
  
  etrace_open(libetracer_handler,filename);
  libetracer_slot                     = etrace_slot_new(libetracer_handler);
  libetracer_slot_bkp                 = etrace_slot_new(libetracer_handler);

  libetracer_current.mode             = ETRACER_MODE_ACTIVE;
  libetracer_current.stopped          = 1;
  libetracer_current.next_must_be_NS  = 0;
  libetracer_init                     = 1;

  libtracer_ws_mode                   = ws_mode;

  DMSG_ETRACER("etracer: init ok\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static uint64_t cumul = 0;
static uint64_t slots = 0;

void etracer_close(void)
{
  fprintf(stderr,"ETRACER timing %" PRId64 "\n",cumul);
  fprintf(stderr,"ETRACER slots  %" PRId64 "\n",slots);

  etrace_commit(libetracer_handler);
  etrace_close (libetracer_handler);
  DMSG_ETRACER ("etracer: close ok\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void etracer_start(void)
{
  if (libetracer_current.stopped == 1)
    {
      etracer_slot_insn_ptr   = etracer_slot_insn_internal;
      etracer_slot_access_ptr = etracer_slot_access_internal;
      etracer_slot_event_ptr  = etracer_slot_event_internal;
      etracer_slot_set_pc_ptr = etracer_slot_set_pc_internal;
      etracer_slot_set_ns_ptr = etracer_slot_set_ns_internal;
      etracer_slot_end_ptr    = etracer_slot_end_internal;

      libetracer_current.stopped         = 0;
      libetracer_current.next_must_be_NS = 1;
      DMSG_ETRACER ("etracer: start ok\n");
    }
  else
    {
      WARNING("etracer: etracer_start() called while already running\n");
      DMSG_ETRACER ("etracer: already started\n");
      libetracer_current.stopped         = 0;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void etracer_stop(void)
{
  if (libetracer_current.stopped == 0)
    {
      etracer_slot_insn_ptr   = NULL;
      etracer_slot_access_ptr = NULL;
      etracer_slot_event_ptr  = NULL;
      etracer_slot_set_pc_ptr = NULL;
      etracer_slot_set_ns_ptr = NULL;
      etracer_slot_end_ptr    = NULL;

      libetracer_current.stopped         = 1;
      libetracer_current.next_must_be_NS = 1;
      DMSG_ETRACER ("etracer: stopped ok\n");
      WARNING("etracer: etracer_stop()\n");
    }
  else
    {
      WARNING("etracer: etracer_stop() called while already stopped\n");
      DMSG_ETRACER ("etracer: already stopped\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ADDRESS_MASK 0x0000ffffu

static void etracer_slot_insn_internal(uint32_t pc, uint8_t class, uint16_t cycles, uint8_t non_seq, uint16_t ret_addr, uint8_t b_type, uint8_t except, uint8_t reti)
{
  libetracer_slot->insn.pc        = pc & ADDRESS_MASK;
  libetracer_slot->insn.ret_addr  = ret_addr;
  libetracer_slot->insn.class_id  = class;
  libetracer_slot->insn.cycles    = cycles;
  libetracer_slot->insn.non_seq   = non_seq;
  libetracer_slot->insn.except    = except;
  libetracer_slot->insn.reti      = reti;
  libetracer_slot->insn.b_type    = b_type;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
 * base_addr  : addr
 * burst_size : word size burst (1)
 * type       : RW_ACCESS | ACCESS_WIDTH
 * level      : 1
 * timing     : 0 for Flash & RAM
 */

static void etracer_slot_access_internal(uint32_t addr, uint8_t burst_size, uint8_t type, uint8_t width, uint8_t level, uint16_t timing)
{
  int n = libetracer_slot->hdr.num_accesses;

  libetracer_slot->accesses[n].base_addr  = addr;
  libetracer_slot->accesses[n].burst_size = burst_size;
  libetracer_slot->accesses[n].type       = type;
  libetracer_slot->accesses[n].width      = width;
  libetracer_slot->accesses[n].level      = level;
  libetracer_slot->accesses[n].timing     = timing;

  libetracer_slot->hdr.num_accesses ++;
  DEBUG_MAX_VALUE_REACHED(libetracer_slot->hdr.num_accesses,libetracer_conf.max_accesses);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* 
 * periph_id  : 
 * event_id   : 
 * skew       : time shift from beginning of slot
 */

static void etracer_slot_event_internal(uint8_t periph_id, uint8_t event_id, uint8_t arg, uint8_t skew)
{
  int n = libetracer_slot->hdr.num_per_evts;

  libetracer_slot->per_evts[n].periph_id = periph_id;
  libetracer_slot->per_evts[n].event_id  = event_id;
  libetracer_slot->per_evts[n].skew      = skew;
  libetracer_slot->per_evts[n].arg       = arg;

  libetracer_slot->hdr.num_per_evts ++;
  DEBUG_MAX_VALUE_REACHED(libetracer_slot->hdr.num_per_evts,libetracer_conf.max_per_evts);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void etracer_slot_set_pc_internal(uint32_t pc)
{
  libetracer_slot->insn.pc        = pc & ADDRESS_MASK;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void etracer_slot_set_ns_internal()
{
  libetracer_slot->insn.non_seq  = 1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ETRACER_MAX_IDLE 65530

static void etracer_slot_end_internal(int timing)
{
  libetracer_slot->hdr.timing += timing;
  if (libetracer_current.next_must_be_NS == 1)
    {
      etracer_slot_set_ns_internal();
      libetracer_current.next_must_be_NS = 0;
    }

  if (libetracer_slot->insn.class_id != ETRACER_INSN_CLASS_IDLE)
    {
      libetracer_current.mode = ETRACER_MODE_ACTIVE;
    }
  else
    {
      libetracer_current.mode = ETRACER_MODE_IDLE;
    }


  if (libetracer_current.mode == ETRACER_MODE_ACTIVE       ||
      libetracer_slot->hdr.num_per_evts              > 0   || 
      libetracer_slot->hdr.num_accesses              > 0   ||
      ETRACER_MAX_IDLE - libetracer_slot->hdr.timing < 1000)
    {
      cumul += libetracer_slot->hdr.timing;
      slots += 1;
      etrace_push(libetracer_handler,libetracer_slot);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void etracer_state_save(void)
{
  etrace_commit(libetracer_handler);
  etrace_slot_copy(libetracer_slot_bkp,libetracer_slot);
  memcpy(&libetracer_backup, &libetracer_current, sizeof(struct libetracer_data_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void etracer_state_restore(void)
{
  etrace_cancel(libetracer_handler);
  etrace_slot_copy(libetracer_slot,libetracer_slot_bkp);
  memcpy(&libetracer_current, &libetracer_backup, sizeof(struct libetracer_data_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#else /* defined(ETRACE) */

/* this function is left so that libetrace.a is not empty */
int libetrace_empty_stub(int a)
{
  return a*a+1;
}

#endif
