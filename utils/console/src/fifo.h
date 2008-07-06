
/**
 *  \file    fifo.h
 *  \brief   Terminal for wsim serial line emulation (device fifo)
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#ifndef __FIFO_H_
#define __FIFO_H_

#define FIFO_NOEVENT        0x0100u
#define FIFO_SELECT_ERROR   0x0200u
#define FIFO_IO_ERROR       0x0400u
#define FIFO_QUIT           0x0800u

#define MAX_FILENAME        256
#define INPUT_FIFO_SIZE     1024
#define OUTPUT_FIFO_SIZE    1024

#define FIFO_BLOCKING       1
#define FIFO_NONBLOCKING    2

#include <termios.h>

struct fifo_t {
  int    fd_in;
  int    fd_out;

  struct termios in_termios;
  struct termios out_termios;

  char   local_name[MAX_FILENAME];
  char   remote_name[MAX_FILENAME];

  unsigned char   input_val[INPUT_FIFO_SIZE];
  unsigned int    input_read_ptr;
  unsigned int    input_write_ptr;
  unsigned int    input_state;

  unsigned char   output_val[OUTPUT_FIFO_SIZE];
  unsigned int    output_read_ptr;
  unsigned int    output_write_ptr;
  unsigned int    output_state;
};

struct fifo_t* fifo_init      (const char* filename);
void           fifo_delete    (struct fifo_t *fifo);
void           fifo_putchar   (struct fifo_t *fifo, char c);
int            fifo_event     (struct fifo_t *fifo, int blocking);
void           fifo_post_quit (struct fifo_t *fifo);
#endif
