/**
 *  \file   libselect_file.h
 *  \brief  libselect file API
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#ifndef __LIBSELECT_FILE_H_
#define __LIBSELECT_FILE_H_

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#include "src/options.h" /* defines MAX_FILENAME */
int libselect_get_system_fifo(char local_name[MAX_FILENAME], char remote_name[MAX_FILENAME]);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
