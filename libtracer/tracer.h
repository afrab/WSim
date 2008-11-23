
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

#define __NO__TRACE_ME_HARDER


#define TRACER_MCU_RESET       0
#define TRACER_MCU_PC          1
#define TRACER_MCU_INTR        2
#define TRACER_MCU_GIE         3
#define TRACER_MCU_POWER       4
#define TRACER_MCU_PORT1       5
#define TRACER_MCU_PORT2       6
#define TRACER_MCU_PORT3       7
#define TRACER_MCU_PORT4       8
#define TRACER_MCU_PORT5       9
#define TRACER_MCU_PORT6      10
#define TRACER_USART0         11
#define TRACER_USART1         12

#define TRACER_MSP430_MCLK    13
#define TRACER_MSP430_SMCLK   14
#define TRACER_MSP430_ACLK    15

#define TRACER_LED1           16
#define TRACER_LED2           17
#define TRACER_LED3           18
#define TRACER_LED4           19

#define TRACER_DS2411         20 /* ds2411 */

/* radio */
#define TRACER_CC1100_STATE   21 /* cc1100 */
#define TRACER_CC1100_STROBE  22 /* cc1100 */
#define TRACER_CC1100_CS      23 /* cc1100 */

#define TRACER_CC2420_STATE   21 /* cc2420 */

/* flash memories */
#define TRACER_M25P10_STATE   26 /* flash m25p10 */
#define TRACER_M25P10_STROBE  27
#define TRACER_M25P80_STATE   26 /* flash m25p80 */
#define TRACER_M25P80_STROBE  27
#define TRACER_AT45DB_STATE   26 /* flash at45db */
#define TRACER_AT45DB_STROBE  27

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(TRACE_ME_HARDER)
#define TRACER_TRACE_PC(xpc)  tracer_event_record(TRACER_MCU_PC, xpc)
#define TRACER_TRACE_GIE(gie) tracer_event_record(TRACER_MCU_GIE,gie)
#else
#define TRACER_TRACE_PC(xpc)  do { } while (0)
#define TRACER_TRACE_GIE(gie) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

typedef uint32_t              tracer_ev_t;     /* event number */
typedef uint32_t              tracer_id_t;     /* signal id    */
typedef uint64_t              tracer_time_t;   /* time         */
typedef uint64_t              tracer_val_t;    /* value        */

typedef uint64_t (*get_nanotime_function_t)();

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* 
 * an EVENT_TRACER macro must be defined externally to point
 * to a static tracer_t variable
 */
void  tracer_init            (char *filename, get_nanotime_function_t f);
void  tracer_event_add_id    (tracer_id_t id, int width, char* label, char* module);
void  tracer_close           (void);

/*
 * start and stop can be called within the program execution time   
 * these functioins modify the tracer_event_record function pointer 
 */
void  tracer_start           (void);
void  tracer_stop            (void);

/*
 * header information 
 */
void  tracer_set_node_id     (int id);
void  tracer_set_initial_time(tracer_time_t time);

/*
 * Record a value change for id
 */
extern void  (*tracer_event_record_ptr) (tracer_id_t id, tracer_val_t val);

#define tracer_event_record(id,val)          \
do {                                         \
  if (tracer_event_record_ptr != NULL)       \
    tracer_event_record_ptr(id,val);         \
} while(0)

void  tracer_state_save      (void);
void  tracer_state_restore   (void);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#if 0
#define NOOP  do { } while (0)

#define tracer_close()                NOOP
#define tracer_init(f1,f2)            NOOP
#define tracer_start()                NOOP
#define tracer_stop()                 NOOP
#define tracer_event_add_id(i,j,k,l)  NOOP
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
