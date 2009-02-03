/*
 *  tracer_private.h
 *  
 *
 *  Created by Antoine Fraboulet on 01/05/06.
 *  Copyright 2006 __WorldSens__. All rights reserved.
 *
 */

#ifndef _TRACER_PRIVATE_H
#define _TRACER_PRIVATE_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/tracer.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
enum _tracer_format_t {
	tracer_unknown = -1,
	tracer_raw     =  0,
	tracer_all     =  1,
	tracer_gplot   =  2,
	tracer_vcd     =  3,
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define TRACER_FMT_RAW   "raw"
#define TRACER_FMT_ALL   "all"
#define TRACER_FMT_GPLOT "gplot"
#define TRACER_FMT_VCD   "vcd"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
typedef uint32_t              tracer_ev_t;     /* event number */
typedef uint64_t              tracer_time_t;   /* time         */
typedef enum _tracer_format_t tracer_format_t;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#define TRACER_MAX_ID                100
#define TRACER_MAX_NAME_LENGTH       100


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _sample_t {
	tracer_id_t    id;
	tracer_time_t  time;
	tracer_val_t   val;
};


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
struct _tracer_t {
	/* events : these values are updated during record */
	tracer_ev_t      ev_count;                   /* global event counter   */
	tracer_ev_t      id_last[TRACER_MAX_ID];     /* last event for each id */
	
	tracer_ev_t      ev_count_bkp;               /* backup                 */
	tracer_ev_t      id_last_bkp[TRACER_MAX_ID];
	
	struct _sample_t *ev_data;                   /* data [event]           */
	
	/* id */
	char             id_name[TRACER_MAX_ID][TRACER_MAX_NAME_LENGTH];
	
	/* id min/max event : used only for dump */
	tracer_ev_t      id_count_ev  [TRACER_MAX_ID]; /* count for each id */
	tracer_ev_t      id_min_val_ev[TRACER_MAX_ID]; /* id_min            */
	tracer_ev_t      id_max_val_ev[TRACER_MAX_ID]; /* id max            */
	
	/* output format */
	tracer_format_t  format;
};

typedef struct _tracer_t tracer_t;


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
void  tracer_start           ();
void  tracer_stop            ();
void  tracer_complete        ();
void  tracer_state_save      ();
void  tracer_state_restore   ();
int   tracer_dump_set_format (char *format);
void  tracer_dump_file       (char *filename);
int   tracer_instantiate     (char *key, FILE *config_fd);


#endif //_TRACER_PRIVATE_H
