/*
 *  tracer.c
 *  
 *
 *  Created by Antoine Fraboulet on 01/05/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include <private/tracer_private.h>

#include <public/simulation.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define EVENT_TRACER core_tracer

#define UNUSED __attribute__((unused))  

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
/* recording blocks are 1M samples */
#define TRACER_BLOCK_EV              0xfffffu 
static struct _sample_t ev_data_buff[TRACER_BLOCK_EV];

/* blobk access macro */
#define tracer_end_of_block(e) ((e & TRACER_BLOCK_EV) == TRACER_BLOCK_EV)

#define tracer_get_event_id(e)   EVENT_TRACER.ev_data[e].id
#define tracer_get_event_time(e) EVENT_TRACER.ev_data[e].time
#define tracer_get_event_val(e)  EVENT_TRACER.ev_data[e].val

#define tracer_set_event(e,id,time,val)					\
  do {									\
    EVENT_TRACER.ev_data[e].id   = id;					\
    EVENT_TRACER.ev_data[e].time = time;				\
    EVENT_TRACER.ev_data[e].val  = val;					\
  } while(0)

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
tracer_t core_tracer;
char tracer_file[256];


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
/* start/stop recording event */
void  (*tracer_event_record) (tracer_id_t id, tracer_val_t val);

static void tracer_event_record_active(tracer_id_t id, tracer_val_t val);
static void tracer_event_record_stub  (tracer_id_t id, tracer_val_t val);
static void tracer_event_record_time_nocheck(tracer_id_t id, tracer_val_t val, tracer_time_t time);


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int
tracer_instantiate(char *key, FILE *config_fd)
{
		
	memset(&EVENT_TRACER, 0, sizeof(EVENT_TRACER));
	EVENT_TRACER.ev_data = ev_data_buff;
	if (strcmp(key, "none") == 0) {
		tracer_stop();
		return 0;
	} else {
		tracer_start();
	}
	
	if (fscanf(config_fd, "%s", tracer_file) != 1) {
		fprintf(stderr, "tracer_instantiate: failed to read \"tracer.inst\"\n");
		return -1;
	}
	
	//tracer_dump_set_format(key);	
	return 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void
tracer_complete(void) {
  // tracer_dump_file(tracer_file);
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
void
tracer_start()
{
	tracer_event_record = tracer_event_record_active;
}

void tracer_stop()
{
	tracer_event_record = tracer_event_record_stub;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void
tracer_event_add_id(tracer_id_t id, char* label)
{
	if (id < (TRACER_MAX_ID - 1))
    {
		strncpy(EVENT_TRACER.id_name[id],label,TRACER_MAX_NAME_LENGTH);
		tracer_event_record_time_nocheck(id,0,0);
    }
	else
    {
    }
}

static void
tracer_event_record_time_nocheck(tracer_id_t id, tracer_val_t val, tracer_time_t time)
{
	if (! tracer_end_of_block(EVENT_TRACER.ev_count))
    {
		tracer_set_event(EVENT_TRACER.ev_count,id,time,val);
		EVENT_TRACER.id_last[id] = EVENT_TRACER.ev_count ++; 
		if (tracer_end_of_block(EVENT_TRACER.ev_count))
		{
			 fprintf(stderr, "Tracer: max event reached\n");
		}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
tracer_event_record_time(tracer_id_t id, tracer_val_t val, tracer_time_t time)
{
	if (val != tracer_get_event_val(EVENT_TRACER.id_last[id]))
    {
		tracer_event_record_time_nocheck(id,val,time);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void 
tracer_event_record_active(tracer_id_t id, tracer_val_t val)
{
	tracer_event_record_time(id,val,g_time);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
tracer_event_record_stub(tracer_id_t UNUSED id, tracer_val_t UNUSED val)
{
	/* do nothing */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
