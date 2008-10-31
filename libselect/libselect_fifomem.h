
/**
 *  \file   libselect_fifomem.h
 *  \brief  Fifo buffer mems for wsim select() wrapper
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef LIBSELECT_FIFOMEM_H
#define LIBSELECT_FIFOMEM_H

typedef struct libselect_fifo_struct_t *libselect_fifo_t;

#define LIBSELECT_FIFO_OK      0x000u
#define LIBSELECT_FIFO_ERROR   0x100u

libselect_fifo_t libselect_fifo_create (int size);
void             libselect_fifo_delete (libselect_fifo_t fifo);

int libselect_fifo_size     (libselect_fifo_t fifo);
int libselect_fifo_flush    (libselect_fifo_t fifo);

int libselect_fifo_getchar  (libselect_fifo_t fifo, unsigned char *c);
int libselect_fifo_getblock (libselect_fifo_t fifo, unsigned char *data, unsigned int size);

int libselect_fifo_putchar  (libselect_fifo_t fifo, unsigned char  c);
int libselect_fifo_putblock (libselect_fifo_t fifo, unsigned char *data, unsigned int size);

#endif
