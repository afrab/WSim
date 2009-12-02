/*
 * Copyright  2008-2009 INRIA/SensTools
 * 
 * <dev-team@sentools.info>
 * 
 * This software is a set of libraries designed to develop applications
 * for the WSN430 embedded hardware platform.
 * 
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */

/**
 *  \file   ds2411.c
 *  \brief  DS2411 electronic ID driver
 **/

#include <io.h>

#include "ds2411.h"

/***************************/
/** ds2411 1wire commands **/
/***************************/
#define DS2411_READ_ROM            0x33
#define DS2411_SEARCH_ROM          0xF0
#define DS2411_OVERDRIVE_SKIP_ROM  0x3C
#define DS2411_FAMILY_CODE         0x01

/************************/
/** ds2411 error codes **/
/************************/

#define DS2411_BAD_FAMILY_CODE       -1
#define DS2411_BAD_CRC               -2

ds2411_serial_number_t ds2411_id;


enum ds2411_result_t {
  DS2411_ERROR   = 0, 
  DS2411_SUCCESS = 1
};

/***************************************************/
/***************************************************/
/***************************************************/

/* adjust timing loop according to wait loop */

// 8MHz -> *2
// 4MHz -> *1

#define MICRO  *2

enum ds2411_timing_t // micro seconds
  {
    STD_A = 6   MICRO,  // t_W1L / t_RL
    STD_B = 64  MICRO,  // t_SLOT - t_W1L
    STD_C = 60  MICRO,  // t_W0L
    STD_D = 10  MICRO,  // t_SLOT - t_W0L

    STD_E = 6   MICRO,  // near-max t_MSR

    STD_F = 55  MICRO,  // t_REC
    STD_G = 0   MICRO,  // unused
    STD_H = 480 MICRO,  // t_RSTL 
    STD_I = 70  MICRO,  // t_MSP
    STD_J = 410 MICRO   // t_REC
  };


static void __inline__ micro_wait(register unsigned int n)
{
  /* MCLK is running 8MHz, 1 cycle = 125ns    */
  /* n=1 -> waiting = 4*125ns = 500ns         */

  /* MCLK is running 4MHz, 1 cycle = 250ns    */
  /* n=1 -> waiting = 4*250ns = 1000ns        */

    __asm__ __volatile__ (
		"1: \n"
		" dec	%[n] \n"      /* 2 cycles */
		" jne	1b \n"        /* 2 cycles */
        : [n] "+r"(n));
}


/***************************************************/
/***************************************************/
/***************************************************/
/**
 * Compute CRC for a new byte.
 * \param crc the previous crc value.
 * \param byte the nex byte
 * \return the new crc value taking the new byte into account
 */
static uint8_t ds2411_crc8_byte( uint8_t crc, uint8_t byte )
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

#define DS2411_PIN (1 << 4)

#define MSP430_DIR_ds2411_INPUT()        P2DIR &= ~DS2411_PIN
#define MSP430_DIR_ds2411_OUTPUT()       P2DIR |=  DS2411_PIN
#define MSP430_SEL_ds2411_IO()           P2SEL &= ~DS2411_PIN
#define MSP430_SEL_ds2411_FUN()          P2SEL |=  DS2411_PIN
#define MSP430_SET_ds2411_LOW()          P2OUT &= ~DS2411_PIN
#define MSP430_SET_ds2411_HIGH()         P2OUT |=  DS2411_PIN
#define MSP430_READ_ds2411()            (P2IN >> 4) & 1

#define MSP430_WRITE_ds2411(val)     \
do {                           \
    if (val)                \
         P2OUT |=  BIT(b);  \
       else                 \
         P2OUT &= ~BIT(b);  \
} while (0)
   
/* DS2411 Pin to MSP430 Pin */

#define  DS2411Pin_init()                   \
do {                                        \
  MSP430_SEL_ds2411_IO();                   \
  MSP430_DIR_ds2411_INPUT();                \
  MSP430_SET_ds2411_LOW();                  \
} while(0)

#define DS2411Pin_output_low()              \
do {                                        \
  MSP430_DIR_ds2411_OUTPUT();               \
  MSP430_SET_ds2411_LOW();                  \
} while (0)

#define DS2411Pin_output_high()             \
do {                                        \
  MSP430_DIR_ds2411_OUTPUT();               \
  MSP430_SET_ds2411_HIGH();                 \
} while (0)

#define DS2411Pin_prepare_read()            MSP430_DIR_ds2411_INPUT()
#define DS2411Pin_read()                    MSP430_READ_ds2411()

/***************************************************/
/* INIT ********************************************/
/***************************************************/
/**
 * Execute the reset procedure as defined
 * by the 1-Wire protocol.
 */
static int ds2411_reset(void) // >= 960us
{
  int present;
  DS2411Pin_output_low();
  micro_wait(STD_H);            // t_RSTL
  DS2411Pin_output_high();
  DS2411Pin_prepare_read();
  micro_wait(STD_I);            // t_MSP
  present = DS2411Pin_read();
  micro_wait(STD_J);            // t_REC
  return (present == 0);
}

/***************************************************/
/* WRITE BIT ***************************************/
/***************************************************/

/**
 * Write a '0' bit with the 1-Wire protocol
 * to the DS2411 device.
 */
static void ds2411_write_bit_one(void) // >= 70us
{
  DS2411Pin_output_low();
  micro_wait(STD_A);  //t_W1L
  DS2411Pin_output_high();
  micro_wait(STD_B);  //t_SLOT - t_W1L
}

/**
 * Write a '1' bit with the 1-Wire protocol
 * to the DS2411 device.
 */
static void ds2411_write_bit_zero(void) // >= 70us
{
  DS2411Pin_output_low();
  micro_wait(STD_C);  //t_W0L
  DS2411Pin_output_high();
  micro_wait(STD_D);  //t_SLOT - t_W0L
}

/***************************************************/
/* WRITE *******************************************/
/***************************************************/

/**
 * Write one bit with the 1-Wire protocol
 * to the DS2411 device.
 * \param is_one the bit to write
 */
static void ds2411_write_bit( int is_one ) // >= 70us
{
  if(is_one)
    ds2411_write_bit_one();
  else
    ds2411_write_bit_zero();
}

/**
 * Write one byte with the 1-Wire protocol
 * to the DS2411 device.
 * \param byte the byte to write
 */
static void ds2411_write_byte( uint8_t byte ) // >= 560us
{
  uint8_t bit;
  for( bit=0x01; bit!=0; bit<<=1 )
    ds2411_write_bit( byte & bit );
}

/***************************************************/
/* READ ********************************************/
/***************************************************/
/**
 * Read 1 bit with the 1-Wire protocol
 * from the DS2411 device.
 * \return the read bit
 */
static uint8_t ds2411_read_bit(void) // >= 70us
{
  int bit;
  DS2411Pin_output_low();
  micro_wait(STD_A);  //t_RL
  DS2411Pin_output_high();
  DS2411Pin_prepare_read();
  micro_wait(STD_E);  //near-max t_MSR
  bit = DS2411Pin_read();
  micro_wait(STD_F);  //t_REC
  return bit;
}

/**
 * Read one byte with the 1-Wire protocol
 * from the DS2411 device.
 * \return the byte read
 */
static uint8_t ds2411_read_byte(void) // >= 560us
{
  uint8_t byte = 0;
  uint8_t bit;
  for( bit=0x01; bit!=0; bit<<=1 )
  {
    if( ds2411_read_bit() )
      byte |= bit;
  }
  return byte;
}

/***************************************************/
/***************************************************/

uint16_t ds2411_init(void)
{
  uint16_t retry = 5;
  
  DS2411Pin_init();
  DS2411Pin_output_high();
  ds2411_reset();

  while( retry-- > 0 )
  {
    int crc = 0;
    if( ds2411_reset() )
    {
      uint16_t i;
      ds2411_write_byte(DS2411_READ_ROM);
      for( i=0; i<8; i++ )
      {
        ds2411_id.raw[7-i] = ds2411_read_byte();
        crc = ds2411_crc8_byte( crc, ds2411_id.raw[7-i] );
      }
      if( crc == 0 )
      {
        return 1;
      }
    }
  }
  return 0;
}
