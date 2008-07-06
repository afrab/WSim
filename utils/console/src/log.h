
/**
 *  \file    log.h
 *  \brief   Terminal log
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#ifndef _LOG_H
#define _LOG_H

FILE* log_open    (char* filename);
void  log_close   (FILE* log);
void  log_putchar (FILE* log, uint8_t v);
void  log_print   (FILE* log, char *s);

#endif
