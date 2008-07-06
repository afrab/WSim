
/**
 *  \file   cc2420_crc_ccitt.h
 *  \brief  CC2420 packet CRC
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_crc_ccitt.h
 *  
 *
 *  Created by Nicolas Boulicault on 04/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 *
 * CRC calculations for the polynomial x^16 + x^12 + x^5 + 1
 * stolen from "Numerical recipes in C", p 900
 *
 */

#ifndef _CC2420_CRC_CCITT_
#define _CC2420_CRC_CCITT_

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * compute a CRC from a buffer
 */
unsigned short cc2420_icrc(unsigned char *bufptr, unsigned long len);

/***************************************************/
/***************************************************/
/***************************************************/
/*
 * display an ushort as a couple of bytes
 */
void cc2420_display_crc(unsigned short crc);

/***************************************************/
/***************************************************/
/***************************************************/

#endif
