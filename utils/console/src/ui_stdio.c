
/**
 *  \file    ui_stdio.c
 *  \brief   Terminal for wsim serial line emulation 
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "ui.h"

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int read_print_char(int fd)
{
  int  r,i;
  char c[250];
  int  stop = 0;

  printf("  ");
  if ((r=read(fd,c,250)) > 0)
    {
      for(i=0; i < r ; i++)
	{
	  switch (c[i])
	    {
	    case 0x04: /* C-d */
	      stop = 1;
	      break;
	    case 0x0a:
	      fprintf(stdout,"[lf]");
	      break;
	    case 0x0d:
	      fprintf(stdout,"[cr]");
	      break;
	    default:
	      fprintf(stdout,"[%c]",c[i]);
	      break;
	    }
	}
      fflush(stdout);
    }
  else
    {
      fprintf(stdout,"read returned with a bad value\n");
      perror("read");
    }

  fprintf(stdout,"-- %d",r);
  fprintf(stdout,"\n");
  if (r == -1)
    {
      stop = 1;
    }
  return stop;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int tty_reset(int fd, struct termios *termios_backup) 
{
  if (tcsetattr(fd, TCSAFLUSH, termios_backup) < 0)
    return -1;
  
  return 0;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define STDIN 0

struct termios stdin_termios;

void tty_stdin_reset(void)
{
  tty_reset(STDIN,&stdin_termios);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int tty_raw(int fd, struct termios *termios_backup)
{      
    struct termios  buf;

    if (tcgetattr(fd, termios_backup) < 0) /* get the original state */
        return -1;

    memcpy(&buf,termios_backup,sizeof(struct termios));
    
    /*
      echo off, 
      canonical mode off, 
      extended input processing off, 
    */
    //    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    buf.c_lflag &= ~(ICANON | IEXTEN);

    /* no SIGINT on BREAK, CR-toNL off, input parity
       check off, don't strip the 8th bit on input,
       ouput flow control off */
    // buf.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);

    /* clear size bits, parity checking off */
    // buf.c_cflag &= ~(CSIZE | PARENB);

    /* set 8 bits/char */
    // buf.c_cflag |= CS8;

    /* output processing off */
    // buf.c_oflag &= ~(OPOST);

    buf.c_cc[VMIN] = 1;  /* 1 byte at a time */
    buf.c_cc[VTIME] = 0; /* no timer on input */

    if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
        return -1;

    return 0;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     


struct ui_t*
ui_create(char* name, int w, int h, int n)
{
  return NULL;
}

void
ui_delete(struct ui_t* ui) 
{ 
}

void ui_setname(char* name, char* icon)
{
}

int
ui_kbd_event(struct ui_t *ui)
{
  return UI_ERROR;
}

void
ui_putchar(struct ui_t *ui, int n, char c)
{
}



#if 0
  if (tty_raw(STDIN,&stdin_termios) < 0)
    {
      fprintf(stderr,"Cannot set tty raw mode\n");
      return 1;
    }

  atexit(tty_stdin_reset);

  while (c = getchar())
    {
      fprintf(stdout,"[%c]",c);
    }
#endif
