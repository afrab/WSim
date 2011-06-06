
/**
 *  \file   libselect_fifomem.h
 *  \brief  Fifo buffer mems for wsim select() wrapper
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef LIBSELECT_FIFOMEM_H
#define LIBSELECT_FIFOMEM_H


#define LIBSELECT_FIFO_OK      0x000u
#define LIBSELECT_FIFO_ERROR   0x100u

/* 
 * output fifo
 *    writes are coming from Wsim
 *    fifo data are flushed on sync and rendez-vous
 *
 * input fifo 
 *    writes are coming from outside world
 *    read can be cancelled on backtracks
 */

typedef struct libselect_fifo_input_struct_t  *libselect_fifo_input_t;
typedef struct libselect_fifo_output_struct_t *libselect_fifo_output_t;

#define FIFO_INPUT  1
#define FIFO_OUTPUT 2

libselect_fifo_input_t libselect_fifo_input_create (int id, int size);
void                   libselect_fifo_input_delete (libselect_fifo_input_t fifo);
int  libselect_fifo_input_avail      (libselect_fifo_input_t fifo);
int  libselect_fifo_input_putblock   (libselect_fifo_input_t fifo, unsigned char *data, unsigned int size);
int  libselect_fifo_input_getblock   (libselect_fifo_input_t fifo, unsigned char *data, unsigned int size);
int  libselect_fifo_input_readblock  (libselect_fifo_input_t fifo, unsigned char *data, unsigned int size);
int  libselect_fifo_input_readcommit (libselect_fifo_input_t fifo);
int  libselect_fifo_input_readcancel (libselect_fifo_input_t fifo);

libselect_fifo_output_t libselect_fifo_output_create (int id, int size);
void                    libselect_fifo_output_delete (libselect_fifo_output_t fifo);
int  libselect_fifo_output_avail      (libselect_fifo_output_t fifo);
int  libselect_fifo_output_putblock   (libselect_fifo_output_t fifo, unsigned char *data, unsigned int size);
int  libselect_fifo_output_getblock   (libselect_fifo_output_t fifo, unsigned char *data, unsigned int size);
int  libselect_fifo_output_flush      (libselect_fifo_output_t fifo);

#endif
