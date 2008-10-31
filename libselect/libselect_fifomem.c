
/**
 *  \file   libselect_fifomem.c
 *  \brief  Fifo buffer mems for wsim select() wrapper
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdio.h>

#include "config.h"
#include "liblogger/logger.h"
#include "libselect_fifomem.h"

/****************************************
 * DEBUG
 * 
 * DMSG is used for general tracer messages
 * while debugging select code
 ****************************************/
#define DEBUG_FIFO

#if defined(DEBUG_FIFO)
#define DMSG(x...) fprintf(stderr,x)
#else
#define DMSG(x...) do {} while(0)
#endif

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

struct libselect_fifo_struct_t {
  unsigned char*  val;
  unsigned int    read_ptr;
  unsigned int    write_ptr;
  unsigned int    state;
  unsigned int    size;
};

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

libselect_fifo_t libselect_fifo_create(int size)
{
  struct libselect_fifo_struct_t *fifo = NULL;
  
  if ((fifo = malloc(sizeof(struct libselect_fifo_struct_t))) == NULL)
    {
      ERROR("libselect_fifo: malloc error [%s]\n",strerror(errno));
      return NULL;
    }

  memset(fifo,0,sizeof(struct libselect_fifo_struct_t));

  if ((fifo->val = (unsigned char*)malloc(sizeof(unsigned char)*size)) == NULL)
    {
      ERROR("libselect_fifo: malloc error [%s]\n",strerror(errno));
      free(fifo);
      return NULL;
    }

  fifo->size = size;
  return fifo;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void libselect_fifo_delete (libselect_fifo_t fifo)
{
  if (fifo)
    {
      free(fifo->val);
      fifo->val = NULL;

      free(fifo);
      fifo = NULL;
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_flush(libselect_fifo_t fifo)
{
  if (fifo)
    {
      fifo->read_ptr   = 0;
      fifo->write_ptr  = 0;
      fifo->state      = 0;

      return LIBSELECT_FIFO_OK;
    }
  return LIBSELECT_FIFO_ERROR;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_size(libselect_fifo_t fifo)
{
  return fifo->state;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_putchar(libselect_fifo_t fifo, unsigned char c)
{
  if (fifo->state == fifo->size)
    {
      return 0;
    }

  fifo->val[fifo->write_ptr] = c;
  fifo->state     =  fifo->state     + 1;
  fifo->write_ptr = (fifo->write_ptr + 1) % fifo->size;
  return 1;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_putblock(libselect_fifo_t fifo, unsigned char *data, unsigned int size)
{
  int part1, part2;
  if ((fifo->state + size) > fifo->size)
    {
      DMSG("libselect_fifo:     fifo size %d, rdptr %d, wrptr %d\n",
	    fifo->size, fifo->read_ptr, fifo->write_ptr);
      return size - (fifo->size - fifo->state);
    }

  /*
   *      part2          part1
   * [0] ....... [wptr] .......  [size]
   *
   */

  DMSG("libselect_fifo: write block %d bytes\n",size);
  DMSG("libselect_fifo:      read %3d write %3d state=%3d/%3d\n",
       fifo->read_ptr,fifo->write_ptr,fifo->state,fifo->size);

  part1 = fifo->size - fifo->write_ptr; 
  part2 = size       - part1;

  if (part2 > 0)
    {
      memcpy(fifo->val + fifo->write_ptr, data        , part1);
      memcpy(fifo->val                  , data + part1, part2);
    }
  else
    {
      memcpy(fifo->val + fifo->write_ptr, data        , size);
    }

  fifo->state     =  fifo->state     + size;
  fifo->write_ptr = (fifo->write_ptr + size) % fifo->size;

  DMSG("libselect_fifo:      read %3d write %3d state=%3d/%3d\n",
       fifo->read_ptr,fifo->write_ptr,fifo->state,fifo->size);
  
  return size;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_getchar(libselect_fifo_t fifo, unsigned char *c)
{
  if (fifo->size == 0)
    {
      *c = 0;
      return 0;
    }

  DMSG("libselect_fifo: getchar\n");
  DMSG("libselect_fifo:      read %3d write %3d state=%3d/%3d\n",
       fifo->read_ptr,fifo->write_ptr,fifo->state,fifo->size);
  *c = fifo->val[fifo->read_ptr];
  fifo->state    =  fifo->state    - 1;
  fifo->read_ptr = (fifo->read_ptr + 1) % fifo->size;
  DMSG("libselect_fifo:      read %3d write %3d state=%3d/%3d\n",
       fifo->read_ptr,fifo->write_ptr,fifo->state,fifo->size);
  return 1;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_getblock(libselect_fifo_t fifo, unsigned char *data, unsigned int size)
{
  int part1, part2;  
  if (size > fifo->state)
    {
      return fifo->state;
    }

  /*
   *      part2          part1
   * [0] ....... [rptr] .......  [size]
   *
   */

  DMSG("libselect_fifo: read block %d bytes\n",size);
  DMSG("libselect_fifo:      read %3d write %3d state=%3d/%3d\n",
       fifo->read_ptr,fifo->write_ptr,fifo->state,fifo->size);

  part1 = fifo->size - fifo->read_ptr;
  part2 = size       - part1;

  if (part2 > 0)
    {
      memcpy(data        , fifo->val + fifo->read_ptr, part1);
      memcpy(data + part1, fifo->val                 , part2);
    }
  else
    {
      memcpy(data        , fifo->val + fifo->read_ptr, size);
    }

  fifo->state    =  fifo->state    - size;
  fifo->read_ptr = (fifo->read_ptr + size) % fifo->size;

  DMSG("libselect_fifo:      read %3d write %3d state=%3d/%3d\n",
       fifo->read_ptr,fifo->write_ptr,fifo->state,fifo->size);
  return size;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     
