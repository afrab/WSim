
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

/**
 * global variables
 **/
#define EVENT_FIFO_SIZE    0xff
#define LOOP_SKIP_COUNTER 50000

struct ui_internal_t {
  void *        backend;
  int           mustlock;
  int           display_on;

  int           e_fifo[EVENT_FIFO_SIZE];
  int           e_rptr;
  int           e_wptr;
  int           e_state;
};

static struct ui_internal_t ui;

#define GUI_DATA_INTERNAL  ui
#define GUI_DATA_MACHINE   machine.ui

/**************************************************/
/**************************************************/
/**************************************************/

static struct moption_t gui_opt = {
  .longname    = "ui",
  .type        = no_argument,
  .helpstring  = "enable GUI",
  .value       = NULL
};

static struct moption_t title_opt = {
  .longname    = "ui_title",
  .type        = required_argument,
  .helpstring  = "GUI window title",
  .value       = NULL
};

/**************************************************/
/**************************************************/
/**************************************************/

int ui_options_add(void)
{
  options_add(& gui_opt            );
  options_add(& title_opt          );
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

static int ui_option_validate(void)
{
  GUI_DATA_INTERNAL.display_on = gui_opt.isset;
  return UI_OK;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_create(int w, int h, int id)
{
  GUI_DATA_INTERNAL.backend     = NULL;
  GUI_DATA_INTERNAL.mustlock    = 0;
  GUI_DATA_INTERNAL.display_on  = 0;

  ui_option_validate();

  if (GUI_DATA_INTERNAL.display_on == 0)
    {
      return UI_OK;
    }

#define MAX_TITLE_SIZE 20
  char name[MAX_TITLE_SIZE];

  if (id != -1)
    {
      if (title_opt.isset && (title_opt.value != NULL))
	{
	  strncpy(name, title_opt.value, MAX_TITLE_SIZE);
	}
      else
	{
	  snprintf(name,MAX_TITLE_SIZE,"WSim %d",id);
	}
    }
  else
    {
      strncpy(name, "WSim", MAX_TITLE_SIZE);
    }

  GUI_DATA_INTERNAL.backend = 
    ui_backend_create(w, h, name, & GUI_DATA_INTERNAL.mustlock);

  GUI_DATA_MACHINE.b_up   = 0;
  GUI_DATA_MACHINE.b_down = 0;

  if (GUI_DATA_INTERNAL.backend == NULL)
    {
      return UI_ERROR;
    }

  GUI_DATA_INTERNAL.e_rptr  = 0;
  GUI_DATA_INTERNAL.e_wptr  = 0;
  GUI_DATA_INTERNAL.e_state = 0;

  return UI_OK;
}


/**************************************************/
/**************************************************/
/**************************************************/

void ui_delete(void)
{
  if (GUI_DATA_INTERNAL.backend != NULL)
    {
      ui_backend_delete(GUI_DATA_INTERNAL.backend);
    }
}

/**************************************************/
/**************************************************/
/**************************************************/
 
int ui_refresh(int modified)
{
  uint8_t *fb;
  static int loop = 0;

  if (GUI_DATA_INTERNAL.display_on == 0)
    {
      return UI_OK;
    }

  if (modified)
    {
      if (GUI_DATA_INTERNAL.mustlock &&  ui_backend_lock(GUI_DATA_INTERNAL.backend) < 0)
	{
	  return UI_ERROR;
	}
      
      fb = GUI_DATA_MACHINE.framebuffer;
      ui_backend_framebuffer_blit(GUI_DATA_INTERNAL.backend, fb);

      if (GUI_DATA_INTERNAL.mustlock)
	{
	  ui_backend_unlock(GUI_DATA_INTERNAL.backend);
	}

      ui_backend_update(GUI_DATA_INTERNAL.backend);
    }

  if (loop == LOOP_SKIP_COUNTER)
    {
      ui_event_process();
      loop = 0;
    }
  loop ++;

  return UI_OK;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_event_process(void)
{
  int evt;
  evt = ui_backend_getevent(GUI_DATA_INTERNAL.backend,
			    &GUI_DATA_MACHINE.b_up,
			    &GUI_DATA_MACHINE.b_down);

  if (evt != UI_EVENT_NONE)
    {
      if (GUI_DATA_INTERNAL.e_wptr != EVENT_FIFO_SIZE)
	{
	  GUI_DATA_INTERNAL.e_fifo[ GUI_DATA_INTERNAL.e_wptr ] = evt;
	  GUI_DATA_INTERNAL.e_state =  GUI_DATA_INTERNAL.e_state + 1;
	  GUI_DATA_INTERNAL.e_wptr  = (GUI_DATA_INTERNAL.e_wptr  + 1) % EVENT_FIFO_SIZE;
	}
    }

  return UI_OK;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_getevent(void)
{
  int ret = UI_EVENT_NONE;

  if (GUI_DATA_INTERNAL.display_on == 0)
    {
      return UI_EVENT_NONE;
    }

  if (GUI_DATA_INTERNAL.e_state)
    {
      ret = GUI_DATA_INTERNAL.e_fifo[ GUI_DATA_INTERNAL.e_rptr ];
      GUI_DATA_INTERNAL.e_state =  GUI_DATA_INTERNAL.e_state - 1;
      GUI_DATA_INTERNAL.e_rptr  = (GUI_DATA_INTERNAL.e_rptr  + 1) % EVENT_FIFO_SIZE;
    }

  return ret;
}

/**************************************************/
/**************************************************/
/**************************************************/

void ui_default_input (char *name)
{
  int ev;
  switch ((ev = ui_getevent()))
    {
    case UI_EVENT_USER:
      HW_DMSG_UI("%s: UI event %04x %04x\n", name, machine.ui.b_up,machine.ui.b_down);
      break;

    case UI_EVENT_QUIT:
      HW_DMSG_UI("%s: UI event QUIT\n",name);
      mcu_signal_add(SIG_HOST | SIGTERM);
      break;

    case UI_EVENT_NONE:
      break;
	
    default:
      ERROR("%s: unknown ui event\n",name);
      break;
    }
}

/**************************************************/
/**************************************************/
/**************************************************/

