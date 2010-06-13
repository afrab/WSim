/* Copyright (c) 2006 by ARES Inria.  All Rights Reserved */

/**
 *  \file   ds2411.h
 *  \brief  Maxim DS2411 device
 *  \author Eric Fleury, Antoine Fraboulet
 *  \date   2006
 **/

/***
   NAME
     ds2411
   PURPOSE
     
   NOTES
     
   HISTORY
     efleury - Jan 28, 2006: Created.
     $Log: ds2411.h,v $
     Revision 1.4  2008-07-06 21:17:55  afraboul
     - doxygen header description
     - \brief description

     Revision 1.3  2006-07-02 18:35:48  afraboul
     - split 1wire and ds2411 automata
     - support for 1 wire protocol complete (except overdrive mode)
     - complete READROM command for ds2411

     Revision 1.2  2006/05/24 14:52:10  afraboul
     - réécriture des .h
     - début mise en forme du .c (uniformisation avec le reste)
     - passage au temps en nano
     - vérification de l'automate à faire avec temps en nano à faire/finir

     Revision 1.1  2006/03/28 21:55:40  efleury
     petit pbm de configure avec new version...

***/

#ifndef __DS2411
#define __DS2411

/***************************/
/** ds2411 data structure **/
/***************************/

#define DS2411_FAMILY_CODE_LEN     1 /* byte */
#define DS2411_SERIAL_NUMBER_LEN   6 /* byte */
#define DS2411_CRC_LEN             1 /* byte */

#define DS2411_REG_NUMBER_LEN   ( DS2411_FAMILY_CODE_LEN   + \
				  DS2411_SERIAL_NUMBER_LEN + \
				  DS2411_CRC_LEN )

/** DS2411 serial ID **/

/*
 *   MSB                                  LSB
 *
 *   CRC : S5 : S4 : S3 : S2 : S1 : S0 : FAMILY
 *
 */

#if defined(WORDS_BIGENDIAN)
struct ds2411_fields_t
{
  uint8_t crc;      /* MSB */
  uint8_t serial5;
  uint8_t serial4;
  uint8_t serial3;
  uint8_t serial2;
  uint8_t serial1;
  uint8_t serial0;
  uint8_t family;   /* LSB */
};
#else
struct ds2411_fields_t
{
  uint8_t family;   /* LSB */
  uint8_t serial0;
  uint8_t serial1;
  uint8_t serial2;
  uint8_t serial3;
  uint8_t serial4;
  uint8_t serial5;
  uint8_t crc;      /* MSB */
};
#endif

union ds2411_serial_number_union_t
{
  uint64_t                rawint;
  uint8_t                 raw[DS2411_REG_NUMBER_LEN];
  struct ds2411_fields_t  fields;
};

typedef union ds2411_serial_number_union_t ds2411_serial_number_t;

/************************/
/** ds2411 error codes **/
/************************/

#define DS2411_BAD_FAMILY_CODE       -1
#define DS2411_BAD_CRC               -2

#endif /* __DS2411 */
