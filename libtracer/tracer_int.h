
/**
 *  \file   tracer_int.h
 *  \brief  Simulator activity tracer, internals
 *  \author Antoine Fraboulet
 *  \date   2010
 **/


#ifndef WSIM_TRACER_INT_H
#define WSIM_TRACER_INT_H


/****************************************
 * DEBUG
 * 
 * DMSG_TRACER is used for general tracer messages
 * DMSG_EVENT  is used for event recording messages
 * ERROR is used for ... errors
 ****************************************/

#if defined(DEBUG)
#define DEBUG_TRACER
// #define DEBUG_EVENT
#endif

#if defined(DEBUG_TRACER)
#define DMSG_TRACER(x...) HW_DMSG(x)
#else
#define DMSG_TRACER(x...) do { } while (0)
#endif

#if defined(DEBUG_EVENT)
#define DMSG_EVENT(x...) HW_DMSG(x)
#else
#define DMSG_EVENT(x...) do { } while (0)
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/****************************************
 * WSNET3 WORKAROUNDS
 ****************************************/ 
#ifdef WSNET3

#define _TRACER_DBG_MSG      /* tracer debug messages       */
#define TRACER_ERROR_MSG    /* tracer error messages       */

#ifdef TRACER_DBG_MSG
#    define TRACER_DBG(x...) fprintf(stderr, x)
#else
#    define TRACER_DBG(x...) do { } while (0)
#endif

#ifdef TRACER_ERROR_MSG
#    define TRACER_ERROR(x...) fprintf(stderr, x)
#else
#    define TRACER_ERROR(x...) do { } while (0)
#endif

#define MAX_FILENAME 256
#define OUTPUT(x...) TRACER_DBG(x)
#define ERROR(x...)  TRACER_ERROR(x)
#undef DMSG_TRACER
#define DMSG_TRACER(x...) TRACER_DBG(x)
#undef DMSG_EVENT
#define DMSG_EVENT(x...) TRACER_DBG(x)

#if !defined(strncpyz)
#define strncpyz(dst,src,size)			\
  do {						\
    strncpy(dst,src,size);			\
    dst[size - 1] = '\0';			\
  } while (0)
#endif

enum wsens_mode_t {
  WS_MODE_WSNET0 = 0
};
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/****************************************
 * For performance purpose and because this
 * is an early version most of the tracer
 * dimensions are fixed
 ****************************************/

/*
 *  64ksamples = 750kB
 * 128ksamples = 1.5MB
 * 256ksamples = 3MB
 *
 * recording blocks are   256ksamples * 12bytes = 3MB
 * recording threshold is 128ksamples * 12bytes = 1.5MB
 */

#define TRACER_BLOCK_EV              (256*1024)
#define TRACER_BLOCK_THRESHOLD_INIT  (128*1024)
extern unsigned int TRACER_BLOCK_THRESHOLD;

/****************************************
 * struct _sample_t is the sample type
 * that is recorded. Its size should be 
 * 12 bytes  (96 bits)
 ****************************************/

#define PACKED __attribute__((packed))

struct PACKED _tracer_sample_t {
  tracer_id_t    id;   /* 32 Bits */
  tracer_time_t  time; /* 64 bits */
  tracer_val_t   val;  /* 64 bits */
};
typedef struct _tracer_sample_t tracer_sample_t;

struct _tracer_state_t {
  /* events : these values are updated during record                     */
  /* these values are for the current block                              */
  tracer_ev_t      ev_count;                   /* event counter (index)  */
  tracer_val_t     id_val    [TRACER_MAX_ID];  /* current id value       */
  /* these value are for the complete trace                              */
  tracer_ev_t      ev_count_total;             /* global counter         */
  tracer_ev_t      id_count  [TRACER_MAX_ID];  /* count for each id      */
  tracer_val_t     id_val_min[TRACER_MAX_ID];  /* id_min                 */
  tracer_val_t     id_val_max[TRACER_MAX_ID];  /* id max                 */
};
typedef struct _tracer_state_t tracer_state_t;

extern tracer_state_t tracer_current;
#define EVENT_TRACER tracer_current

extern get_nanotime_function_t tracer_get_nanotime;
extern int32_t                 tracer_node_id;
extern tracer_time_t           tracer_initial_time;
extern char                    tracer_id_name   [TRACER_MAX_ID][TRACER_MAX_NAME_LENGTH];
extern char                    tracer_id_module [TRACER_MAX_ID][TRACER_MAX_NAME_LENGTH];
extern uint8_t                 tracer_width     [TRACER_MAX_ID];
extern tracer_sample_t         tracer_buffer    [TRACER_BLOCK_EV];

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline tracer_time_t TRACER_GET_NANOTIME()
{
  if (tracer_get_nanotime != NULL)
    {
      return tracer_get_nanotime();
    }
  else
    {
      return 0;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
