
/**
 *  \file    ui_screen.c
 *  \brief   Terminal for wsim serial line emulation 
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "ui_screen.h"

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define ERROR(x...) fprintf(stderr,x)

#undef DEBUG
#if defined(DEBUG)
#define DBG_MSG(x...) fprintf(stderr,x)
#else
#define DBG_MSG(x...) do { } while(0)
#endif

#define CHECK_NULL_SCREEN(s)           \
do {                                   \
  if (s == NULL)                       \
    {                                  \
      ERROR("NULL screen scroll\n");   \
      return ;                         \
    }                                  \
} while(0)

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

typedef unsigned char  char_t;
typedef char_t        *line_t;

struct s_screen_t {
  int      num;    /* screen number */
  int      width;
  int      height;

  int      posx;   /* cursor position */
  int      posy;

  line_t  *lines;  /* text */
};

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

screen_t screen_create(int n, int w, int h)
{
  int i;
  screen_t s = NULL;

  if ((s = malloc(sizeof(struct s_screen_t))) == NULL)
    {
      ERROR("Screen %d malloc error [%s]",n,strerror(errno));
      return NULL;
    }

  s->num      = n;
  s->width    = w;
  s->height   = h;
  s->posx     = 0;
  s->posy     = 0;

  assert(s->lines = (line_t*)malloc(s->height * sizeof(line_t)));
  for(i=0; i < s->height; i++)
    {
      assert(s->lines[i] = (line_t)malloc(s->width * sizeof(char_t)));
    }

  screen_clear(s);

  return s;
}


/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void screen_delete(screen_t s)
{
  int i;
  CHECK_NULL_SCREEN(s);
  for(i=0; i < s->height; i++)
  {
    free(s->lines[i]);
  }
  free(s->lines);
  free(s);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void
screen_clear(screen_t s)
{
  int i;
  CHECK_NULL_SCREEN(s);
  s->posx = 0;
  s->posy = 0;
  for(i=0; i < s->height; i++)
    {
      memset(s->lines[i],' ', s->width * sizeof(char_t));
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

static void
screen_scroll_up(screen_t s)
{
  int i;
  CHECK_NULL_SCREEN(s);
  for(i=0; i < (s->height - 1); i++)
    {
      memcpy(s->lines[i], s->lines[i+1], s->width * sizeof(char_t));
    }
  memset(s->lines[s->height - 1],' ', s->width * sizeof(char_t));
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

static void 
screen_next_line(screen_t s)
{
  CHECK_NULL_SCREEN(s);
  if (s->posy == s->height - 1)
    {
      /* s->posy does not change */
      screen_scroll_up(s);
    }
  else
    {
      s->posy ++;
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void
screen_write(screen_t s, char c)
{
  CHECK_NULL_SCREEN(s);

  switch (c)
    {
    case 0x00: /* key modifier */
      break;
    case 0x04: /* ^D */
      break;
    case 0x08: /* backspace */
      break;
    case 0x7f: /* delete */
      break;
    case 0x0d: /* LF */
      s->posx = 0;
    case 0x0a: /* CR */
      screen_next_line(s);
      s->posx = 0;
      break;

    default:
      DBG_MSG("Screen: write [%c] at (%d,%d)\n",c,s->posx,s->posy);
      s->lines[s->posy][s->posx++] = c;
      if (s->posx == s->width) /* go next line */
	{
	  s->posx = 0;
	  screen_next_line(s);
	}
      break;
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int screen_get_height(screen_t s)
{
  return (s == NULL) ? 0 : s->height;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int screen_get_width(screen_t s)
{
  return (s == NULL) ? 0 : s->width;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

unsigned char screen_get_char(screen_t s, int x, int y)
{
  char_t c;

  if (s == NULL)
    {
      c = 0;
    }
  else
    {
      if ((x < 0) || (x > s->width) || (y < 0) || (y > s->height))
	{
	  ERROR("Out of bound screen %d get (%d%d)\n",s->num,x,y);
	  c = 0;
	}
      else
	{
	  c = s->lines[y][x];
	}
    }

  return c;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

