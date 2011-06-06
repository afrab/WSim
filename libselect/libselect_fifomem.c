
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
#include <assert.h>

#include "config.h"
#include "liblogger/logger.h"
#include "libselect_fifomem.h"

/****************************************
 * DEBUG
 * 
 * DMSG is used for general tracer messages
 * while debugging select code
 ****************************************/

#define DEBUG_FIFO  1
#define DEBUG_FIFO2 1

#if DEBUG_FIFO != 0
#define DMSG(x...)  DMSG_LIB_SELECT(x)
#else
#define DMSG(x...) do {} while(0)
#endif

#if DEBUG_FIFO2 != 0
#define DMSG2(x...) DMSG_LIB_SELECT(x)
#else
#define DMSG2(x...) do {} while(0)
#endif

#define DEBUG_OUTPUT(msg)						\
  do {                                                                  \
    DMSG2("libselect_fifo:output:%02d:%-15s read %03d write %03d state=%03d/%3d\n", \
          fifo->id,							\
          msg, fifo->read_ptr, fifo->write_ptr,				\
          fifo->state, fifo->size);					\
  } while(0)

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

struct libselect_fifo_output_struct_t {
  unsigned char*  val;
  unsigned int    read_ptr;
  unsigned int    write_ptr;
  unsigned int    state;
  unsigned int    size;
  int             id;
};

struct libselect_fifo_input_struct_t {
  unsigned char*  val;
  unsigned int    read_ptr;
  unsigned int    read_ptr_virtual;
  unsigned int    write_ptr;
  unsigned int    state;
  unsigned int    state_virtual;
  unsigned int    size;
  int             id;
};

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

libselect_fifo_output_t libselect_fifo_output_create(int id, int size)
{
  struct libselect_fifo_output_struct_t *fifo = NULL;
  
  if ((fifo = malloc(sizeof(struct libselect_fifo_output_struct_t))) == NULL)
    {
      ERROR("libselect_fifo: malloc error [%s]\n",strerror(errno));
      return NULL;
    }

  memset(fifo,0,sizeof(struct libselect_fifo_output_struct_t));

  if ((fifo->val = (unsigned char*)malloc(sizeof(unsigned char)*size)) == NULL)
    {
      ERROR("libselect_fifo: malloc error [%s]\n",strerror(errno));
      free(fifo);
      return NULL;
    }

  fifo->id   = id;
  fifo->size = size;
  return fifo;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void libselect_fifo_output_delete (libselect_fifo_output_t fifo)
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

int libselect_fifo_output_flush(libselect_fifo_output_t fifo)
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

int libselect_fifo_output_avail(libselect_fifo_output_t fifo)
{
  return fifo->state;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_output_putblock(libselect_fifo_output_t fifo, unsigned char *data, unsigned int size)
{
  int part1, part2;
  if ((fifo->state + size) > fifo->size)
    {
      DMSG("libselect_fifo:output:%02d: fifo size %d, rdptr %d, wrptr %d\n",
           fifo->id, fifo->size, fifo->read_ptr, fifo->write_ptr);
      return size - (fifo->size - fifo->state);
    }

  /*
   *      part2          part1
   * [0] ....... [wptr] .......  [size]
   *
   */

  DMSG("libselect_fifo:output:%02d: write block %d bytes\n",fifo->id,size);
  DEBUG_OUTPUT("putblock1");

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
  DEBUG_OUTPUT("putblock2");
  
  return size;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_output_getblock(libselect_fifo_output_t fifo, unsigned char *data, unsigned int size)
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

  DMSG("libselect_fifo:output:%02d: read  block %d bytes\n",fifo->id,size);
  DEBUG_OUTPUT("getblock1");

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

  DEBUG_OUTPUT("getblock2");
  return size;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define DEBUG_INPUT(msg)						\
  do {                                                                  \
    DMSG2("libselect_fifo:input:%02d:%-15s read %03d,%03d write %03d state=%03d,%03d/%3d\n", \
          fifo->id,							\
          msg, fifo->read_ptr, fifo->read_ptr_virtual,fifo->write_ptr,	\
          fifo->state, fifo->state_virtual, fifo->size);			\
  } while(0)


/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

libselect_fifo_input_t libselect_fifo_input_create(int id, int size)
{
  struct libselect_fifo_input_struct_t *fifo = NULL;
  
  if ((fifo = malloc(sizeof(struct libselect_fifo_input_struct_t))) == NULL)
    {
      ERROR("libselect_fifo: malloc error [%s]\n",strerror(errno));
      return NULL;
    }

  memset(fifo,0,sizeof(struct libselect_fifo_input_struct_t));

  if ((fifo->val = (unsigned char*)malloc(sizeof(unsigned char)*size)) == NULL)
    {
      ERROR("libselect_fifo: malloc error [%s]\n",strerror(errno));
      free(fifo);
      return NULL;
    }

  fifo->id   = id;
  fifo->size = size;
  return fifo;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void libselect_fifo_input_delete (libselect_fifo_input_t fifo)
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

int libselect_fifo_input_avail(libselect_fifo_input_t fifo)
{
  return fifo->state_virtual;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_input_putblock(libselect_fifo_input_t fifo, unsigned char *data, unsigned int size)
{
  int part1, part2;
  /* 
   * fifo->state_virtual < fifo->state 
   * we need to ensure that the block will fit if we don't
   */
  if ((fifo->state + size) > fifo->size)
    {
      DMSG("libselect_fifo:input:%02d: fifo size %d, rdptr %d, wrptr %d\n",
           fifo->id, fifo->size, fifo->read_ptr, fifo->write_ptr);
      return size - (fifo->size - fifo->state);
    }

  /*
   *      part2          part1
   * [0] ....... [wptr] .......  [size]
   *
   */

  DMSG("libselect_fifo:input:%02d: write block %d bytes\n",fifo->id,size);
  DEBUG_INPUT("putblock1");

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

  fifo->state         =  fifo->state         + size;
  fifo->state_virtual =  fifo->state_virtual + size;
  fifo->write_ptr     = (fifo->write_ptr + size) % fifo->size;
  DEBUG_INPUT("putblock2");
  
  return size;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_input_getblock(libselect_fifo_input_t fifo, unsigned char *data, unsigned int size)
{
  int r1,r2;
  DMSG("libselect_fifo:input:%02d:getblock start %d bytes\n",fifo->id,size);
  r1 = libselect_fifo_input_readblock(fifo, data, size);
  r2 = libselect_fifo_input_readcommit(fifo);
  if (r1 != r2)
    {
      DMSG("libselect_fifo:input:getblock: ERROR r1 %d r2 %d\n",r1,r2);
    }
  assert(r1==r2);
  DMSG("libselect_fifo:input:%02d:getblock stop\n",fifo->id);
  return r1;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_input_readblock(libselect_fifo_input_t fifo, unsigned char *data, unsigned int size)
{
  int part1, part2;  
  if (size > fifo->state_virtual)
    {
      return fifo->state_virtual;
    }

  /*
   *      part2          part1
   * [0] ....... [rptr] .......  [size]
   *
   */

  DMSG("libselect_fifo:input:%02d:readblock start %d bytes\n",fifo->id,size);
  DEBUG_INPUT("readblock1");

  part1 = fifo->size - fifo->read_ptr_virtual;
  part2 = size       - part1;

  if (part2 > 0)
    {
      memcpy(data        , fifo->val + fifo->read_ptr_virtual, part1);
      memcpy(data + part1, fifo->val                         , part2);
    }
  else
    {
      memcpy(data        , fifo->val + fifo->read_ptr_virtual, size);
    }

  fifo->state_virtual    =  fifo->state_virtual    - size;
  fifo->read_ptr_virtual = (fifo->read_ptr_virtual + size) % fifo->size;

  DEBUG_INPUT("readblock2");
  DMSG("libselect_fifo:input:%02d:readblock stop\n",fifo->id);
  return size;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_input_readcommit(libselect_fifo_input_t fifo)
{
  int diff = fifo->state - fifo->state_virtual;
  DEBUG_INPUT("readcommit1");
  fifo->read_ptr         = fifo->read_ptr_virtual;
  fifo->state            = fifo->state_virtual;
  DEBUG_INPUT("readcommit2");
  return diff;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int libselect_fifo_input_readcancel(libselect_fifo_input_t fifo)
{
  int diff = fifo->state - fifo->state_virtual;
  DEBUG_INPUT("readcancel1");
  fifo->read_ptr_virtual = fifo->read_ptr;
  fifo->state_virtual    = fifo->state;
  DEBUG_INPUT("readcancel2");
  return diff;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     
