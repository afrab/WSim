
/**
 *  \file   cc2420_crc_ccitt.c
 *  \brief  CC2420 packet CRC
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_crc_ccitt.c
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

#include <stdio.h>
#include <string.h>

#include "cc2420_macros.h"
#include "cc2420_crc_ccitt.h"

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * init function
 * Given a remainder up to now, return the new CRC after one character is added. This routine
 * is functionally equivalent to icrc(,,1,-1,1), but slower. It is used by icrc to initialize its
 * table.
 */

unsigned short cc2420_icrc1(unsigned short crc, unsigned char onech) {
    int i;
    unsigned short ans=(crc ^ onech << 8);
    /* Here is where 8 one-bit shifts, and some XORs with the
       generator polynomial, are done */
    for (i=0;i<8;i++) { 
	if (ans & 0x8000) {
	    /* AFR: changed from ((ans <<=1) ^ 4129) to ((ans << 1) ^ 4129) */
	    /* AFR: prevents "ans might be undefined" warning               */
	    ans = (ans << 1) ^ 4129; 
	}
	else {
	    ans <<= 1;
	}
    }
    return ans;
}


/***************************************************/
/***************************************************/
/***************************************************/

/*
 * CRC function
 */

unsigned short cc2420_icrc(unsigned char *bufptr, unsigned long len) {

    static unsigned short icrctb[256],init=0;
    static unsigned char rchr[256];
    unsigned short j,cword=0;
    static unsigned char it[16]={0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};
    
    /* Table of 4-bit bit-reverses. */
    if (!init) {
	init=1;
	for (j=0;j<=255;j++) {
	    /* The two tables are: CRCs of all characters, and bit-reverses of all characters. */
	    icrctb[j]=cc2420_icrc1(j << 8,(unsigned char)0);
	    rchr[j]=(unsigned char)(it[j & 0xF] << 4 | it[j >> 4]);
	}
    }

    /* Main loop over the characters in the array. */
    for (j=0;j<len;j++) 
	cword=icrctb[rchr[bufptr[j]] ^ CC2420_HIBYTE(cword)] ^ CC2420_LOBYTE(cword) << 8;

    return (rchr[CC2420_HIBYTE(cword)] | rchr[CC2420_LOBYTE(cword)] << 8);

}


/***************************************************/
/***************************************************/
/***************************************************/

/*
 * display an unsigned short as a couple of bytes
 */
#if defined(DEBUG)
void cc2420_display_crc(unsigned short crc) 
{
  unsigned char c1 = CC2420_LOBYTE(crc);
  unsigned char c2 = CC2420_HIBYTE(crc);
  fprintf(stdout, "display_crc : 0x%x%x\n", c2, c1);
}
#endif

/***************************************************/
/***************************************************/
/***************************************************/
