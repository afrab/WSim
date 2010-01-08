
/**
 *  \file   tracer.h
 *  \brief  Simulator activity tracer
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef TRACER_H
#define TRACER_H

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

typedef uint32_t              tracer_ev_t;     /* event number */
typedef uint32_t              tracer_id_t;     /* signal id    */
typedef uint64_t              tracer_time_t;   /* time         */
typedef uint64_t              tracer_val_t;    /* value        */

typedef tracer_time_t (*get_nanotime_function_t)();

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* 
 * an EVENT_TRACER macro must be defined externally to point
 * to a static tracer_t variable
 */
void        tracer_init         (char *filename, int ws_mode);
void        tracer_set_timeref  (get_nanotime_function_t f);
tracer_id_t tracer_event_add_id (int width, char* label, char* module);
void        tracer_close        (void);

/*
 * start and stop can be called within the program execution time   
 * these functioins modify the tracer_event_record function pointer 
 */
void  tracer_start              (void);
void  tracer_stop               (void);

/*
 * header information 
 */
void  tracer_set_node_id        (int id);
void  tracer_set_initial_time   (tracer_time_t time);

/*
 * Record a value change for id
 */
extern void  (*tracer_event_record_ptr) (tracer_id_t id, tracer_val_t val);

#define tracer_event_record(id,val)          \
do {                                         \
  if (tracer_event_record_ptr != NULL)       \
    tracer_event_record_ptr(id,val);         \
} while(0)

void  tracer_state_save         (void);
void  tracer_state_restore      (void);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#if 0
#define NOOP  do { } while (0)
#define tracer_close()                NOOP
#define tracer_init(f1,f2)            NOOP
#define tracer_set_timeref(f1)        NOOP
#define tracer_start()                NOOP
#define tracer_stop()                 NOOP
#define tracer_event_add_id(j,k,l)    0
#define tracer_event_record(i,v)      NOOP
#define tracer_state_save()           NOOP
#define tracer_state_restore()        NOOP
#define tracer_dump_set_format(f)     1
#define tracer_dump_file(n)           NOOP
#define tracer_set_node_id(i)         NOOP
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif /* TRACER_H */
