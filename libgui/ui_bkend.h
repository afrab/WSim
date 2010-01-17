
/**
 *  \file   ui_bkend.h
 *  \brief  WorldSens Graphical UI definition 
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#ifndef HW_GUI_BACKEND_H
#define HW_GUI_BACKEND_H

void* ui_backend_create           (int w, int h, char* title, int* mustlock);
void  ui_backend_delete           (void*);

int   ui_backend_lock             (void*);
int   ui_backend_unlock           (void*);

int   ui_backend_framebuffer_blit (void*, uint8_t* fb);
int   ui_backend_update           (void*);

int   ui_backend_getevent         (void*, uint32_t *b_up, uint32_t* b_down);
#endif

