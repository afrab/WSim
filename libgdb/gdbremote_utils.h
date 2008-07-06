/**
 *  \file   gdbremote_utils.h
 *  \brief  GDB Remote functions, common functions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef __GDBREMOTE_UTILS_H_
#define __GDBREMOTE_UTILS_H_

extern const char gdbremote_hexchars[];

int gdbremote_hexchar2int (unsigned char ch);
int gdbremote_hex2int     (char **ptr, int *intValue);

#endif
