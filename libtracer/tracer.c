
/**
 *  \file   tracer.c
 *  \brief  Simulator activity tracer
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>   /* basename */
#include <inttypes.h>
#include <errno.h>
#include <inttypes.h>

#ifndef WSNET3
#include "arch/common/hardware.h"
#include "liblogger/logger.h"
#include "src/options.h"
#endif

#include "tracer.h"
#include "tracer_int.h"
#include "tracer_bin.h"
#include "tracer_vcd.h"

void app_exit_error();

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* values that are not saved during a state_save */

tracer_state_t tracer_current;

int32_t                    tracer_node_id       = 0xFFFF;
tracer_time_t              tracer_initial_time  = 0;
char                       tracer_id_name        [TRACER_MAX_ID][TRACER_MAX_NAME_LENGTH];
char                       tracer_id_module      [TRACER_MAX_ID][TRACER_MAX_NAME_LENGTH];
uint8_t                    tracer_width          [TRACER_MAX_ID];
tracer_sample_t            tracer_buffer         [TRACER_BLOCK_EV];
unsigned int               TRACER_BLOCK_THRESHOLD;

get_nanotime_function_t    tracer_get_nanotime  = NULL;

static tracer_state_t      tracer_saved;
static tracer_id_t         tracer_registered_id = 0; /* registered id from simulation */
static char               *tracer_filename      = NULL;
static int                 tracer_init_done     = 0;
static enum wsens_mode_t   tracer_ws_mode;

/* block access macro */
#define tracer_end_of_block(e)   ((e & TRACER_BLOCK_EV) == TRACER_BLOCK_EV)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* start/stop recording event */
void  (*tracer_event_record_ptr)               (tracer_id_t id, tracer_val_t val);
void  (*tracer_event_record_force_ptr)         (tracer_id_t id, tracer_val_t val);
void  (*tracer_event_record_time_ptr)          (tracer_id_t id, tracer_val_t val, tracer_time_t time);

static void tracer_event_record_active         (tracer_id_t id, tracer_val_t val);
static void tracer_event_record_active_ws      (tracer_id_t id, tracer_val_t val);

static void tracer_event_record_active_force   (tracer_id_t id, tracer_val_t val);
static void tracer_event_record_active_force_ws(tracer_id_t id, tracer_val_t val);

static void tracer_event_record_active_time    (tracer_id_t id, tracer_val_t val, tracer_time_t time);
static void tracer_event_record_active_time_ws (tracer_id_t id, tracer_val_t val, tracer_time_t time);

static void tracer_event_record_time_nocheck   (tracer_id_t id, tracer_val_t val, tracer_time_t time);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void (*tracer_dump_data)      (void);
void (*tracer_output_close)   (void);
void (*tracer_start_internal) (void);
void (*tracer_stop_internal)  (void);

void
tracer_output_open(char *filename)
{
  char file[MAX_FILENAME];
  TRACER_BLOCK_THRESHOLD = TRACER_BLOCK_THRESHOLD_INIT;

  strncpyz(file,filename,MAX_FILENAME);
  if (strstr(file,"vcd:") == file)
    { 
      char* fptr = file + 4;
      char* tptr = strchr(fptr,':');
      if (tptr != NULL)
	{
	  TRACER_BLOCK_THRESHOLD = atoi(tptr + 1);
	  if (TRACER_BLOCK_THRESHOLD > TRACER_BLOCK_THRESHOLD_INIT)
	    {
	      TRACER_BLOCK_THRESHOLD = TRACER_BLOCK_THRESHOLD_INIT;
	    }
	  OUTPUT("wsim:tracer: write buffer threshold set to %d\n",TRACER_BLOCK_THRESHOLD);
	  tptr[0] = 0;
	}
      tracer_vcd_open(fptr);
      tracer_dump_data       = tracer_vcd_dump_data;
      tracer_output_close    = tracer_vcd_close;
      tracer_start_internal  = tracer_vcd_start;
      tracer_stop_internal   = tracer_vcd_finish;
    }
  // else if (format)
  //  {
  //  }
  else /* default */ 
    { 
      if (strstr(filename,"bin:") == filename)
	tracer_binary_open(filename + 4);
      else
	tracer_binary_open(filename);

      tracer_dump_data       = tracer_binary_dump_data;
      tracer_output_close    = tracer_binary_close;
      tracer_start_internal  = tracer_binary_start;
      tracer_stop_internal   = tracer_binary_finish;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void
tracer_init(char *filename, int ws_mode)
{
  int id;

  tracer_ws_mode = ws_mode;

  if (filename == NULL)
    return ;

  tracer_get_nanotime = NULL;
  tracer_filename     = strdup(filename);
  for(id=0; id < TRACER_MAX_ID; id++)
    {
      tracer_id_name   [id][0]    = '\0';
      tracer_id_module [id][0]    = '\0';
      EVENT_TRACER.id_count[id]   = 0;
      EVENT_TRACER.id_val_min[id] = 0;
      EVENT_TRACER.id_val_max[id] = 0;
    }
  EVENT_TRACER.ev_count       = 0;
  EVENT_TRACER.ev_count_total = 0;;
  tracer_stop();
  tracer_output_open(filename);
  tracer_init_done = 1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_set_timeref(get_nanotime_function_t fun)
{
  tracer_get_nanotime = fun;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void
tracer_close(void)
{
  tracer_dump_data();
  tracer_stop_internal();
  tracer_output_close();
  DMSG_TRACER("tracer: close ok\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void
tracer_start(void)
{
  static char first_start = 1;
  if (tracer_init_done == 1)
    { 
      if (first_start == 1)
	{
	  tracer_start_internal();
	  first_start = 0;
	}

      if (tracer_ws_mode == WS_MODE_WSNET0)
	{
	  tracer_event_record_ptr       = tracer_event_record_active;
	  tracer_event_record_force_ptr = tracer_event_record_active_force;
	  tracer_event_record_time_ptr  = tracer_event_record_active_time;
	}
      else
	{
	  tracer_event_record_ptr       = tracer_event_record_active_ws;
	  tracer_event_record_force_ptr = tracer_event_record_active_force_ws;
	  tracer_event_record_time_ptr  = tracer_event_record_active_time_ws;
	}
    }
  else
    {
      ERROR("tracer: attempt to start the tracer before init\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_stop(void)
{
  tracer_event_record_ptr = NULL;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

tracer_id_t
tracer_event_add_id(int width, const char* name, const char* module)
{
  tracer_id_t i;
  tracer_id_t id;
  
  /* check if mobule/label is already registered :: */
  for(i=0;  i < tracer_registered_id; i++)
    {
      if (strncmp(tracer_id_name   [i], name,   TRACER_MAX_NAME_LENGTH) == 0 &&
	  strncmp(tracer_id_module [i], module, TRACER_MAX_NAME_LENGTH) == 0 &&
	  tracer_width[i] == width)
	{
	  ERROR("tracer: event %s.%s is already registered\n",module,name);
	  app_exit_error();
	  return -1;
	}
    }

  /* get new id */
  id = tracer_registered_id++;

  if (id >= (TRACER_MAX_ID - 1))
    {
      ERROR("tracer: max event recording reached, could not register [%s] = %d\n",name,id);
      app_exit_error();
    }

  if ((name == NULL) || (strlen(name) == 0))
    {
      ERROR("tracer: event id %d must have a valid name (non null)\n",id);
      app_exit_error();
    }

  if (((width < 1) || (width > 64)) && (strcmp(name,"__WSIMLOGBUFFER") != 0))
    {
      ERROR("tracer: event id %d \"%s\" must have 0 < width < 65 bits\n",id,name);
    }
  
  strncpyz(tracer_id_name   [id], name,   TRACER_MAX_NAME_LENGTH - 1);
  strncpyz(tracer_id_module [id], module, TRACER_MAX_NAME_LENGTH - 1);
  tracer_width[id] = width;

  DMSG_TRACER("tracer:add:id: %02d=%-10s module=%-10s\n",id,name,module);
  tracer_event_record_time_nocheck(id,0,0);
  return id;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline void 
tracer_set_event(tracer_ev_t e, tracer_id_t id, tracer_time_t time, uint64_t val)
{
  tracer_buffer[e].id   = id;
  tracer_buffer[e].time = time;
  tracer_buffer[e].val  = val;
} 

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define min(a,b)  ((a < b) ? a : b)
#define max(a,b)  ((a < b) ? b : a)

static void
tracer_event_record_time_nocheck(tracer_id_t id, tracer_val_t val, tracer_time_t time)
{
  if (! tracer_end_of_block(EVENT_TRACER.ev_count))
    {
      tracer_set_event(EVENT_TRACER.ev_count,id,time,val);

      EVENT_TRACER.ev_count       += 1;
      EVENT_TRACER.id_val[id]      = val;

      EVENT_TRACER.ev_count_total += 1;
      EVENT_TRACER.id_count[id]   += 1;
      EVENT_TRACER.id_val_min[id]  = min(EVENT_TRACER.id_val_min[id],val);
      EVENT_TRACER.id_val_max[id]  = max(EVENT_TRACER.id_val_max[id],val);

      if (tracer_end_of_block(EVENT_TRACER.ev_count))
	{
	  ERROR("tracer:error: max event reached (id %d %s.%s)\n",id,tracer_id_module[id],tracer_id_name[id]);
	}
    }
  DMSG_EVENT("tracer:add:event: [%s] = (%" PRId64 ",%" PRId64 ")\n",tracer_id_name[id],time,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/****************************************
 * 2 functions to select data log file
 * dump behavior depending on worldsens mode
 *
 * if wsnet1 or wsnet2 (_ws function)
 *   we can save at each backtrack
 *   this is minimized by recording to memory 
 *   and save only when threshold is reached and
 *   a backtrack occurs
 * else
 *   we can save whenever the threshold is reached
 * 
 ****************************************/

static void 
tracer_event_record_active_time(tracer_id_t id, tracer_val_t val, tracer_time_t time)
{
  /* we record only value change to limit trace size */
  if (val != EVENT_TRACER.id_val[id])
    {
      tracer_event_record_time_nocheck(id,val,time);
    }
  if (EVENT_TRACER.ev_count > TRACER_BLOCK_THRESHOLD)
    {
      tracer_dump_data();
    }
}

static void 
tracer_event_record_active_time_ws(tracer_id_t id, tracer_val_t val, tracer_time_t time)
{
  /* we record only value change to limit trace size */
  if (val != EVENT_TRACER.id_val[id])
    {
      tracer_event_record_time_nocheck(id,val,time);
    }
}

static void 
tracer_event_record_active(tracer_id_t id, tracer_val_t val)
{
  /* we record only value change to limit trace size */
  if (val != EVENT_TRACER.id_val[id])
    {
      uint64_t time = TRACER_GET_NANOTIME();
      tracer_event_record_time_nocheck(id,val,time);
    }
  if (EVENT_TRACER.ev_count > TRACER_BLOCK_THRESHOLD)
    {
      tracer_dump_data();
    }
}

static void 
tracer_event_record_active_ws(tracer_id_t id, tracer_val_t val)
{
  /* we record only value change to limit trace size */
  if (val != EVENT_TRACER.id_val[id])
    {
      uint64_t time = TRACER_GET_NANOTIME();
      tracer_event_record_time_nocheck(id,val,time);
    }
}


/* force:: we write a zero to make sure VCD will notify */
static void 
tracer_event_record_active_force(tracer_id_t id, tracer_val_t val)
{
  if (val != EVENT_TRACER.id_val[id])
    {
      uint64_t time = TRACER_GET_NANOTIME();
      tracer_event_record_time_nocheck(id,0      , time-1);
      tracer_event_record_time_nocheck(id,val    , time);
      tracer_event_record_time_nocheck(id,0      , time+1);
    }
  if (EVENT_TRACER.ev_count > TRACER_BLOCK_THRESHOLD)
    {
      tracer_dump_data();
    }
}

static void 
tracer_event_record_active_force_ws(tracer_id_t id, tracer_val_t val)
{
  if (val != EVENT_TRACER.id_val[id])
    {
      uint64_t time = TRACER_GET_NANOTIME();
      tracer_event_record_time_nocheck(id,0      , time-1);
      tracer_event_record_time_nocheck(id,val    , time);
      tracer_event_record_time_nocheck(id,0      , time+1);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void
tracer_state_save(void)
{
  if (EVENT_TRACER.ev_count > TRACER_BLOCK_THRESHOLD)
    {
      tracer_dump_data();
    }
  memcpy(&tracer_saved, &tracer_current, sizeof(tracer_state_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void
tracer_state_restore(void)
{
  memcpy(&tracer_current, &tracer_saved, sizeof(tracer_state_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_set_node_id(int id)
{
  tracer_node_id = id;
  DMSG_TRACER("tracer:data: node_id to %d (0x%04x)\n",tracer_node_id,tracer_node_id);
}

void tracer_set_initial_time(tracer_time_t time)
{
  tracer_initial_time = time;
  DMSG_TRACER("=================================================\n");
  DMSG_TRACER("tracer:time: connexion time to WSNET : %"PRIu64" \n", time);
  DMSG_TRACER("=================================================\n");
} 

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
