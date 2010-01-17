
/**
 *  \file   sdl_bkend.c
 *  \brief  WorldSens Graphical SDL UI definition 
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "src/mgetopt.h"
#include "src/options.h"

#include "SDL.h"

#include "ui.h"
#include "ui_bkend.h"

/**************************************************/
/**************************************************/
/**************************************************/

#if defined(DEBUG)
#define DMSG_UI(x...) HW_DMSG_UI(x)
#else
#define DMSG_UI(x...) do { } while (0)
#endif

/**************************************************/
/**************************************************/
/**************************************************/

void* ui_backend_create(int w, int h, char* title, int *mustlock)
{
  int desired_bpp;
  Uint32 video_flags;

  SDL_Surface * sdl_screen;
  sdl_screen  = NULL;

  /* init SDL */
  desired_bpp = 24;
  video_flags = 0;

  /* Enable UNICODE translation for keyboard input */
  /* SDL_EnableUNICODE(1); */

  if ( SDL_Init(SDL_INIT_VIDEO) == -1 ) 
    {
      ERROR("Couldn't initialize SDL: %s\n", SDL_GetError());
      return NULL;
    }

  /* Enable auto repeat for keyboard input */
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  atexit(SDL_Quit);			/* Clean up on exit */
  sdl_screen = SDL_SetVideoMode(w, h, desired_bpp, video_flags);

  if ( sdl_screen == NULL ) 
    {
      ERROR("Couldn't set %dx%dx%d video mode: %s\n",
	      w, h, desired_bpp, SDL_GetError());
      return NULL;
    }

  SDL_WM_SetCaption(title,title);

  HW_DMSG_UI("gui:Pixel format 0x%04x\n",
	     (unsigned int)SDL_MapRGB(sdl_screen->format, 250,  250,  250));
  HW_DMSG_UI("gui:Display %dx%dx%d \n",
	     sdl_screen->w, sdl_screen->h, sdl_screen->format->BitsPerPixel);
  HW_DMSG_UI("gui:   %d bytes per pixel, pitch %d\n", 
	     sdl_screen->format->BytesPerPixel, 
	     sdl_screen->pitch);
  HW_DMSG_UI("gui:   mask R 0x%08x G 0x%08x B 0x%08x A 0x%08x\n",
	     sdl_screen->format->Rmask, sdl_screen->format->Gmask,
	     sdl_screen->format->Bmask, sdl_screen->format->Amask);

  HW_DMSG_UI("gui:   colorkey 0x%08x\n", sdl_screen->format->colorkey);

  if (SDL_MUSTLOCK(sdl_screen) == 0)
    {
      *mustlock = 0;
    }
  else
    {
      *mustlock = 1;
    }

  if (sdl_screen->format->BytesPerPixel != 3)
    {
      ERROR("  Incorrect bytes per pixel format : core dump expected\n");
      return NULL;
    }

  return sdl_screen;
}


/**************************************************/
/**************************************************/
/**************************************************/

void ui_backend_delete(void* sdl_screen)
{
  if (sdl_screen != NULL)
    {
      SDL_Quit();
    }
}

/**************************************************/
/**************************************************/
/**************************************************/
 
int ui_backend_framebuffer_blit(void *sdl_ptr, uint8_t *fb)
{
  int w, h, idx_pixl, idx_buff;
  Uint8 *buffer;
  SDL_Surface *sdl_screen;

  sdl_screen = (SDL_Surface*)sdl_ptr;
  buffer=(Uint8 *)sdl_screen->pixels;

  for(h=0; h < sdl_screen->h; h++)
    {
      idx_pixl =  h * sdl_screen->pitch;
      idx_buff =  h * sdl_screen->w * 3;
      
      for(w=0; w < sdl_screen->w; w++)
	{
	  /* idx_buff = (h * GUI_DATA_BKTRK.width + w) * 3 ;     */
	  /* idx_pixl =  h * GUI_DATA.sdl_screen->pitch + w * 3; */

	  buffer[idx_pixl + 0] = fb[idx_buff + 0];
	  buffer[idx_pixl + 1] = fb[idx_buff + 1];
	  buffer[idx_pixl + 2] = fb[idx_buff + 2];

	  idx_pixl += 3;
	  idx_buff += 3;
	}
    }
  return UI_OK;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_lock(void* sdl_ptr)
{
  SDL_Surface *sdl_screen;
  sdl_screen = (SDL_Surface*)sdl_ptr;
  return SDL_LockSurface(sdl_screen);
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_unlock(void* sdl_ptr)
{
  SDL_Surface *sdl_screen;
  sdl_screen = (SDL_Surface*)sdl_ptr;
  SDL_UnlockSurface(sdl_screen);
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_update(void* sdl_ptr)
{
  SDL_Surface *sdl_screen;
  sdl_screen = (SDL_Surface*)sdl_ptr;
  SDL_UpdateRect(sdl_screen, 0, 0, 0, 0);
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

#if defined(DEBUG_KEYBOARD)
static void print_modifiers(void)
{
	int mod;
	HW_DMSG_UI(" modifiers:");
	mod = SDL_GetModState();
	if(!mod) {
		HW_DMSG_UI(" (none)");
		return;
	}
	if(mod & KMOD_LSHIFT)
		HW_DMSG_UI(" LSHIFT");
	if(mod & KMOD_RSHIFT)
		HW_DMSG_UI(" RSHIFT");
	if(mod & KMOD_LCTRL)
		HW_DMSG_UI(" LCTRL");
	if(mod & KMOD_RCTRL)
		HW_DMSG_UI(" RCTRL");
	if(mod & KMOD_LALT)
		HW_DMSG_UI(" LALT");
	if(mod & KMOD_RALT)
		HW_DMSG_UI(" RALT");
	if(mod & KMOD_LMETA)
		HW_DMSG_UI(" LMETA");
	if(mod & KMOD_RMETA)
		HW_DMSG_UI(" RMETA");
	if(mod & KMOD_NUM)
		HW_DMSG_UI(" NUM");
	if(mod & KMOD_CAPS)
		HW_DMSG_UI(" CAPS");
	if(mod & KMOD_MODE)
		HW_DMSG_UI(" MODE");
}
#endif

/**************************************************/
/**************************************************/
/**************************************************/

#if defined(DEBUG_KEYBOARD)
static void PrintKey(SDL_keysym *sym, int pressed)
{
	/* Print the keycode, name and state */
	if ( sym->sym)
	  {
	    if (pressed)
	      {
		HW_DMSG_UI("Key %s: sym %d keyname %s ", pressed ?  "pressed" : "released",
		       sym->sym, SDL_GetKeyName(sym->sym));
	      }
	  } 
	else 
	  {
	    HW_DMSG_UI("Unknown Key (scancode = %d) %s ", sym->scancode,
		   pressed ?  "pressed" : "released");
	  }

	/* Print the translated character, if one exists */
	if ( sym->unicode ) 
	  {
	    /* Is it a control-character? */
	    if ( sym->unicode < ' ' ) 
	      {
		HW_DMSG_UI(" (^%c)", sym->unicode+'@');
	      } 
	    else 
	      {
#ifdef UNICODE
		HW_DMSG_UI(" (%c)", sym->unicode);
#else
		/* This is a Latin-1 program, so only show 8-bits */
		if ( !(sym->unicode & 0xFF00) )
		  HW_DMSG_UI(" (%c)", sym->unicode);
#endif
	      }
	  }
	print_modifiers();
	HW_DMSG_UI("\n");
}
#endif

/**************************************************/
/**************************************************/
/**************************************************/
/*
static const int butkey [NB_BUTTONS] = {
  SDLK_a,
  SDLK_z,
  SDLK_e,
  SDLK_r,
  SDLK_t,
  SDLK_y,
  SDLK_u,
  SDLK_i
};
*/

static const int butkey [NB_BUTTONS] = {
  SDLK_1,
  SDLK_2,
  SDLK_3,
  SDLK_4,
  SDLK_5,
  SDLK_6,
  SDLK_7,
  SDLK_8
};

static const int butcode[NB_BUTTONS] = { 
  UI_BUTTON_1, 
  UI_BUTTON_2, 
  UI_BUTTON_3, 
  UI_BUTTON_4, 
  UI_BUTTON_5,
  UI_BUTTON_6,
  UI_BUTTON_7,
  UI_BUTTON_8
};

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_getevent(void* sdl_ptr, uint32_t *b_up, uint32_t* b_down)
{
  int i;
  int ret = UI_EVENT_NONE;
  SDL_Event event;
  Uint8 *keys;

  SDL_Surface *sdl_screen;
  sdl_screen = (SDL_Surface*)sdl_ptr;

  if (SDL_PollEvent(&event) == 1)
    {
      switch (event.type) 
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	  
	  keys = SDL_GetKeyState(NULL);
	  *b_up   = 0;
	  *b_down = 0;

	  for(i=0; i < NB_BUTTONS; i++)
	    {
	      if (keys[butkey[i]] == SDL_PRESSED)
		{
		  *b_down |= butcode[i];
		  ret |= UI_EVENT_USER;
		}
	      else if (keys[butkey[i]] == SDL_RELEASED)
		{
		  *b_up   |= butcode[i];
		  ret |= UI_EVENT_USER;
		}
	    }

	  if (keys[SDLK_q] == SDL_PRESSED)
	    {
	      ret = UI_EVENT_QUIT;
	    }
	  break;

	case SDL_MOUSEBUTTONDOWN:
	  break;

	case SDL_QUIT:
	  HW_DMSG_UI("==============\n");
	  HW_DMSG_UI("== UI  quit ==\n");
	  HW_DMSG_UI("==============\n");
	  ret = UI_EVENT_QUIT;
	  break;

	default:
	  break;
	}
    }
  else
    {
      // there was no event, we keep buttons as they were
      // not done at this time
    }
  return ret;
}

/**************************************************/
/**************************************************/
/**************************************************/
