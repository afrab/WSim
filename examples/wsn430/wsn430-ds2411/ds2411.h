#ifndef DS2411_H
#define DS2411_H

/***************************/
/** ds2411 data structure **/
/***************************/

#define DS2411_FAMILY_CODE_LEN     1 /* byte */
#define DS2411_SERIAL_NUMBER_LEN   6 /* byte */
#define DS2411_CRC_LEN             1 /* byte */

#define DS2411_REG_NUMBER_LEN   ( DS2411_FAMILY_CODE_LEN   + \
				  DS2411_SERIAL_NUMBER_LEN + \
				  DS2411_CRC_LEN )

/*
 * DS1  = 0a:00:00:00:00:00:01:01
 * DS2  = 53:00:00:00:00:00:02:01
 * DS3  = 64:00:00:00:00:00:03:01
 * DS4  = e1:00:00:00:00:00:04:01
 * DS5  = d6:00:00:00:00:00:05:01
 * DS6  = 8f:00:00:00:00:00:06:01
 * DS7  = b8:00:00:00:00:00:07:01
 * DS8  = 9c:00:00:00:00:00:08:01
 * DS9  = ab:00:00:00:00:00:09:01
 * DS10 = f2:00:00:00:00:00:0a:01
 */

/** DS2411 serial ID **/

/*
 *   MSB                                  LSB
 *   CRC : S5 : S4 : S3 : S2 : S1 : S0 : FAMILY
 */

/* MSP430 is little endian */
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

union ds2411_serial_number_union_t
{
  uint8_t                 raw[DS2411_REG_NUMBER_LEN];
  struct ds2411_fields_t  fields;
};

typedef union ds2411_serial_number_union_t ds2411_serial_number_t;

/***************************/
/** ds2411 1wire commands **/
/***************************/

#define DS2411_CMD_LEN                1 /* byte */

#define DS2411_READ_ROM            0x33
#define DS2411_SEARCH_ROM          0xF0
#define DS2411_OVERDRIVE_SKIP_ROM  0x3C
#define DS2411_FAMILY_CODE         0x01

/************************/
/** ds2411 error codes **/
/************************/

#define DS2411_BAD_FAMILY_CODE       -1
#define DS2411_BAD_CRC               -2

enum ds2411_result_t {
  DS2411_ERROR   = 0, 
  DS2411_SUCCESS = 1
};

/***************/
/** functions **/
/***************/
enum ds2411_result_t ds2411_init     (void);
void                 ds2411_get_id   (ds2411_serial_number_t *id);
void                 ds2411_print_id (ds2411_serial_number_t *id);

#endif
