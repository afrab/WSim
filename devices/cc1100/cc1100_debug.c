
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
    case CC1100_STROBE_SWORRST : return "SWORRST"; // 0x3C
    case CC1100_STROBE_SNOP    : return "SNOP";    // 0x3D
    }
  return "unknown";
}

/***************************************************/
/***************************************************/
/***************************************************/

char* cc1100_register_to_str (int regs)
{
  switch (regs)
  {
  case CC1100_REG_IOCFG2   : return "IOCFG2";                       // 0x00
  case CC1100_REG_IOCFG1   : return "IOCFG1";                       // 0x01
  case CC1100_REG_IOCFG0   : return "IOCFG0";                       // 0x02
  case CC1100_REG_FIFOTHR  : return "FIFOTHR";                      // 0x03
  case CC1100_REG_SYNC1    : return "SYNC1";                        // 0x04
  case CC1100_REG_SYNC0    : return "SYNC0";                        // 0x05
  case CC1100_REG_PKTLEN   : return "PKTLEN";                       // 0x06
  case CC1100_REG_PKTCTRL1 : return "PKTCTRL1";                     // 0x07
  case CC1100_REG_PKTCTRL0 : return "PKTCTRL0";                     // 0x08
  case CC1100_REG_ADDR     : return "ADDR";                         // 0x09
  case CC1100_REG_CHANNR   : return "CHANNR";                       // 0x0A
  case CC1100_REG_FSCTRL1  : return "FSCTRL1";                      // 0x0B
  case CC1100_REG_FSCTRL0  : return "FSCTRL0";                      // 0x0C
  case CC1100_REG_FREQ2    : return "FREQ2";                        // 0x0D
  case CC1100_REG_FREQ1    : return "FREQ1";                        // 0x0E
  case CC1100_REG_FREQ0    : return "FREQ0";                        // 0x0F
  case CC1100_REG_MDMCFG4  : return "MDMCFG4";                      // 0x10
  case CC1100_REG_MDMCFG3  : return "MDMCFG3";                      // 0x11
  case CC1100_REG_MDMCFG2  : return "MDMCFG2";                      // 0x12
  case CC1100_REG_MDMCFG1  : return "MDMCFG1";                      // 0x13
  case CC1100_REG_MDMCFG0  : return "MDMCFG0";                      // 0x14
  case CC1100_REG_DEVIATN  : return "DEVIATN";                      // 0x15
  case CC1100_REG_MCSM2    : return "MCSM2";                        // 0x16
  case CC1100_REG_MCSM1    : return "MCSM1";                        // 0x17
  case CC1100_REG_MCSM0    : return "MCSM0";                        // 0x18
  case CC1100_REG_FOCCFG   : return "FOCCFG";                       // 0x19
  case CC1100_REG_BSCFG    : return "BSCFG";                        // 0x1A
  case CC1100_REG_AGCCTRL2 : return "AGCCTRL2";                     // 0x1B
  case CC1100_REG_AGCCTRL1 : return "AGCCTRL1";                     // 0x1C
  case CC1100_REG_AGCCTRL0 : return "AGCCTRL0";                     // 0x1D
  case CC1100_REG_WOREVT1  : return "WOREVT1";                      // 0x1E
  case CC1100_REG_WOREVT0  : return "WOREVT0";                      // 0x1F
  case CC1100_REG_WORCTRL  : return "WORCTRL";                      // 0x20
  case CC1100_REG_FREND1   : return "FREND1";                       // 0x21
  case CC1100_REG_FREND0   : return "FREND0";                       // 0x22
  case CC1100_REG_FSCAL3   : return "FSCAL3";                       // 0x23
  case CC1100_REG_FSCAL2   : return "FSCAL2";                       // 0x24
  case CC1100_REG_FSCAL1   : return "FSCAL1";                       // 0x25
  case CC1100_REG_FSCAL0   : return "FSCAL0";                       // 0x26
  case CC1100_REG_RCCTRL1  : return "RCCTRL1";                      // 0x27
  case CC1100_REG_RCCTRL0  : return "RCCTRL0";                      // 0x28

    /* read only */

  case CC1100_REG_PARTNUM  : return "PARTNUM";                      // 0x30
  case CC1100_REG_VERSION  : return "VERSION";                      // 0x31
  case CC1100_REG_FREQEST  : return "FREQEST";                      // 0x32
  case CC1100_REG_LQI      : return "LQI";                          // 0x33
  case CC1100_REG_RSSI     : return "RSSI";                         // 0x34
  case CC1100_REG_MARCSTATE: return "MARCSTATE";                    // 0x35
  case CC1100_REG_WORTIME1 : return "WORTIME1";                     // 0x36
  case CC1100_REG_WORTIME0 : return "WORTIME0";                     // 0x37
  case CC1100_REG_PKTSTATUS: return "PKTSTATUS";                    // 0x38
  case CC1100_REG_VCO_VC_DAC: return "VCO_VC_DAC";                  // 0x39
  case CC1100_REG_TXBYTES  : return "TXBYTES";                      // 0x3A
  case CC1100_REG_RXBYTES  : return "RXBYTES";                      // 0x3B

  }
  return "unknown";
}

/***************************************************/
/***************************************************/
/***************************************************/
