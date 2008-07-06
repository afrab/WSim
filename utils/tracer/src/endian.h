
/**
 *  \file   endian.h
 *  \brief  Tracer endianess handling
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef TRACER_ENDIAN_H
#define TRACER_TENDIAN_H

#include <inttypes.h>

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ENDIAN_BIG    0
#define ENDIAN_LITTLE 1

char*    endian_tostring (int e);

uint16_t endian_swap2    (uint16_t v);
uint32_t endian_swap4    (uint32_t v);
uint64_t endian_swap8    (uint64_t v);


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
