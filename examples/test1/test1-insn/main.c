
#ifndef LINUX
#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include "uart.h"
#else
#include <inttypes.h>
#endif

#include <stdio.h>
#include <string.h>


/**********************
 * Delay function.
 **********************/

#define DELAY 0x100

#ifndef LINUX
void delay(unsigned int d) 
{
  int i,j;
  for(j=0; j < 0xff; j++)
    {
      for (i = 0; i<d; i++) 
	{
	  nop();
	  nop();
	}
    }
}
#else
void delay(unsigned int d)
{
}
#endif
/**********************
 * printf 
 **********************/

#ifndef LINUX
int putchar(int c)
{
  return uart0_putchar(c);
}
#endif

/**********************/
/**********************/

uint8_t onewire_crc8_byte( uint8_t crc, uint8_t byte )
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

uint8_t onewire_crc8_bytes( uint8_t crc, uint8_t* bytes, uint8_t len )
{
  uint8_t* end = bytes+len;
  while( bytes != end )
    crc = onewire_crc8_byte( crc, *bytes++ );
  return crc;
}

/**********************/
/**********************/

uint8_t test_and(uint8_t crc, uint8_t* bytes, uint8_t len)
{
  int i;
  uint8_t* end = bytes+len;
  while( bytes != end )
    {
      crc ^= *bytes++;
      for(i=0; i<8; i++)
	{
	  if (crc & 1)
	    crc = (crc >> 1) & 0x8c;
	  else
	    crc = crc >> 1;
	}
    }
  return crc;
}

/**********************/
/**********************/

void test_int(int cc)
{
  if ((cc>=1) && (cc<=3))
    {
      printf("ok\n");
    }
  else
    {
      printf("fail\n");
    }
}

void test_char(char cc)
{
  if ((cc>=1) && (cc<=3))
    {
      printf("ok\n");
    }
  else
    {
      printf("fail\n");
    }
}


/**********************/
/**********************/

void test_equal(int n, uint16_t val, uint16_t res)
{
  if (n < 10) printf("test  ");
  else  printf("test ");

  if (val == res)
    printf("%d: ok\n",n);
  else
    printf("%d: failed val:%x res:%x\n",n,val,res);
}

void test_equalf8(int n, uint8_t (*f)(uint8_t,uint8_t),uint8_t val1, uint8_t val2, uint8_t res)
{
  test_equal(n,f(val1,val2),res);
}

void test_equalf16(int n, uint16_t (*f)(uint16_t,uint16_t),uint16_t val1, uint16_t val2, uint16_t res)
{
  test_equal(n,f(val1,val2),res);
}

/**********************/
/**********************/

uint8_t  tand8 (uint8_t  v1, uint8_t  v2) {return v1 & v2;}
uint8_t  tor8  (uint8_t  v1, uint8_t  v2) {return v1 | v2;}
uint8_t  txor8 (uint8_t  v1, uint8_t  v2) {return v1 ^ v2;}

/* v1 = (~v2 & v1) */
uint8_t  tbic8 (register uint8_t v1, register uint8_t v2) 
{ 
  __asm__ __volatile__ ("bic.b %1, %0" : "=r" (v1) : "r" (v2)); 
  return v1; 
}

/* v1 = (v2 | v1) */
uint8_t  tbis8 (register uint8_t v1, register uint8_t v2) 
{ 
  __asm__ __volatile__ ("bis.b %1, %0" : "=r" (v1) : "r" (v2)); 
  return v2; 
}

/* v1 = (v2 & v1) */
uint8_t  tbit8 (register uint8_t v1, register uint8_t v2) 
{ 
  __asm__ __volatile__ ("bit.b %1, %0" : "=r" (v1) : "r" (v2));
  return v2; 
}

uint16_t tand16(uint16_t v1, uint16_t v2) {return v1 & v2;}
uint16_t tor16 (uint16_t v1, uint16_t v2) {return v1 | v2;}
uint16_t txor16(uint16_t v1, uint16_t v2) {return v1 ^ v2;}

/********************************************************/
/********************************************************/
/********************************************************/

int main(void) 
{

#if !defined(LINUX)
  asm("mov #0,r15");
  asm("mov #0,r14");
  asm("mov #0,r13");
  asm("mov #0,r12");
  asm("mov #0,r11");
  asm("mov #0,r10");
  asm("mov #0,r9");
  asm("mov #0,r8");
  asm("mov #0,r7");
  asm("mov #0,r6");
  asm("mov #0,r5");
  asm("mov #0,r4");

  uart0_init();
#endif

  asm("add #0,r0");

  printf("sizeof char      : %d\n",sizeof(char));
  printf("sizeof short     : %d\n",sizeof(short));
  printf("sizeof int       : %d\n",sizeof(int));
  printf("sizeof long      : %d\n",sizeof(long int));
  printf("sizeof long long : %d\n",sizeof(long long int));

  printf("Test program ready.\n");

  printf("should fail: ");
  test_int(-1);
  printf("should fail: ");
  test_int(0);
  printf("should pass: ");
  test_int(1);
  printf("should pass: ");
  test_int(2);
  printf("should fail: ");
  test_int(8);

  printf("should fail: ");
  test_char(-1);
  printf("should fail: ");
  test_char(0);
  printf("should pass: ");
  test_char(2);
  printf("should fail: ");
  test_char(8);

  printf("  [test] = [%s]\n","test");
  printf("  [2]    = [%d]\n",2);
  printf("  [80]   = [%x]\n",(3 << 7) & 0xff);

  test_equal   (1 ,onewire_crc8_bytes(0,(uint8_t*)"01:02:03:04:05:06:07",7),0x5a);
  test_equal   (2 ,test_and(0,(uint8_t*)"01:02:03:04:05:06:07",7),0);

  test_equalf16(11 ,tand16,0xffff,0xff,0xff);
  test_equalf16(12 ,tand16,0xff  ,0xff,0xff);
  test_equalf16(13 ,txor16,0xfaff,0xff,0xfa00);
  test_equalf16(14 ,tand16,0xff  ,0xff,0xff);

  test_equalf8 (21 ,tand8,0xffu,0xffu,0xffu);
  test_equalf8 (22 ,tand8,0xff   ,0xffu,0xffu);
  test_equalf8 (23 ,txor8,0xffu,0xffu,0x00);

  /* bic (~v2 & v1) */
  test_equalf8 (24 ,tbic8,0xff  ,0x00,0xff);
  test_equalf8 (25 ,tbic8,0xaa  ,0x00,0xaa);
  test_equalf8 (26 ,tbic8,0xff  ,0xff,0x00);


#define MOV_R4R4()   __asm__ __volatile__ ("mov r4,r4")
#define MOV_R5R5()   __asm__ __volatile__ ("mov r5,r5")
#define MOV_R6R6()   __asm__ __volatile__ ("mov r6,r6")
#define MOV_R7R7()   __asm__ __volatile__ ("mov r7,r7")
#define MOV_R8R8()   __asm__ __volatile__ ("mov r8,r8")
#define MOV_R9R9()   __asm__ __volatile__ ("mov r9,r9")
#define MOV_R10R10() __asm__ __volatile__ ("mov r10,r10")
#define MOV_R11R11() __asm__ __volatile__ ("mov r11,r11")
#define MOV_R12R12() __asm__ __volatile__ ("mov r12,r12")
#define MOV_R13R13() __asm__ __volatile__ ("mov r13,r13")
#define MOV_R14R14() __asm__ __volatile__ ("mov r14,r14")
#define MOV_R15R15() __asm__ __volatile__ ("mov r15,r15")

  MOV_R4R4();
  MOV_R5R5();
  MOV_R6R6();
  MOV_R7R7();
  MOV_R8R8();
  MOV_R9R9();
  MOV_R10R10();
  MOV_R11R11();
  MOV_R12R12();
  MOV_R13R13();
  MOV_R14R14();
  MOV_R15R15();

#if defined(LINUX)
  while (1)
    {
      int c;
      while ((c = uart0_getchar()))
	{
	  printf("%c",c);
	}
    }
#else
  LPM4;
#endif
  return 0;
}
