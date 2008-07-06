
/**
 *  \file    ui_screen.h
 *  \brief   Terminal for wsim serial line emulation 
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#ifndef UI_SCREEN_H
#define UI_SCREEN_H

typedef struct s_screen_t* screen_t;

screen_t       screen_create     (int n,int w, int h);
void           screen_delete     (screen_t s);

void           screen_write      (screen_t s, char c);
void           screen_clear      (screen_t s);

int            screen_get_height (screen_t s);
int            screen_get_width  (screen_t s);
unsigned char  screen_get_char   (screen_t s, int x, int y);

#endif
