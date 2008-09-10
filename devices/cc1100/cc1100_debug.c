
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

char* cc1100_status_to_str(int status)
{
  switch (status)
    {
    case CC1100_STATUS_IDLE             : return "CC1100_STATUS_IDLE";             // 0x00
    case CC1100_STATUS_RX               : return "CC1100_STATUS_RX";               // 0x01
    case CC1100_STATUS_TX               : return "CC1100_STATUS_TX";               // 0x02
    case CC1100_STATUS_FSTXON           : return "CC1100_STATUS_FSTXON";           // 0x03
    case CC1100_STATUS_CALIBRATE        : return "CC1100_STATUS_CALIBRATE";        // 0x04
    case CC1100_STATUS_SETTLING         : return "CC1100_STATUS_SETTLING";         // 0x05
    case CC1100_STATUS_RXFIFO_OVERFLOW  : return "CC1100_STATUS_RXFIFO_OVERFLOW";  // 0x06
    case CC1100_STATUS_TXFIFO_UNDERFLOW : return "CC1100_STATUS_TXFIFO_UNDERFLOW"; // 0x07
    }
  return "";
}

/***************************************************/
/***************************************************/
/***************************************************/

char* cc1100_state_to_str(int state)
{
  switch (state)
    {
    case CC1100_STATE_SLEEP         : return "CC1100_STATE_SLEEP";         // 0x00
    case CC1100_STATE_IDLE          : return "CC1100_STATE_IDLE";          // 0x01
    case CC1100_STATE_XOFF          : return "CC1100_STATE_XOFF";          // 0x02
    case CC1100_STATE_MANCAL        : return "CC1100_STATE_MANCAL";        // 0x03
    case CC1100_STATE_FS_WAKEUP     : return "CC1100_STATE_FS_WAKEUP";     // 0x06
    case CC1100_STATE_FS_CALIBRATE  : return "CC1100_STATE_FS_CALIBRATE";  // 0x08
    case CC1100_STATE_SETTLING      : return "CC1100_STATE_SETTLING";      // 0x09
    case CC1100_STATE_CALIBRATE     : return "CC1100_STATE_CALIBRATE";     // 0x12
    case CC1100_STATE_RX            : return "CC1100_STATE_RX";            // 0x13
    case CC1100_STATE_TXRX_SETTLING : return "CC1100_STATE_TXRX_SETTLING"; // 0x16
    case CC1100_STATE_RX_OVERFLOW   : return "CC1100_STATE_RX_OVERFLOW";   // 0x17
    case CC1100_STATE_FSTXON        : return "CC1100_STATE_FSTXON";        // 0x18
    case CC1100_STATE_TX            : return "CC1100_STATE_TX";            // 0x19
    case CC1100_STATE_RXTX_SETTLING : return "CC1100_STATE_RXTX_SETTLING"; // 0x21
    case CC1100_STATE_TX_UNDERFLOW  : return "CC1100_STATE_TX_UNDERFLOW";  // 0x22
    case CC1100_STATE_IDLING        : return "CC1100_STATE_IDLING";        // 0x23
    }
  return "";
}

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
    case CC1100_STROBE_SAFC    : return "SAFC";    // 0x37 
    case CC1100_STROBE_SWOR    : return "SWOR";    // 0x38 
    case CC1100_STROBE_SPWD    : return "SPWD";    // 0x39
    case CC1100_STROBE_SFRX    : return "SFRX";    // 0x3A
    case CC1100_STROBE_SFTX    : return "SFTX";    // 0x3B
    case CC1100_STROBE_SNOP    : return "SNOP";    // 0x3D
    }
  return "unknown";
}

/***************************************************/
/***************************************************/
/***************************************************/
