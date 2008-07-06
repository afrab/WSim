
/**
 *  \file   cc2420_macros.h
 *  \brief  CC2420 macros
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

#ifndef _CC2420_MACROS_H
#define _CC2420_MACROS_h


/*
 * byte access macros
 */


#define CC2420_LOBYTE(x) ( (unsigned char)(  x & 0x00FF) ) 
#define CC2420_HIBYTE(x) ( (unsigned char)( (x & 0xFF00) >> 8)) 

#endif
