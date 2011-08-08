#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "config.h"
#include "log.h"
#include "wsim_endian.h"

#include "tracer.h"

static int  tracer_load        (tracer_t *t);
static void tracer_swap_header (tracer_t *t);
static void tracer_swap_sample (tracer_sample_t *s);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

tracer_t* tracer_create()
{
  tracer_t *t;
  t = (tracer_t*)malloc(sizeof(tracer_t));
  memset(t,0,sizeof(tracer_t));
  return t;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if !defined(strncpyz)
#define strncpyz(dst,src,size)			\
  do {						\
    strncpy(dst,src,size);			\
    dst[size - 1] = '\0';			\
  } while (0)
#endif

#define TERROR(test,msg)                      \
  do {					      \
    if (test)				      \
      {					      \
	ERROR(msg);			      \
	return 1;			      \
      }					      \
  } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_dirmode_init(tracer_t *t, const char* pattern)
{
  TERROR(t->in_Dir == NULL, "tracer:dirmode: dir is null\n");
  DMSG(t,"tracer:dirmode: init %s\n",t->in_Dir);

  if ((t->dir=opendir(t->in_Dir))==NULL)
    {
      ERROR("Directory %s could not be open\n",t->in_Dir);
      return 1;
    }
      
  strncpyz(t->dir_pattern,pattern,FILENAME_MAX);
  t->dir_pattern[FILENAME_MAX - 1] = '\0';
  strncpyz(t->in_filename,"",FILENAME_MAX);
  t->in_filename[FILENAME_MAX - 1] = '\0';
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_dirmode_next(tracer_t *t)
{
  int            flen;
  int            slen;
  char          *ptr;
  struct dirent *dirent;
  
  TERROR(t->in_Dir == NULL,"tracer:dirmode:next error, dir is NULL\n");
  DMSG(t,"tracer:dirmode: next\n");

  slen = strlen(t->dir_pattern);
  while ((dirent = readdir(t->dir)) != NULL)
    {
      ptr = strstr(dirent->d_name,t->dir_pattern);
      flen = strlen(dirent->d_name);

      /* 
       * aaaa.trc   pattern == .trc 
       * flen = 8, slen = 4
       * check that the name end with the suffix
       */
      
      if ((ptr != NULL) && (flen - (ptr - dirent->d_name) == slen))
	{
	  
	  if (t->in_Dir[strlen(t->in_Dir) - 1] == '/')
	    {
	      snprintf(t->in_filename,  FILENAME_MAX, "%s%s",t->in_Dir, dirent->d_name);
	      snprintf(t->out_filename, FILENAME_MAX, "%s%s",t->in_Dir, dirent->d_name);
	    }
	  else
	    {
	      snprintf(t->in_filename,  FILENAME_MAX, "%s/%s", t->in_Dir, dirent->d_name);
	      snprintf(t->out_filename, FILENAME_MAX, "%s/%s", t->in_Dir, dirent->d_name);
	    }

	  DMSG(t,"tracer:dirmode: in filename %s\n",t->in_filename);
	  return 0;
	}
    }
  return 1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_dirmode_close(tracer_t *t)
{
  TERROR(t->dir == NULL,"tracer:dirmode:next error, dir is NULL\n");
  DMSG(t,"tracer:dirmode: close\n");
  closedir(t->dir);
  t->dir = NULL;
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_file_in_open(tracer_t *t)
{
  if ((t->in_fd = fopen(t->in_filename,"rb")) == NULL)
    {
      ERROR("tracer:file: open error on input %s\n",t->in_filename);
      return 1;
    }
  tracer_load(t);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_file_in_close(tracer_t *t)
{
  if (t)
    {
      if (t->in_fd)
	{
	  fclose(t->in_fd);
	  t->in_fd = NULL;
	}
    }
  else
    {
      ERROR("tracer:file: close error on input %s\n",t->in_filename);
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_file_in_rewind(tracer_t *t)
{
  TERROR(t->in_fd == NULL,"tracer:file: rewind error\n");
  fseek(t->in_fd,t->hdr.header_length,SEEK_SET);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int tracer_load(tracer_t *t)
{
  char version;
  char e,te;
  int  r = 0;
  char buff[1024];

  TERROR(t == NULL, "tracer:file:load tracer is NULL\n");
  TERROR(t->in_fd == NULL, "tracer:file:load file is NULL\n");

  r = fread(buff,1,TRACER_MAGIC_SIZE,t->in_fd);
  if ((r < TRACER_MAGIC_SIZE) || 
      (strncmp(buff,TRACER_MAGIC,TRACER_MAGIC_SIZE) != 0))
    {
      ERROR("tracer: read file magic value error\n");
      return 1;
    }
  DMSG(t,"tracer:hdr: file id      : ok\n");

  /* version */
  r += fread(&version,1,1,t->in_fd);
  DMSG(t,"tracer:hdr: version      : %d\n",version);
  if (version != TRACER_VERSION)
    {
      ERROR("tracer: version mismatch. reader version = %d, file version = %d\n",TRACER_VERSION,version);
      return 1;
    }
  
  /* endianess */
  r += fread(&e,1,1,t->in_fd);
  DMSG(t,"tracer:hdr: endian       : %d\n",e);

#if defined(WORDS_BIGENDIAN)
  te = 0;
#else
  te = 1;
#endif

  if (e == te)
    {
      /* DMSG(t,"tracer: hdr: recording machine was %s, host is %s\n",endian_tostring(te),endian_tostring(e)); */
      t->hdr.need_endian_swap = 0;
    }
  else
    {
      /* DMSG(t,"tracer: hdr: host and simulation machine have different endianess\n"); */
      t->hdr.need_endian_swap = 1;
    }
  DMSG(t,"tracer:hdr: swap         : %d\n",t->hdr.need_endian_swap);

  /* backtracks uint32_t */
  r += fread(&(t->hdr.backtracks), 1, sizeof(t->hdr.backtracks),t->in_fd);
  DMSG(t,"tracer:hdr: backtrack    : %d\n",t->hdr.backtracks);

  /* simulated cycles */
  r += fread(&(t->hdr.simul_cycles), 1, sizeof(t->hdr.simul_cycles), t->in_fd);
  DMSG(t,"tracer:hdr: cycles       : %"PRId64"\n",t->hdr.simul_cycles);

  /* simulated nanoseconds */
  r += fread(&(t->hdr.simul_nano), 1, sizeof(t->hdr.simul_nano), t->in_fd);
  DMSG(t,"tracer:hdr: nano         : %"PRId64"\n",t->hdr.simul_nano);
  
  /* simulated instructions */
  r += fread(&(t->hdr.simul_insn), 1, sizeof(t->hdr.simul_insn), t->in_fd);
  DMSG(t,"tracer:hdr: insn         : %"PRId64"\n",t->hdr.simul_insn);

  /* node id */
  r += fread(&(t->hdr.node_id), 1, sizeof(t->hdr.node_id),t->in_fd);
  DMSG(t,"tracer:hdr: node id      : %d\n",t->hdr.node_id);

  /* initial time */
  r += fread(&(t->hdr.initial_time), 1, sizeof(t->hdr.initial_time),t->in_fd);
  DMSG(t,"tracer:hdr: init time    : %"PRIu64"\n",t->hdr.initial_time);

  /* max number of id */
  r += fread(&(t->hdr.tracer_max_id),1,sizeof(t->hdr.tracer_max_id),t->in_fd);
  DMSG(t,"tracer:hdr: max id       : %d\n",t->hdr.tracer_max_id);

  /* number of events */
  r += fread(&(t->hdr.ev_count_total),1,sizeof(t->hdr.ev_count_total),t->in_fd);
  DMSG(t,"tracer:hdr: number       : %d\n",t->hdr.ev_count_total);

  /* simulation time */
  r += fread(&(t->hdr.sim_time_total),1,sizeof(tracer_time_t),t->in_fd);
  DMSG(t,"tracer:hdr: nano         : %" PRId64 "\n",t->hdr.sim_time_total);

  /* events name */
  r += fread(t->hdr.id_name,1,t->hdr.tracer_max_id * TRACER_MAX_NAME_LENGTH,t->in_fd); 

  /* events modules */
  r += fread(t->hdr.id_module,1,t->hdr.tracer_max_id * TRACER_MAX_NAME_LENGTH,t->in_fd); 

  /* events width */
  r += fread(t->hdr.id_width,  1,sizeof(tracer_width_t) * t->hdr.tracer_max_id,t->in_fd);

  /* id_count, id_min, id_max */
  r += fread(t->hdr.id_count  ,1,sizeof(tracer_ev_t)  * t->hdr.tracer_max_id,t->in_fd);
  r += fread(t->hdr.id_val_min,1,sizeof(tracer_val_t) * t->hdr.tracer_max_id,t->in_fd);
  r += fread(t->hdr.id_val_max,1,sizeof(tracer_val_t) * t->hdr.tracer_max_id,t->in_fd);

  t->hdr.header_length = r;
  DMSG(t,"tracer:hdr: bin size     : %d\n",t->hdr.header_length);

  if (t->hdr.need_endian_swap)
    {
      tracer_swap_header(t);
    }
  
  if (t->debug || t->verbose)
    {
      tracer_dump_header(t);
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void tracer_swap_sample(tracer_sample_t *s)
{
  s->id   = endian_swap4(s->id);
  s->time = endian_swap8(s->time);
  s->val  = endian_swap8(s->val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void tracer_swap_header(tracer_t *t)
{
  int i;
  t->hdr.backtracks     = endian_swap4(t->hdr.backtracks);
  t->hdr.simul_cycles   = endian_swap8(t->hdr.simul_cycles);
  t->hdr.simul_insn     = endian_swap8(t->hdr.simul_insn);
  t->hdr.simul_nano     = endian_swap8(t->hdr.simul_nano);
  t->hdr.node_id        = endian_swap2(t->hdr.node_id);
  t->hdr.initial_time   = endian_swap8(t->hdr.initial_time);
  t->hdr.tracer_max_id  = endian_swap4(t->hdr.tracer_max_id);
  t->hdr.ev_count_total = endian_swap4(t->hdr.ev_count_total);
  t->hdr.sim_time_total = endian_swap8(t->hdr.sim_time_total);
  for(i=0; i < TRACER_MAX_ID; i++)
    {
      t->hdr.id_count  [i] = endian_swap4(t->hdr.id_count  [i]);
      t->hdr.id_val_min[i] = endian_swap8(t->hdr.id_val_min[i]);
      t->hdr.id_val_max[i] = endian_swap8(t->hdr.id_val_max[i]);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_dup(tracer_t *dest,tracer_t *source)
{
  long pos;
  memcpy(dest,source,sizeof(struct tracer_struct_t));
  pos = ftell(source->in_fd);

  dest->in_fd     = fdopen(dup(fileno(source->in_fd)),"rb");
  dest->out_fd    = NULL;
  dest->dir       = NULL;

  fseek(dest->in_fd, pos, SEEK_SET);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_dump_header(tracer_t *t)
{
  int i;
  OUTPUT("tracer:hdr: node id %d\n",t->hdr.node_id);
  OUTPUT("tracer:hdr: id max %d\n",t->hdr.tracer_max_id);
  OUTPUT("tracer:hdr: ev_count %d\n",t->hdr.ev_count_total);
  OUTPUT("tracer:hdr: simulation time %"PRId64"\n",t->hdr.sim_time_total);
  for(i=0; i < TRACER_MAX_ID; i++)
    {
      if (t->hdr.id_name[i][0] != '\0')
	{
	  OUTPUT("tracer:hdr: id=%2d %10s::%-15s [%2d] count=%4d, min=%"PRId64", max=%"PRId64"\n", i,
		 t->hdr.id_module[i],t->hdr.id_name[i],t->hdr.id_width[i],t->hdr.id_count[i],t->hdr.id_val_min[i],t->hdr.id_val_max[i]);
	}
    } 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tracer_delete(tracer_t *t)
{
  if (t)
    {
      free(t);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

tracer_id_t tracer_find_id_by_name (tracer_t *t, char* name)
{
  int i;
  for(i=0; i < t->hdr.tracer_max_id; i++)
    {
      if (strcmp(name,t->hdr.id_name[i]) == 0)
	{
	  DMSG(t,"tracer: find %s by id = %d\n",name,i);
	  return i;
	}
    }
  return -1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_read_sample(tracer_t *t, tracer_sample_t *s)
{
  size_t size = sizeof(tracer_sample_t);
  if (fread(s, 1, size, t->in_fd) != size)
    {
      DMSG(t,"tracer:sample: read error\n");
      return 0;
    }

  if (t->hdr.need_endian_swap)
    {
      tracer_swap_sample(s);
    }

  /* DMSG(t,"tracer:sample: id: %03x time: %010"PRIx64" val:%010"PRIx64"\n", s->id,s->time,s->val); */

  return 1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define TRACER_MAX_DRIVERS 10
struct tracer_registered_drivers_struct_t {
  int max;
  tracer_driver_t drivers[TRACER_MAX_DRIVERS];
};

typedef struct tracer_registered_drivers_struct_t tracer_registered_drivers_t;
static tracer_registered_drivers_t tracer_registered_drivers = 
 { 
   0,
   {
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL }
   }
 };

void tracer_driver_register(tracer_driver_t *drv)
{
  int n = tracer_registered_drivers.max;
  memcpy(& tracer_registered_drivers.drivers[n], drv, sizeof(tracer_driver_t));
  tracer_registered_drivers.max ++;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

tracer_driver_t *tracer_driver_get_by_name(char* name)
{
  int i,n;
  n = tracer_registered_drivers.max;
  for(i=0; i < n ; i++)
    {
     if (strcmp(name, tracer_registered_drivers.drivers[i].name) == 0)
	{
	  return & (tracer_registered_drivers.drivers[i]);
	}
    }
  return NULL;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_file_out_open(tracer_t *t)
{
  if (strcmp(t->out_filename,"-") == 0)
    {
      t->out_fd = stdout;
      return 0;
    }

  if ((t->out_fd = fopen(t->out_filename,"wb")) == NULL)
    {
      ERROR("tracer:file: error on open out file %s\n",t->out_filename);
      return 1;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int tracer_file_out_close(tracer_t *t)
{
  if (t->out_fd)
    {
      fclose(t->out_fd);
      t->out_fd = NULL;
      return 0;
    }
  else
    {
      ERROR("tracer:file: close error on output %s\n",t->out_filename);
    }
  return 1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
