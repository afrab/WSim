
/**
 *  \file   ui.h
 *  \brief  WorldSens Simulator UI Definition
 *  \author Antoine Fraboulet
 *  \author David Gräff
 *  \date   2005,2013
 **/

#ifndef HW_GUI_H
#define HW_GUI_H

#include <inttypes.h>
#include "gui_defines.h"
struct ui_t
{
  int width;
  int height;
  int bpp;

  uint8_t  *framebuffer;
  uint32_t  framebuffer_size;
  uint32_t  framebuffer_background;

  /* buttons state */
  uint32_t b_up;     
  uint32_t b_down;
  uint32_t b_down_previous;
  /* button names -> only named buttons are shown in the UI! */
  struct button_t {
    char name[255];
    uint8_t pin;
  } buttons[8];
};


#if defined(WORDS_BIGENDIAN)
#define setpixel(pixel,r,v,b)               \
  do {                                      \
  machine.ui.framebuffer[pixel+0]  = r;     \
  machine.ui.framebuffer[pixel+1]  = v;     \
  machine.ui.framebuffer[pixel+2]  = b;     \
  } while (0)      
#else
#define setpixel(pixel,r,v,b)               \
  do {                                      \
  machine.ui.framebuffer[pixel+2]  = r;     \
  machine.ui.framebuffer[pixel+1]  = v;     \
  machine.ui.framebuffer[pixel+0]  = b;     \
  } while (0)      
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

  #ifdef __cplusplus
  extern "C" {
  #endif

	  struct ui_t* get_machine_ui();

	#if defined(GUI)

		int  ui_options_add   (void);
		int  ui_create        (int w, int h, int id);
		void ui_delete        (void);
		int  ui_refresh       (int modified);
		int  ui_getevent      (void);
		void ui_default_input (const char* name);

	#else

		/* we define static inline dummy functions */
		#define UNUSED __attribute__((unused))

		static inline int  ui_options_add (void) { return 0;             }
		static inline int  ui_create      (int UNUSED w, int UNUSED h, int UNUSED id) { return UI_OK; }
		static inline void ui_delete      (void) { return ;              }
		static inline int  ui_refresh     (int UNUSED r) { return UI_OK;         }
		static inline int  ui_getevent    (void) { return UI_EVENT_NONE; }

		#if !defined(WSNET1)
		static inline void ui_default_input (char UNUSED *s);
		#endif

		/* ************************************************** */
		/* ************************************************** */
		/* ************************************************** */

	#endif

	#ifdef __cplusplus
	}
	#endif

#endif
