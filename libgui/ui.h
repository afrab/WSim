
/**
 *  \file   ui.h
 *  \brief  WorldSens Simulator UI Definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef HW_GUI_H
#define HW_GUI_H

/*
 *
 */

#define UI_OK                     0
#define UI_ERROR                  1


/* UI
 *
 *                      | .... .... | .... .... | .... .... | .... .... |
 *
 *   system event                                             xxxx xxxx
 *   user event                                   xxxx xxxx   
 *
 *
 *   0 must be a value OK / No Event
 */

#define UI_EVENT_NONE             0x00000000u
#define UI_EVENT_QUIT             0x00000001u
#define UI_EVENT_UNKNOWN          0x00000002u
#define UI_EVENT_USER             0x00000100u



/*
 * User button codes
 *
 */
#define NB_BUTTONS 8

#define UI_BUTTON_1               (1 << 0)
#define UI_BUTTON_2               (1 << 1)
#define UI_BUTTON_3               (1 << 2)
#define UI_BUTTON_4               (1 << 3)
#define UI_BUTTON_5               (1 << 4)
#define UI_BUTTON_6               (1 << 5)
#define UI_BUTTON_7               (1 << 6)
#define UI_BUTTON_8               (1 << 7)

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

#if defined(GUI)

int  ui_options_add   (void);
int  ui_create        (int w, int h, int id);
void ui_delete        (void);
int  ui_refresh       (int modified);
int  ui_event_process (void);
int  ui_getevent      (void);
void ui_default_input (char* name);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#else

/* we define static inline dummy functions */
#define UNUSED __attribute__((unused))

static inline int  ui_options_add (void) { return 0;             }
static inline int  ui_create      (int UNUSED w, int UNUSED h, int UNUSED id) { return UI_OK; }
static inline void ui_delete      (void) { return ;              }
static inline int  ui_refresh     (int UNUSED r) { return UI_OK;         }
static inline int  ui_event_process (void) { return UI_OK; }
static inline int  ui_getevent    (void) { return UI_EVENT_NONE; }

#if !defined(WSNET1)
static inline void ui_default_input (char UNUSED *s);
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
#endif
