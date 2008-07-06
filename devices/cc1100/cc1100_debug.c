
/**
 *  \file   cc1100_debug.c
 *  \brief  CC1100 debug messages
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <string.h>

#include "liblogger/logger.h"
#include "cc1100_globals.h"
#include "cc1100_debug.h"

/***************************************************/
/***************************************************/
/***************************************************/

char* cc1100_strobe_to_str(int strb)
{
  switch (strb)
    {
    case CC1100_STROBE_SRES    : return "SRES";    // 0x30
    case CC1100_STROBE_SFSTXON : return "SFSTXON"; // 0x31
    case CC1100_STROBE_SXOFF   : return "SXOFF";   // 0x32
    case CC1100_STROBE_SCAL    : return "SCAL";    // 0x33
    case CC1100_STROBE_SRX     : return "SRX";     // 0x34
    case CC1100_STROBE_STX     : return "STX";     // 0x35
    case CC1100_STROBE_SIDLE   : return "SIDLE";   // 0x36
    case CC1100_STROBE_SPWD    : return "SPWD";    // 0x39
    case CC1100_STROBE_SFRX    : return "SFRX";    // 0x3A
    case CC1100_STROBE_SFTX    : return "SFTX";    // 0x3B
    case CC1100_STROBE_SNOP    : return "SNOP";    // 0x3D
    }
  return "";
}

/***************************************************/
/***************************************************/
/***************************************************/
