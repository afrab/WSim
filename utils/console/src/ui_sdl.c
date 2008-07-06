
/**
 *  \file    ui_sdl.c
 *  \brief   Terminal for wsim serial line emulation (SDL)
 *  \author  Antoine Fraboulet
 *  \date    2005
 *  \license GPLv2
 **/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

#include "config.h"
#include "ui.h"
#include "ui_font.h"

#define ERROR(x...) fprintf(stderr,x)


#if defined(DEBUG)
#define DBG_MSG(x...) fprintf(stderr,x)
#else
#define DBG_MSG(x...) do { } while(0)
#endif

//#define DEBUG_ME_HARDER
#if defined(DEBUG_ME_HARDER)
#define DBG_MSG2(x...) fprintf(stderr,x)
#else
#define DBG_MSG2(x...) do { } while(0)
#endif

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define LINE_SEP    0
#define SCREEN_SEP  10

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

struct ui_t *ui_create(char* name, int screen_w, int screen_h, int mode)
{
  int i;
  int desired_bpp;
  Uint32 video_flags;
  char windowname[200];

  struct ui_t *ui = NULL;
  if ((ui = malloc(sizeof(struct ui_t))) == NULL)
    {
      ERROR("Fifo malloc error [%s]",strerror(errno));
      return NULL;
    }

  memset(ui,0,sizeof(struct ui_t));

  ui->screen_width  = screen_w;
  ui->screen_height = screen_h;

  switch(mode)
    {
    case UI_MODE_NONE:
      ERROR("wconsole:ui: cannot create ui, mode NONE set\n");
      return NULL;
      break;
    case UI_MODE_OUTPUT:
      ui->nscreens = 1;
      break;
    case UI_MODE_INPUT:
      ui->nscreens = 1;
      break;
    case UI_MODE_IO:
      ui->nscreens = 2;
      break;
    default:
      ERROR("wconsole:ui: cannot create ui, unknown mode set\n");
      return NULL;
      break;
    }

  DBG_MSG("Create window %d*(%d,%d)\n",ui->nscreens,ui->screen_width,ui->screen_height);

  ui->screen    = (screen_t*)malloc(ui->nscreens * sizeof(screen_t));
  for(i=0; i < ui->nscreens; i++)
    {
      ui->screen[i] = screen_create(i,ui->screen_width,ui->screen_height);
    }

  /* font */
  ui->window_width  = ui->screen_width * FONT_WIDTH;
  ui->window_height = ui->nscreens * (ui->screen_height * (FONT_HEIGHT + LINE_SEP) + SCREEN_SEP);

  DBG_MSG("Window %d*%d - Screen %d*%d - Font %d*%d\n",
	  ui->window_width,ui->window_height,ui->screen_width,ui->screen_height,FONT_WIDTH,FONT_HEIGHT);

  /* sdl */
  ui->window_mustlock = 0;
  ui->window_bpp      = 3;

  desired_bpp = ui->window_bpp * 8;
  video_flags = 0;

  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
    {
      fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
      return NULL;
    }

  atexit(SDL_Quit);			/* Clean up on exit */
  ui->sdl_screen = SDL_SetVideoMode(ui->window_width, ui->window_height, desired_bpp, video_flags);

  if ( ui->sdl_screen == NULL ) 
    {
      fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
	      ui->window_width, ui->window_height, desired_bpp, SDL_GetError());
      return NULL;
    }

  DBG_MSG("Pixel format 0x%04x\n",(unsigned int)SDL_MapRGB(ui->sdl_screen->format, 250,  250,  250));
  DBG_MSG("Display %dx%dx%d \n",ui->sdl_screen->w, ui->sdl_screen->h,ui->sdl_screen->format->BitsPerPixel);
  DBG_MSG("   %d bytes per pixel, pitch %d\n",ui->sdl_screen->format->BytesPerPixel,ui->sdl_screen->pitch);
  DBG_MSG("   mask R 0x%08x G 0x%08x B 0x%08x A 0x%08x\n",
	  ui->sdl_screen->format->Rmask,ui->sdl_screen->format->Gmask,
	  ui->sdl_screen->format->Bmask,ui->sdl_screen->format->Amask);
  DBG_MSG("   colorkey 0x%08x\n",ui->sdl_screen->format->colorkey);

  /* Enable UNICODE translation for keyboard input */
  SDL_EnableUNICODE(1); 

  /* Enable auto repeat for keyboard input */
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  /* Enable names */
  sprintf(windowname,"Wsim serial I/O %s",name);
  SDL_WM_SetCaption(windowname, "Wsim sio");

  return ui;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void
ui_delete(struct ui_t* ui)
{
  int i;
  if (ui)
    {
      for(i=0; i < ui->nscreens; i++)
	{
	  screen_delete(ui->screen[i]);
	}
      free(ui->screen);
      free(ui);
      SDL_Quit();
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int
ui_kbd_event(struct ui_t *ui, int mode)
{
  int sdl_ret = 0;
  int ret     = UI_NOEVENT;

  SDL_Event event;

  if (ui == NULL)
    return ret;

  switch (mode)
    {
    case KBD_BLOCKING:
      sdl_ret = SDL_WaitEvent(&event);
      if (sdl_ret == 0) /* error */
	{
	  ret = UI_QUIT;
	}
      break;
    case KBD_NONBLOCKING:
      sdl_ret = SDL_PollEvent(&event);
      break;
    default:
      ERROR("wconsole:kbd: unknown polling mode\n");
      break;
    }

  if (sdl_ret == 1)
    {
      switch (event.type) 
	{
	case SDL_KEYDOWN:
	  DBG_MSG2("SDL: key scancode %d = [0x%02x]\n",event.key.keysym.scancode,event.key.keysym.unicode);
	  if (event.key.keysym.unicode)
	    {
	      ret = event.key.keysym.unicode;
	    }
	  break;

	case SDL_MOUSEBUTTONDOWN:
	  break;

	case SDL_QUIT:
	  DBG_MSG("==============\n");
	  DBG_MSG("== UI  quit ==\n");
	  DBG_MSG("==============\n");
	  ret = UI_QUIT;
	  break;

	default:
	  break;
	}
    }  
  return ret;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#if defined(WORDS_BIGENDIAN)
#define setpixel(buffer,pixel,r,v,b)        \
  do {                                      \
  buffer[pixel+0]  = r;                     \
  buffer[pixel+1]  = v;                     \
  buffer[pixel+2]  = b;                     \
  } while (0)      
#else
#define setpixel(buffer,pixel,r,v,b)        \
  do {                                      \
  buffer[pixel+2]  = r;                     \
  buffer[pixel+1]  = v;                     \
  buffer[pixel+0]  = b;                     \
  } while (0)      
#endif

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define BG_R 0x00
#define BG_V 0x00
#define BG_B 0x00

static void
ui_draw_font(Uint8 *buffer, struct ui_t *ui, int px, int py, unsigned char c, 
	     unsigned char r, unsigned char v, unsigned char b)
{
  int cx,cy;
  int pixel;

  pixel = (px + py * ui->window_width) * ui->window_bpp;

  for(cy=0; cy < FONT_HEIGHT; cy++)
    {
      int pii = pixel;
      for(cx=0; cx < FONT_WIDTH; cx++)
	{
	  if (font[c][cy][cx] != 0)
	    {
	      setpixel(buffer,pii,r,v,b);
	    }
	  else
	    {
	      setpixel(buffer,pii,BG_R,BG_V,BG_B);
	    }
#if defined(DEBUG_ME_HARDER)
	  if (isalpha(c))
	    {
	      DBG_MSG("%c", font[c][cy][cx] ? 'X':' ');
	    }
#endif
	  pii += ui->window_bpp;
	}

#if defined(DEBUG_ME_HARDER)
      if (isalpha(c))
	{
	  DBG_MSG("\n");
	}
#endif
      pixel += ui->window_width * ui->window_bpp;
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

static void
ui_draw_screen(Uint8 *buffer, struct ui_t *ui, int n)
{
  int height, width;

  int cw,ch;
  int px,py;

  px = 0;
  py = n * (ui->screen_height * (FONT_HEIGHT + LINE_SEP) + SCREEN_SEP - 1);

  height = screen_get_height(ui->screen[n]);
  width  = screen_get_width (ui->screen[n]);

  for(ch=0; ch < height; ch++)
    {
      for(cw=0; cw < width; cw++)
	{
	  unsigned char cc = screen_get_char(ui->screen[n],cw,ch);
#if defined(DEBUG_ME_HARDER)
	  DBG_MSG("SDL: drawing screen %d at %02d,%02d\n",n,cw,ch);
	  if (isalpha(cc))
	    {
	      DBG_MSG("SDL: draw [%c] on screen %d\n",cc,n);
	    }
#endif
	  if (n == 0)
	    {
	      ui_draw_font(buffer,ui,px,py,cc,0x8fu,0x8fu,0x8fu);
	    }
	  else
	    {
	      ui_draw_font(buffer,ui,px,py,cc,0xffu,0xffu,0xffu);
	    }
	  px += FONT_WIDTH;
	}
      py += FONT_HEIGHT;
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void
ui_refresh(struct ui_t *ui)
{
  int i;
  Uint8 *buffer;

  if ((ui == NULL) || (ui->sdl_screen == NULL))
    return ;

  if (ui->window_mustlock &&  SDL_LockSurface(ui->sdl_screen) < 0 ) 
    {
      fprintf(stderr, "Couldn't lock the display surface: %s\n",SDL_GetError());
      return ;
    }

  buffer=(Uint8 *)ui->sdl_screen->pixels;

  for(i=0; i < ui->nscreens ; i++)
    {
      ui_draw_screen(buffer,ui,i);
    }
  
  if (ui->window_mustlock)
    {
      SDL_UnlockSurface(ui->sdl_screen);
    }
  
  SDL_UpdateRect(ui->sdl_screen, 0, 0, 0, 0);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void
ui_putchar(struct ui_t *ui, int n, char c)
{
  if ((ui == NULL) || n > (ui->nscreens - 1) || (ui->screen[n] == NULL))
    return;

  DBG_MSG2("SDL: putchar [0x%02x] on screen %d\n",c,n);
  screen_write(ui->screen[n],c);
  ui_refresh(ui);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void ui_setname(char* name,char *icon)
{
  SDL_WM_SetCaption(name,icon);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void ui_post_quit(struct ui_t *ui)
{
  SDL_QuitEvent ev;
  SDL_PushEvent((SDL_Event*)&ev);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     
