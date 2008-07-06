
/**
 *  \file    ui.h
 *  \brief   Terminal for wsim serial line emulation 
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#ifndef UI_H
#define UI_H

#include "ui_screen.h"

#define UI_ERROR   0x100u
#define UI_QUIT    0x200u
#define UI_NOEVENT 0x300u

#define UI_MODE_NONE    0
#define UI_MODE_OUTPUT  1
#define UI_MODE_INPUT   3
#define UI_MODE_IO      2

#define KBD_BLOCKING    1
#define KBD_NONBLOCKING 2

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"

struct ui_t {
  /* screens */
  int nscreens;
  int screen_width;
  int screen_height;
  screen_t *screen;

  SDL_Surface *sdl_screen;
  SDL_mutex   *lock;

  /* SDL ui stuff */
  int window_mustlock;
  int window_width;
  int window_height;
  int window_bpp;
};


struct ui_t *ui_create    (char* name, int w, int h, int mode);
void         ui_delete    (struct ui_t*);
void         ui_setname   (char* name, char* icon);
int          ui_kbd_event (struct ui_t *ui, int mode);
void         ui_putchar   (struct ui_t *ui, int n, char c);
void         ui_post_quit (struct ui_t *ui);
#endif
