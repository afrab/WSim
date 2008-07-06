
/**
 *  \file   main.c
 *  \brief  Worlsens crc tools
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define MAXLINE 200

/***************************************************/
/***************************************************/
/***************************************************/

#if defined(DEBUG)
#   define DBG(x...)  fprintf(stderr,x)
#else
#   define DBG(x...)  do { } while (0)
#endif 
/***************************************************/
/***************************************************/
/***************************************************/

/* 
 * CRC  x8 + x5 + x4 + 1
 *
 * Numerical Recipies in C : the art of scientific computing
 * ch 20.3 Cyclic Redundancy and Other Checksums (page 896)
 *
 */

static uint8_t crc8_byte( uint8_t crc, uint8_t byte )
{
  int i;
  crc ^= byte;
  for( i=0; i<8; i++ )
    {
      if( crc & 1 )
        crc = (crc >> 1) ^ 0x8c;
      else
        crc >>= 1;
    }
  return crc;
}

static uint8_t crc8_bytes( uint8_t crc, uint8_t* bytes, uint8_t len )
{
  uint8_t* end = bytes+len;
  while( bytes != end )
    {
      crc = crc8_byte( crc, *bytes );
      bytes++;
    }
  return crc;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SERIAL_DEFAULT_ID  "0f:07:06:05:04:03:02:01"
#define SERIAL_MASK_STR    "xx:xx:xx:xx:xx:xx:xx:xx"
#define SERIAL_ID_STR      "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x"

#define CRC_OK  0
#define CRC_ERR 1

#define MAX_LEN 8

uint8_t check_crc(char* line)
{
  int n,i;
  int     val[MAX_LEN];
  uint8_t ui8[MAX_LEN];
  uint8_t crc;

  n = sscanf(line, SERIAL_ID_STR, 
	     & val[0], & val[1], & val[2], & val[3], 
	     & val[4], & val[5], & val[6], & val[7]);

  DBG("read %d :: ",n);
  for(i=0; i < n ; i++)
    {
      ui8[n - i - 1] = val[i];
      DBG("%02x ",ui8[i]);
    }

  crc = crc8_bytes(0,ui8,n);
  DBG(":: crc == 0x%x\n",crc);
  return crc;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int main(int argc, char* argv[])
{
  FILE *fin,*fout;
  uint8_t crc;
  char crcline[MAXLINE];

  fin  = stdin;
  fout = stdout;

  while (fgets(crcline,MAXLINE - 1,fin) != NULL)
    {
      crc = check_crc(crcline);
      fprintf(fout,"crc=0x%x\n",crc);
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
