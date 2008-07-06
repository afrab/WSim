/**
 *  \file   libgdb.h
 *  \brief  GDB mode main loop
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef __LIBGDB_H_
#define __LIBGDB_H_

#define GDB_OK          0
#define GDB_INIT_ERROR  1
#define GDB_CLOSE_ERROR 2

/**
 * GDBRemote main mode 
 * @return 0 ok, otherwise error
 */
int libgdb_target_mode_main(unsigned short port);

#endif
