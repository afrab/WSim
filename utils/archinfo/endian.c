
/**
 *  \file   endian.c
 *  \brief  Host CPU endian configuration
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <inttypes.h>

#if defined(GUI)
#include "SDL.h"
#endif

struct __attribute__ ((packed)) s8_t {
  char 
    b1:1,
    b2:2,
    b3:1,
    b4:4;
};

struct __attribute__ ((packed)) s16_t {
  short 
    b1:1,
    b2:2,
    b3:1,
    b4:6,
    b5:2,
    b6:4;
};

struct __attribute__ ((packed)) mm_t {
  char c1;
  char c2;
};


void bitfield_test()
{
  union {
    struct s8_t s8;
    char   s8s;
  } u8;

  union {
    struct s16_t s16;
    struct mm_t     s16c;
    short  s16s;
  } u16;

  u8.s8s = 0x42;
  printf("u8     0x%02x   %2x %2x %2x %2x\n",
	 (unsigned char) u8.s8s,
	 (unsigned char) u8.s8.b1 & 0x1,
	 (unsigned char) u8.s8.b2 & 0x3,
	 (unsigned char) u8.s8.b3 & 0x1,
	 (unsigned char) u8.s8.b4 & 0xf
	 );

  u16.s16s = 0x4281;
  printf("u16    0x%04x %2x %2x %2x %2x %2x %2x (0x%2x 0x%2x)\n",
	 (unsigned short) u16.s16s,
	 (unsigned char)  u16.s16.b1 & 0x1,
	 (unsigned char)  u16.s16.b2 & 0x3,
	 (unsigned char)  u16.s16.b3 & 0x1,
	 (unsigned char)  u16.s16.b4 & 0x3f,
	 (unsigned char)  u16.s16.b5 & 0x3,
	 (unsigned char)  u16.s16.b6 & 0xf,
	 (unsigned char)  u16.s16c.c1,
	 (unsigned char)  u16.s16c.c2
	 );

  printf("\n");
  printf("Little endian : u8  0x42 0 1 0 4 / u16 0x4281 1 0 0 28 0 4\n");
  printf("Big endian    : u8  0x42 0 2 0 2 / u16 0x4281 0 2 0  a 0 1\n");

  // pc / alpha : u8  0x42 0 1 0 4         42:   0100 0010 == 0,10,0,0010
  // ppc        : u8  0x42 0 2 0 2
  // pc / alpha : u16 0x4281 1 0 0 28 0 4  4281: 0100 0010 1000 0001 > 0,10,0,001010,00,0001
  // ppc        : u16 0x4281 0 2 0  a 0 1
}


void print_sizes()
{
  //  printf("sizeof(#) = %d\n",sizeof(#));

  printf("sizeof(char)          = %d\n",(int)sizeof(char));
  printf("sizeof(short)         = %d\n",(int)sizeof(short));
  printf("sizeof(int)           = %d\n",(int)sizeof(int));
  printf("sizeof(long int)      = %d\n",(int)sizeof(long int));
  printf("sizeof(long long int) = %d\n",(int)sizeof(long long int));
  printf("sizeof(void*)         = %d\n",(int)sizeof(void*));
  printf("sizeof(float)         = %d\n",(int)sizeof(float));
  printf("sizeof(double)        = %d\n",(int)sizeof(double));
}


void endian_test()
{
  char c[] = { 1,2,3,4 };
  
  printf("   {1,2,3,4} = 0x%x\n",((int*)c)[0]);
  switch (((int*)c)[0])
    {
    case 0x04030201:
      printf("machine is little endian\n");
      break;
    case 0x01020304:
      printf("machine is big endian\n");
      break;
    default:
      printf("unknown format machine type\n");
      break;
    }
}


int main(int argc, char* argv[])
{
  printf("==================================\n");
  print_sizes();
  printf("==================================\n");
  bitfield_test();
  printf("==================================\n");
  endian_test();
  printf("==================================\n");
  return 0;
}
