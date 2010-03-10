
/**
 *  \file   tracer_bin.c
 *  \brief  Simulator activity tracer
 *  \author Antoine Fraboulet
 *  \date   2010
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>   /* basename */
#include <inttypes.h>
#include <errno.h>
#include <inttypes.h>


#include "arch/common/hardware.h"
#include "liblogger/logger.h"
#include "src/options.h"
#include "tracer.h"
#include "tracer_int.h"
#include "tracer_bin.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


/****************************************
 * tracer datafile id and version
 * information
 ****************************************/

#define TRACER_MAGIC        "Worldsens tracer datafile"
#define TRACER_MAGIC_SIZE   sizeof(TRACER_MAGIC)
/* version 0: 
      use unpacked struct
      magic_size == 27
   version 1:
      packed struct
      magic_size == 26
   version 2:
      initial time
*/
#define TRACER_VERSION      4

static unsigned char           tracer_max_id        = TRACER_MAX_ID;
static FILE*                   tracer_datafile      = NULL;


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_binary_open(char *filename)
{
  if ((tracer_datafile = fopen(filename,"wb")) == NULL)
    {
      ERROR("tracer: ***********************************\n");
      ERROR("tracer: %s\n",strerror(errno));
      ERROR("tracer: ***********************************\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_binary_close()
{
  if (tracer_datafile)
    {
      fclose(tracer_datafile);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void
tracer_binary_start()
{
  char          v,e;
  uint32_t      i = 0;
  tracer_time_t t = 0;
  uint64_t      cycles;
  uint64_t      time;
  uint64_t      insn;
  uint32_t      size;
  uint32_t      backtracks;

  if (tracer_datafile == NULL)
    return;

  rewind(tracer_datafile);
  
  v = TRACER_VERSION;
#if defined(WORDS_BIGENDIAN)
  e = 0;
#else
  e = 1;
#endif

#if defined(WSNET1)
      cycles     = 0;
      insn       = 0;
      time       = TRACER_GET_NANOTIME();
      backtracks = 0;
#else
      cycles     = mcu_get_cycles();
      insn       = mcu_get_insn();
      time       = TRACER_GET_NANOTIME();
      backtracks = machine.backtrack;
#endif


  /* magic */
  size = sizeof(TRACER_MAGIC);
  i   += fwrite(TRACER_MAGIC,1,size,tracer_datafile);
  DMSG_TRACER("tracer:hdr:magic    : %s (%d)\n",TRACER_MAGIC,size);

  /* version */
  i += fwrite(&v,1,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:version  : %d\n",v);

  /* endianess */
  i += fwrite(&e,1,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:endian   : %d\n",e);

  /* backtracks uint32_t */
  size = sizeof(backtracks);
  i   += fwrite(&backtracks,size,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:backtrack: %d\n",backtracks);

  /* simulated cycles */
  size = sizeof(cycles);
  i   += fwrite(&cycles,size,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:cycles   : %"PRId64"\n",cycles);

  /* simulated nanoseconds */
  size = sizeof(time);
  i   += fwrite(&time,size,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:nano     : %"PRId64"\n",time);
  
  /* simulated instructions */
  size = sizeof(insn);
  i   += fwrite(&insn,size,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:insn     : %"PRId64"\n",insn);
  
  /* tracer node id   */
  size = sizeof(tracer_node_id);
  i   += fwrite(&tracer_node_id,size,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:node id  : %d\n",tracer_node_id);

  /* tracer initial time  */
  size = sizeof(tracer_initial_time);
  i   += fwrite(&tracer_initial_time,size,1,tracer_datafile);
  DMSG_TRACER("tracer:hdr:time     : %"PRIu64"\n",tracer_initial_time);

  /* max number of id */
  size = sizeof(tracer_max_id);
  i   += fwrite(&tracer_max_id,1,size,tracer_datafile);
  DMSG_TRACER("tracer:hdr:max id   : %d (%d)\n",tracer_max_id,size);

  /* number of events */
  size = sizeof(EVENT_TRACER.ev_count_total);
  i   += fwrite(&EVENT_TRACER.ev_count_total,1,size,tracer_datafile);
  DMSG_TRACER("tracer:hdr:number   : %d (%d)\n",EVENT_TRACER.ev_count_total,size);

  /* end simulation time */
  t    = TRACER_GET_NANOTIME();
  size = sizeof(tracer_time_t);
  i   += fwrite(&t,1,size,tracer_datafile);
  DMSG_TRACER("tracer:hdr:nano     : %" PRId64 " (%d)\n",t,size);

  /* events name */
  size = sizeof(tracer_id_name);
  i   += fwrite(tracer_id_name,1,size,tracer_datafile);
  DMSG_TRACER("tracer:hdr:events   : name %d bytes\n",size);

  /* events modules */
  size = sizeof(tracer_id_module);
  i   += fwrite(tracer_id_module,1,size,tracer_datafile);
  DMSG_TRACER("tracer:hdr:modules  : name %d bytes\n",size);

  /* events width */
  size = sizeof(tracer_width);
  i   += fwrite(tracer_width, 1, size, tracer_datafile);
  DMSG_TRACER("tracer:hdr:events   : width %d bytes\n",size);

  /* id_count, id_min, id_max */
  size = sizeof(tracer_ev_t)  * TRACER_MAX_ID;
  i   += fwrite(EVENT_TRACER.id_count  , 1, size, tracer_datafile);
  size = sizeof(tracer_val_t) * TRACER_MAX_ID;
  i   += fwrite(EVENT_TRACER.id_val_min, 1, size, tracer_datafile);
  size = sizeof(tracer_val_t) * TRACER_MAX_ID;
  i   += fwrite(EVENT_TRACER.id_val_max, 1, size, tracer_datafile);

  DMSG_TRACER("tracer:header: total %d bytes\n",i);
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_binary_finish()
{
  tracer_binary_start();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_binary_dump_data()
{
  int written;
  int32_t size;

  size    = sizeof(tracer_sample_t)*EVENT_TRACER.ev_count;
  written = fwrite(tracer_buffer, 1, size, tracer_datafile);
  assert(written == size);
  DMSG_TRACER("tracer:data:dump: write %d ev = %d bytes (%d requested)\n",EVENT_TRACER.ev_count,written,size);
  EVENT_TRACER.ev_count = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
