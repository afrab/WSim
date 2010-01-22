
/**
 *  \file   X11_bkend.c
 *  \brief  WorldSens Graphical X11 UI definition 
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "src/mgetopt.h"
#include "src/options.h"

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

#define UNUSED __attribute__((unused))  

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

struct x11_display_t {
  Display   *display;
  Window     window;
  GC         gc;
  Colormap   colormap;
  Visual    *visual;
  XImage    *ximage;
  char      *data;
  int        width,height,depth;
  Atom wmDeleteMessage;
};

static struct x11_display_t x11_display;

/**************************************************/
/**************************************************/
/**************************************************/

void* ui_backend_create(int w, int h, char *title, int *mustlock)
{
  XSizeHints shint;
  struct x11_display_t *x11 = (struct x11_display_t*) & x11_display;

  *mustlock = 0;

  x11->display = NULL;
  x11->window  = 0;
  x11->ximage  = NULL;
  x11->data    = NULL;

  /* Open display */
  if ((x11->display = XOpenDisplay(NULL)) == NULL) 
    {
      ERROR("x11: cannot open X11 display\n");
      return NULL;
    }

  /* PPosition */
  shint.x = 300;
  shint.y = 0;
  shint.min_width  = shint.max_width  = shint.width  = w;
  shint.min_height = shint.max_height = shint.height = h;
   
  shint.flags = PSize | PMinSize | PMaxSize;

  /* Simple window */
  int screenNumber = DefaultScreen(x11->display);

  x11->visual = DefaultVisual(x11->display, screenNumber);
  if(x11->visual->class!=TrueColor)
    {
      ERROR("x11: Cannot handle non true color visual ...\n");
      ui_backend_delete(x11);
      return NULL;
    }

  x11->colormap = DefaultColormap(x11->display, screenNumber);
  unsigned long white = WhitePixel(x11->display,screenNumber);
  unsigned long black = BlackPixel(x11->display,screenNumber);

  x11->window = XCreateSimpleWindow(
		     x11->display,
                     DefaultRootWindow(x11->display), 
                     shint.x, shint.y, 
		     shint.width, shint.height, 
		     5, white, // border
		     black );  // backgd

  if (x11->window == 0) 
    {
      ERROR("x11: Can't create window.\n");
      ui_backend_delete(x11);
      return NULL;
    }
  
  /* title */
  XSetStandardProperties(x11->display, x11->window,
			 title, "", None, NULL, 0, &shint);

  /* GC */
  x11->gc = XCreateGC( x11->display, x11->window,
		       0,        // mask of values
		       NULL );   // array of values

  /* Map window */
  XMapWindow( x11->display, x11->window );

  /* input type : wait for Notify */
  long eventMask = StructureNotifyMask;
  XSelectInput( x11->display, x11->window, eventMask );

  XEvent evt;
  do{
    XNextEvent( x11->display, &evt );   // calls XFlush
  }while( evt.type != MapNotify );
  
  /* XImage */
  x11->width  = w;
  x11->height = h;
  x11->depth  = 24;

  x11->data = (char *)malloc(x11->width * x11->height * 4);
  x11->ximage = XCreateImage(x11->display, x11->visual, x11->depth, ZPixmap, 
			     0, x11->data, x11->width, x11->height, 32, 0);
  XInitImage(x11->ximage);

  /* WM delete message */
  x11->wmDeleteMessage = XInternAtom(x11->display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(x11->display, x11->window, &x11->wmDeleteMessage, 1);


  eventMask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
  XSelectInput(x11->display, x11->window, eventMask); 
  XMapWindow(x11->display, x11->window);
    

  return &x11_display;
}

/**************************************************/
/**************************************************/
/**************************************************/

void ui_backend_delete(void *ptr)
{
  struct x11_display_t *x11 = (struct x11_display_t*)ptr;
  if (x11 && x11->display)
    {
      if (x11->ximage)
	{
	  XDestroyImage(x11->ximage);
	  x11->ximage = NULL;
	}
      if (x11->window)
	{
	  XDestroyWindow(x11->display, x11->window);
	  x11->window = 0;
	}
      if (x11->display)
	{
	  XCloseDisplay(x11->display);
	  x11->display = NULL;
	}
    }
}

/**************************************************/
/**************************************************/
/**************************************************/
 
int ui_backend_framebuffer_blit(void *ptr, uint8_t *fb)
{
  int w,h;
  int idx_buff;
  struct x11_display_t *x11 = (struct x11_display_t*)ptr;
  
  for(h=0; h < x11->height; h++)
    {
      idx_buff =  h * x11->width * 3;
      
      for(w=0; w < x11->width; w++)
	{
	  unsigned int color = 
	    (fb[idx_buff + 2] << 16) | 
	    (fb[idx_buff + 1] <<  8) | 
	    (fb[idx_buff + 0] <<  0);

	  idx_buff += 3;

	  XPutPixel(x11->ximage, w, h, color);
	}
    }
  return UI_OK;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_lock(void UNUSED *ptr)
{
  // struct x11_display_t *x11 = (struct x11_display_t*)ptr; 
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_unlock(void UNUSED *ptr)
{
  // struct x11_display_t *x11 = (struct x11_display_t*)ptr;
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_update(void *ptr)
{
  struct x11_display_t *x11 = (struct x11_display_t*)ptr;
  XPutImage(x11->display, x11->window, x11->gc, x11->ximage, 
	    0, 0,                     // src
	    0, 0,                     // dst
	    x11->width, x11->height); // size
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_getevent(void *ptr, uint32_t *b_up, uint32_t* b_down)
{
  int ret = UI_EVENT_NONE;
  KeySym key;
  XEvent event;
  char text[255];
  int n;
  struct x11_display_t *x11 = (struct x11_display_t*)ptr;

  n = XEventsQueued (x11->display, QueuedAfterFlush);
 
  if (n == 0)
    {
      return ret;
    }

  XNextEvent(x11->display, &event);

  switch (event.type)
    {
    case KeyPress:
      if (XLookupString(&event.xkey,text,255,&key,0)==1) 
	{
	  ret |= UI_EVENT_USER;
	  *b_up   = 0;
	  *b_down = 0;

	  switch (text[0])
	    {
	    case '1': *b_down |= UI_BUTTON_1; break;
	    case '2': *b_down |= UI_BUTTON_2; break;
	    case '3': *b_down |= UI_BUTTON_3; break;
	    case '4': *b_down |= UI_BUTTON_4; break;
	    case '5': *b_down |= UI_BUTTON_5; break;
	    case '6': *b_down |= UI_BUTTON_6; break;
	    case '7': *b_down |= UI_BUTTON_7; break;
	    case '8': *b_down |= UI_BUTTON_8; break;
	    case 'q': ret = UI_EVENT_QUIT;    break;
	    default:
	      ret = UI_EVENT_NONE;
	      break;
	    }
	}
      break;

    case KeyRelease:
      if (XLookupString(&event.xkey,text,255,&key,0)==1) 
	{
	  ret |= UI_EVENT_USER;
	  *b_up   = 0;
	  *b_down = 0;

	  switch (text[0])
	    {
	    case '1': *b_up |= UI_BUTTON_1; break;
	    case '2': *b_up |= UI_BUTTON_2; break;
	    case '3': *b_up |= UI_BUTTON_3; break;
	    case '4': *b_up |= UI_BUTTON_4; break;
	    case '5': *b_up |= UI_BUTTON_5; break;
	    case '6': *b_up |= UI_BUTTON_6; break;
	    case '7': *b_up |= UI_BUTTON_7; break;
	    case '8': *b_up |= UI_BUTTON_8; break;
	    case 'q': ret = UI_EVENT_QUIT;  break;
	    default:
	      ret = UI_EVENT_NONE;
	      break;
	    }
	}
      break;

    case ClientMessage:
      if (event.xclient.data.l[0] == (unsigned)x11->wmDeleteMessage) 
	{
	  mcu_signal_add(SIG_HOST | SIGTERM);
	  ret = UI_EVENT_QUIT;
	}
      break;
	    
    case DestroyNotify:
      mcu_signal_add(SIG_HOST | SIGTERM);
      ret = UI_EVENT_QUIT;
      break;

    default:
      // printf("message type %d %x\n",event.type,event.type);
      break;
    }

  return ret;
}

/**************************************************/
/**************************************************/
/**************************************************/
 
 
