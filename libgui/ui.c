
/**
 *  \file   ui.c
 *  \brief  WorldSens Graphical UI definition 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "src/mgetopt.h"
#include "src/options.h"
#include "ui.h"

/**************************************************/
/**************************************************/
/**************************************************/

#if !defined(GUI)

int dummy_function(void)
{
  return 0;
}

#else

/**************************************************/
/**************************************************/
/**************************************************/

#include "SDL.h"

#undef DEBUG

#if defined(DEBUG)
#define DEBUG_KEYBOARD
#define DMSG_UI(x...) HW_DMSG_UI(x)
#else
#define DMSG_UI(x...) do { } while (0)
#endif

/**************************************************/
/**************************************************/
/**************************************************/

/**
 * global variables
 **/

struct ui_internal_t {
  SDL_Surface * sdl_screen;
  int           mustlock;
  int           display_on;
};

static struct ui_internal_t ui;

#define GUI_DATA_BKTRK machine.ui
#define GUI_DATA       ui

static struct moption_t gui_opt = {
  .longname    = "ui",
  .type        = no_argument,
  .helpstring  = "enable GUI",
  .value       = NULL
};

#define NB_BUTTONS 8
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

int ui_options_add(void)
{
  options_add(& gui_opt );
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

static int ui_option_validate(void)
{
  GUI_DATA.display_on = gui_opt.isset;
  return UI_OK;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_create(int w, int h, int id)
{
  int desired_bpp;
  Uint32 video_flags;

  GUI_DATA.sdl_screen  = NULL;
  GUI_DATA.mustlock    = 0;
  GUI_DATA.display_on  = 0;

  ui_option_validate();
  if (GUI_DATA.display_on == 0)
    {
      return UI_OK;
    }

  /* init SDL */
  desired_bpp = 8 * GUI_DATA_BKTRK.bpp;
  video_flags = 0;

  /* Enable UNICODE translation for keyboard input */
  /* SDL_EnableUNICODE(1); */

  /* Enable auto repeat for keyboard input */
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
    {
      ERROR("Couldn't initialize SDL: %s\n", SDL_GetError());
      return UI_ERROR;
    }

  atexit(SDL_Quit);			/* Clean up on exit */
  GUI_DATA.sdl_screen = SDL_SetVideoMode(w, h, desired_bpp, video_flags);

  if ( GUI_DATA.sdl_screen == NULL ) 
    {
      ERROR("Couldn't set %dx%dx%d video mode: %s\n",
	      w, h, desired_bpp, SDL_GetError());
      return UI_ERROR;
    }

  if (id != -1)
    {
      char name[20];
      sprintf(name,"WSim %d",id);
      SDL_WM_SetCaption(name,name);
    }
  else
    {
      SDL_WM_SetCaption("WSim","WSim");
    }

  HW_DMSG_UI("gui:Pixel format 0x%04x\n",
	     (unsigned int)SDL_MapRGB(GUI_DATA.sdl_screen->format, 250,  250,  250));
  HW_DMSG_UI("gui:Display %dx%dx%d \n",
	     GUI_DATA.sdl_screen->w, 
	     GUI_DATA.sdl_screen->h,
	     GUI_DATA.sdl_screen->format->BitsPerPixel);
  HW_DMSG_UI("gui:   %d bytes per pixel, pitch %d\n", 
	     GUI_DATA.sdl_screen->format->BytesPerPixel, 
	     GUI_DATA.sdl_screen->pitch);
  HW_DMSG_UI("gui:   mask R 0x%08x G 0x%08x B 0x%08x A 0x%08x\n",
	     GUI_DATA.sdl_screen->format->Rmask,
	     GUI_DATA.sdl_screen->format->Gmask,
	     GUI_DATA.sdl_screen->format->Bmask,
	     GUI_DATA.sdl_screen->format->Amask);
  HW_DMSG_UI("gui:   colorkey 0x%08x\n", GUI_DATA.sdl_screen->format->colorkey);

  if (SDL_MUSTLOCK(GUI_DATA.sdl_screen) == 0)
    {
      GUI_DATA.mustlock = 0;
    }
  else
    {
      GUI_DATA.mustlock = 1;
    }

  if (GUI_DATA_BKTRK.bpp != GUI_DATA.sdl_screen->format->BytesPerPixel)
    {
      ERROR("  Incorrect bytes per pixel format : core dump expected\n");
      return UI_ERROR;
    }

  /* backtracked data */
  GUI_DATA_BKTRK.b_up   = 0;
  GUI_DATA_BKTRK.b_down = 0;

  return UI_OK;
}


/**************************************************/
/**************************************************/
/**************************************************/

void ui_delete()
{
  if (GUI_DATA.sdl_screen != NULL)
    {
      SDL_Quit();
    }
}

/**************************************************/
/**************************************************/
/**************************************************/
 
int ui_refresh()
{
  int w, h, idx_pixl, idx_buff;
  
  Uint8 *buffer;
  Uint8 *fb;

  if (GUI_DATA.display_on == 0)
    return UI_OK;

  if (GUI_DATA.sdl_screen == NULL)
    return UI_ERROR;

  if (GUI_DATA.mustlock &&  SDL_LockSurface(GUI_DATA.sdl_screen) < 0 ) 
    {
      HW_DMSG_UI( "Couldn't lock the display surface: %s\n",SDL_GetError());
      return UI_ERROR;
    }

  //  HW_DMSG("SDL_surface : %dx%d pitch %d\n",sdl_screen->w,sdl_screen->h,sdl_screen->pitch);
  //  HW_DMSG("framebuffer : %dx%d pitch %d\n",GUI_DATA_BKTRK.width,GUI_DATA_BKTRK.height,GUI_DATA_BKTRK.width*GUI_DATA_BKTRK.bpp);

  buffer=(Uint8 *)GUI_DATA.sdl_screen->pixels;

  fb = GUI_DATA_BKTRK.framebuffer;

  /* memcpy(buffer,GUI_DATA_BKTRK.framebuffer,GUI_DATA_BKTRK.framebuffer_size); */

  for(h=0; h < GUI_DATA_BKTRK.height; h++)
    {
      idx_pixl =  h * GUI_DATA.sdl_screen->pitch;
      idx_buff =  h * GUI_DATA_BKTRK.width * 3;
      
      for(w=0; w < GUI_DATA_BKTRK.width; w++)
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


  if (GUI_DATA.mustlock)
    {
      SDL_UnlockSurface(GUI_DATA.sdl_screen);
    }

  SDL_UpdateRect(GUI_DATA.sdl_screen, 0, 0, 0, 0);

  return UI_OK;
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

int ui_getevent(void)
{
  int i;
  int ret = UI_EVENT_NONE;
  SDL_Event event;
  Uint8 *keys;

  if (GUI_DATA.display_on == 0)
    return UI_EVENT_NONE;

  if (SDL_PollEvent(&event) == 1)
    {
      switch (event.type) 
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	  
	  keys = SDL_GetKeyState(NULL);
	  GUI_DATA_BKTRK.b_up   = 0;
	  GUI_DATA_BKTRK.b_down = 0;

	  for(i=0; i < NB_BUTTONS; i++)
	    {
	      if (keys[butkey[i]] == SDL_PRESSED)
		{
		  GUI_DATA_BKTRK.b_down |= butcode[i];
		  ret |= UI_EVENT_USER;
		}
	      else if (keys[butkey[i]] == SDL_RELEASED)
		{
		  GUI_DATA_BKTRK.b_up   |= butcode[i];
		  ret |= UI_EVENT_USER;
		}
	    }

	  if (keys[SDLK_q] == SDL_PRESSED)
	    {
	      ret = UI_EVENT_QUIT;
	    }
	  break;

	case SDL_MOUSEBUTTONDOWN:
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

#endif /* GUI */
