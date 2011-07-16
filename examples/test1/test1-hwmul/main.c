
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

#ifndef LINUX
int putchar(int c)
{
  return uart0_putchar(c);
}
#endif

/********************************************************/
/********************************************************/
/********************************************************/
uint32_t test_uint16_uint16(uint16_t u, uint16_t v)
{
  return u*v;
}

uint32_t test_uint32_uint16(uint32_t u, uint16_t v)
{
  return u*v;
}

uint32_t test_uint32_uint32(uint32_t u, uint32_t v)
{
  return u*v;
}

uint16_t test_uint8_uint8(uint8_t u, uint8_t v)
{
  return u*v;
}




int32_t test_uint16_int16(uint16_t u, int16_t v)
{
  return u*v;
}

int32_t test_uint32_int16(uint32_t u, int16_t v)
{
  return u*v;
}

int32_t test_uint32_int32(uint32_t u, int32_t v)
{
  return u*v;
}

int16_t test_uint8_int8(uint8_t u, int8_t v)
{
  return u*v;
}




int16_t test_int16_int16(int16_t u, int16_t v)
{
  return u*v;
}

int32_t test_int16_int16_to_int32(int16_t u, int16_t v)
{
  return u*v;
}

int32_t test_int32_int16(int32_t u, int16_t v)
{
  return u*v;
}

int32_t test_int32_int32(int32_t u, int32_t v)
{
  return u*v;
}

int16_t test_int8_int8(int8_t u, int8_t v)
{
  return u*v;
}



void test_hwmul()
{
  //
  // 43953  ==               1010101110110001
  // 10     ==                           1010
  // 439530 == 1101011010011101010
  //              ^^^^^^^^^^^^^^^^ / 16 bits
  //              1011010011101010 == 46314
  // 
  // 
  printf(" result  expected / preprocessor\n");
  printf("\n unsigned:\n");
  printf("%7lu ==  46314 / %u\n", test_uint16_uint16(43953U,10U), 43953U*10U    );

  /* operands are both UL, result is as expected   */
  printf("%7lu == 439530 / %lu\n", test_uint32_uint32(43953LU,10LU), 43953LU*10LU);

  /* only one operand is UL, result is as expected */
  printf("%7lu == 439530 / %lu\n", test_uint32_uint16(43953U,10), 43953LU*10     );
  printf("%7lu == 439530 / %lu\n", test_uint32_uint32(43953U,10), 43953LU*10     );

  /* types are promoted to unsigned int, results are as expected */
  printf("%7u ==   2574 / %u\n",     test_uint8_uint8  (234U,11U),     234U*11U  );
  printf("%7u ==  30654 / %u\n",    test_uint8_uint8  (234U,131U),    234U*131U  );



  //
  //  43953  ==                 1010101110110001
  // -10     ==                 1111111111110110                    
  // -439530 == 11111111111110010100101100010110
  //                            ^^^^^^^^^^^^^^^^ / 16 bits
  //                            0100101100010110 == 19222
  //                            1101111000100000
  // 
  printf("\n unsigned * signed:\n");
  printf("%7ld ==   19222 / %ld\n", test_uint16_int16(43953U,-10), 43953L * -10    );

  /* operands are both UL, result is as expected   */
  printf("%7ld == -439530 / %ld\n", test_uint32_int32(43953LU,-10), 43953LU * -10L );

  /* only one operand is UL, result is as expected */
  printf("%7ld == -439530 / %ld\n", test_uint32_int16(43953U,-10), 43953LU*-10     );
  printf("%7ld == -439530 / %ld\n", test_uint32_int32(43953U,-10), 43953LU*-10     );

  /* types are promoted to unsigned int, results are as expected */
  printf("%7d ==   -2574 / %d\n",     test_uint8_int8  (234U,-11),    -11*234      );
  printf("%7d ==  -28080 / %d\n",    test_uint8_int8  (234U,-120),    -120*234     );



  //
  // -12340  ==                 1100111111001100
  // -10     ==                 1111111111110110                    
  //  123400 == 00000000000000011110001000001000
  //                            ^^^^^^^^^^^^^^^^ / 16 bits
  //                            1110001000001000 == -7672
  // 
  printf("\n signed * signed:\n");
  printf("%7d ==  -7672 / %d (integer overflow)\n", test_int16_int16(-12340,-10), -12240 * -10 );
  printf("%7ld ==  -7672 / %d (integer overflow)\n", test_int16_int16_to_int32(-12340,-10), -12240 * -10 );

  /* operands are both UL, result is as expected   */
  printf("%7ld == 122400 / %ld\n", test_int32_int32(-12240,-10), -12240L * -10  );

  /* only one operand is UL, result is as expected */
  printf("%7ld == 122400 / %ld\n", test_int32_int16(-12240,-10), -12240L * -10  );
  printf("%7ld == 122400 / %ld\n", test_int32_int32(-12240,-10), -12240L * -10  );

  /* types are promoted to unsigned int, results are as expected */
  printf("%7d ==   1320 / %d\n",  test_int8_int8  (-120,-11),    -11 * -120     );
  printf("%7d ==  15000 / %d\n",  test_int8_int8  (-125,-120),    -120 * -125   );
}

/********************************************************/
/********************************************************/
/********************************************************/

int main(void) 
{
  WDTCTL = WDTPW + WDTHOLD;

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


  printf("sizeof char      : %d\n",sizeof(char));
  printf("sizeof short     : %d\n",sizeof(short));
  printf("sizeof int       : %d\n",sizeof(int));
  printf("sizeof long      : %d\n",sizeof(long int));
  printf("sizeof long long : %d\n",sizeof(long long int));

  printf("Test program ready.\n");
  test_hwmul();

#if !defined(LINUX)
  LPM4;
#endif
  return 0;
}

/********************************************************/
/********************************************************/
/********************************************************/
