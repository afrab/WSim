/**
 *  \file   gdbremote_utils.c
 *  \brief  GDB Remote functions, common functions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include "gdbremote_utils.h"

const char gdbremote_hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */

int 
gdbremote_hexchar2int(unsigned char ch)
{
  if (ch >= '0' && ch <= '9')
    return ch-'0';
  if (ch >= 'a' && ch <= 'f')
    return ch-'a'+10;
  if (ch >= 'A' && ch <= 'F')
    return ch-'A'+10;
  return -1;
}


int
gdbremote_hex2int(char **ptr, int *intValue)
{
  int numChars = 0;
  int hexValue;

  *intValue = 0;

  while (**ptr)
    {
      hexValue = gdbremote_hexchar2int(**ptr);
      if (hexValue < 0)
	break;

      *intValue = (*intValue << 4) | hexValue;
      numChars ++;

      (*ptr)++;
    }

  return (numChars);
}


