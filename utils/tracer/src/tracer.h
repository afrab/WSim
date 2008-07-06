
/**
 *  \file   tracer.h
 *  \brief  Simulator activity tracer
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef WORLDSENS_TRACER_H
#define WORLDSENS_TRACER_H

#include <sys/types.h>
#include <dirent.h>

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

#define DEFAULT_STOP_TIME 0xffffffffffffffffull /* 64 */

typedef uint32_t              tracer_ev_t;     /* event number */
typedef uint32_t              tracer_id_t;     /* signal id    */
typedef uint64_t              tracer_time_t;   /* time         */
typedef uint64_t              tracer_val_t;    /* value        */
typedef uint8_t               tracer_width_t;  /* width        */

/****************************************
 * struct _sample_t is the sample type
 * that is recorded. Its size should be 
 * 12 bytes  (96 bits)
 ****************************************/

#define PACKED __attribute__((packed))

struct PACKED tracer_sample_struct_t {
  tracer_id_t    id;   /* 32 Bits */
  tracer_time_t  time; /* 64 bits */
  tracer_val_t   val;  /* 64 bits */
};
typedef struct tracer_sample_struct_t tracer_sample_t;

/****************************************
 * tracer datafile id and version
 * information
 ****************************************/

#define TRACER_MAGIC             "Worldsens tracer datafile"
#define TRACER_MAGIC_SIZE        26

/* version 0: 
      use unpacked struct
      magic_size == 27
   version 1:
      packed struct
      magic_size == 26
*/
#define TRACER_VERSION           2

#define TRACER_MAX_ID            255
#define TRACER_MAX_NAME_LENGTH   30


struct _tracer_header_t {
  int32_t          need_endian_swap;
  int32_t          header_length;
  int32_t          node_id;

  /* simulation information for WSim traces */
  uint32_t         backtracks;
  uint64_t         simul_nano;
  uint64_t         simul_insn;
  uint64_t         simul_cycles;

  /* these value are for the complete trace                              */
  uint8_t          tracer_max_id;              /*                        */
  tracer_ev_t      ev_count_total;             /* global counter         */
  tracer_time_t    sim_time_total;

  char             id_name   [TRACER_MAX_ID][TRACER_MAX_NAME_LENGTH];
  tracer_width_t   id_width  [TRACER_MAX_ID];  /*                        */
  tracer_ev_t      id_count  [TRACER_MAX_ID];  /* count for each id      */
  tracer_val_t     id_val_min[TRACER_MAX_ID];  /* id_min                 */
  tracer_val_t     id_val_max[TRACER_MAX_ID];  /* id max                 */
};
typedef struct _tracer_header_t tracer_header_t;

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

struct tracer_struct_t {
  tracer_header_t  hdr;
  int              debug;
  int              merge;

  char             in_filename[FILENAME_MAX];
  char             out_filename[FILENAME_MAX];
  
  FILE            *in_fd;
  FILE            *out_fd;
  int              file_mode;

  char             in_Dir[FILENAME_MAX];
  DIR             *dir;
  char             dir_pattern[FILENAME_MAX];
  int              dir_mode;

  char            *out_format_name;
  char            *out_signal_name;

  tracer_time_t    start_time;
  tracer_time_t    stop_time;
};
typedef struct tracer_struct_t tracer_t;


tracer_t*       tracer_create          ();
void            tracer_delete          (tracer_t *t);

int             tracer_file_open       (tracer_t *t);
int             tracer_file_close      (tracer_t *t);
int             tracer_file_rewind     (tracer_t *t);
void            tracer_file_dump_header(tracer_t *t);

int             tracer_file_out_open   (tracer_t *t, const char* suffix);
int             tracer_file_out_close  (tracer_t *t);

int             tracer_dirmode_init    (tracer_t *t, const char* patterm);
int             tracer_dirmode_next    (tracer_t *t);
int             tracer_dirmode_close   (tracer_t *t);

tracer_id_t     tracer_find_id_by_name (tracer_t *t, char* name);
int             tracer_read_sample     (tracer_t *t, tracer_sample_t *s);

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

/* tracer output driver */

typedef int (*tracer_drv_function_t)(tracer_t*);

struct tracer_driver_struct_t {
  char                  *name; 
  tracer_drv_function_t  init;     /* init process, called once     */
  tracer_drv_function_t  process;  /* process, called for each file */
  tracer_drv_function_t  finalize; /* finalize process, called once */
};

typedef struct tracer_driver_struct_t tracer_driver_t;

void             tracer_driver_register    (tracer_driver_t *drv);
tracer_driver_t *tracer_driver_get_by_name (char* name);

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

#endif /* WORLDSENS_TRACER_H */
