/* Copyright (c) 2006 by ARES Inria.  All Rights Reserved */

/**
 *  \file   ds2411_dev.c
 *  \brief  Maxim DS2411 device
 *  \author Eric Fleury, Antoine Fraboulet
 *  \date   2006
 **/

/***
   NAME
     ds2411_dev
   PURPOSE
     
   NOTES
     
   HISTORY
     efleury - Jan 31, 2006: Created.

***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <strings.h> /* bzero */

#include "arch/common/hardware.h"
#include "devices/ds2411/ds2411.h"
#include "devices/ds2411/ds2411_dev.h"

#undef DEBUG
#undef DEBUG_ME_HARDER

//#define DEBUG
//#define DEBUG_ME_HARDER
//#define DEBUG_WIRE
//#define DEBUG_CRC
//#define DEBUG_SERIAL

#ifdef DEBUG
#define HW_DMSG_DS2411(x...) fprintf(stderr, x)
#else
#define HW_DMSG_DS2411(x...) do {} while(0)
#endif

#define HW_DMSG_DSTATE(x...)                                                            \
do {                                                                                    \
   HW_DMSG_DS2411("ds2411: ======================================================\n");  \
   HW_DMSG_DS2411(x);                                                                   \
   HW_DMSG_DS2411("ds2411: ======================================================\n");  \
} while(0)

#ifdef DEBUG_ME_HARDER
#define HW_DMSG_DSH(x...)  fprintf(stderr, x)
#else
#define HW_DMSG_DSH(x...)  do {} while(0)
#endif

#ifdef DEBUG_WIRE
#define HW_DMSG_WIRE(x...)  fprintf(stderr,x)
#else
#define HW_DMSG_WIRE(x...)  do {} while(0)
#endif

#ifdef DEBUG_CRC
#define HW_DMSG_CRC(x...)  fprintf(stderr,x);
#else
#define HW_DMSG_CRC(x...)  do {} while(0)
#endif

#ifdef DEBUG_SERIAL
#define HW_DMSG_SER(x...)  fprintf(stderr,x);
#else
#define HW_DMSG_SER(x...)  do {} while(0)
#endif

/***************************/
/* Time unit factors       */
/***************************/

#define MILLI *1000*1000llu
#define MICRO *1000llu
#define NANO  *1llu

/***************************/
/* Standard speed settings */
/***************************/

/* TIME from DS2411 - page 2-3 */

/* Reset Low time              */
#define ONEWIRE_RSTL_MIN   (480 MICRO)
#define ONEWIRE_RSTL_MAX   (640 MICRO)
/* Presence detect High time   */
#define ONEWIRE_PDH_MIN    ( 15 MICRO)
#define ONEWIRE_PDH_MAX    ( 60 MICRO)
/* Presence detect Low time    */
#define ONEWIRE_PDL_MIN    ( 60 MICRO)
#define ONEWIRE_PDL_MAX    (240 MICRO)
/* Presence detect Fall time   */ 
#define ONEWIRE_FPD_MIN    (400  NANO) 
#define ONEWIRE_FPD_MAX    (  8 MICRO)
/* Presence detect sample time */
#define ONEWIRE_MSP_MIN    (  6 MICRO) 
#define ONEWIRE_MSP_MAX    ( 10 MICRO)
/* Rising Edge HoldOff         */
#define ONEWIRE_REH_MIN    (1250 NANO) 
#define ONEWIRE_REH_MAX    (  5 MICRO)
/* Recovery time               */
#define ONEWIRE_REC        (  5 MICRO)
/* Timeslot duration           */
#define ONEWIRE_SLOT_MIN   ( 65 MICRO)
#define ONEWIRE_SLOT_MAX   ( 70 MICRO)
/* Write 0 low time            */
#define ONEWIRE_W0L_MIN    ( 60 MICRO)
#define ONEWIRE_W0L_MAX    (120 MICRO) 
/* Write 1 low time            */
#define ONEWIRE_W1L_MIN    (  5 MICRO)
#define ONEWIRE_W1L_MAX    ( 15 MICRO)
/* Read Low time               */
#define ONEWIRE_RL_MIN     (  6 MICRO) // check RL_MIN
#define ONEWIRE_RL_MAX     ( 15 MICRO)
/* Read Sample time            */
#define ONEWIRE_MSR_MIN    (  1 MICRO)
#define ONEWIRE_MSR_MAX    ( 15 MICRO)

/****************************************/
/* Need to add overdrive speed settings */
/****************************************/

enum onewire_timing_enum_t {
  /* Reset Low time              */
  _ONEWIRE_RSTL_MIN    = 0,
  _ONEWIRE_RSTL_MAX    = 1,
  /* Presence detect High time   */
  _ONEWIRE_PDH_MIN     = 2,
  _ONEWIRE_PDH_MAX     = 3,
  /* Presence detect Low time    */
  _ONEWIRE_PDL_MIN     = 4,
  _ONEWIRE_PDL_MAX     = 5,
  /* Presence detect Fall time   */ 
  _ONEWIRE_FPD_MIN     = 6,
  _ONEWIRE_FPD_MAX     = 7,
  /* Presence detect sample time */
  _ONEWIRE_MSP_MIN     = 8,
  _ONEWIRE_MSP_MAX     = 9,
  /* Rising Edge HoldOff         */
  _ONEWIRE_REH_MIN     = 10,
  _ONEWIRE_REH_MAX     = 11,
  /* Recovery time               */
  _ONEWIRE_REC         = 12,
  /* Timeslot duration           */
  _ONEWIRE_SLOT        = 13,
  /* Write 0 low time            */
  _ONEWIRE_W0L_MIN     = 14,
  _ONEWIRE_W0L_MAX     = 15,
  /* Write 1 low time            */
  _ONEWIRE_W1L_MIN     = 16,
  _ONEWIRE_W1L_MAX     = 17,
  /* Read Low time               */
  _ONEWIRE_RL_MIN      = 18,
  _ONEWIRE_RL_MAX      = 19,
  /* Read Sample time            */
  _ONEWIRE_MSR_MIN     = 20,
  _ONEWIRE_MSR_MAX     = 21
};    


/*
static uint64_t onewire_timings[2][22] = 
  {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  };
*/

enum onewire_mode_enum_t {
  ONEWIRE_SPEED_STD           = 0,
  ONEWIRE_SPEED_OVD           = 1
};

enum onewire_command_enum_t {
  ONEWIRE_CMD_NONE            = 0x00,
  ONEWIRE_READ_ROM            = 0x33,
  ONEWIRE_SEARCH_ROM          = 0xF0,
  ONEWIRE_OVERDRIVE_SKIP_ROM  = 0x3C
};

enum onewire_trans_enum_t {
  ONEWIRE_TRANS_LOW           = 0,
  ONEWIRE_TRANS_HIGH          = 1,
  ONEWIRE_TRANS_NONE          = 2,
};

enum onewire_lvl0_state_enum_t {
  ONEWIRE_LVL0_RESET          = 0,
  ONEWIRE_LVL0_READ_COMMAND   = 1,
  ONEWIRE_LVL0_DEVICE
};

enum onewire_reset_state_enum_t {
  ONEWIRE_RESET_WAIT          = 0,
  ONEWIRE_RESET_PULSE         ,
  ONEWIRE_PRESENCE_PULSE      ,
  ONEWIRE_PRESENCE_PULSE_END  ,
  ONEWIRE_PRESENCE_PULSE_REC  ,
};

enum onewire_rcmd_state_enum_t {
  ONEWIRE_RCMD_INIT           = 0,
  ONEWIRE_RCMD_READ           ,
};

enum onewire_signal_state_enum_t {
  ONEWIRE_WAITING_LOW         = 0,
  ONEWIRE_WAITING_HIGH        ,

  ONEWIRE_WRITE_ENDSLOT       ,

  ONEWIRE_READ_VAL            ,
  ONEWIRE_READ_HOLD           ,
  ONEWIRE_READ_REC 
};

enum onewire_wire_enum_t {
  ONEWIRE_INPUT_LOW           = 0,
  ONEWIRE_INPUT_HIGH          = 1,
  ONEWIRE_INPUT_OPEN          = 2,
};

typedef enum onewire_lvl0_state_enum_t     onewire_lvl0_state_t;
typedef enum onewire_reset_state_enum_t    onewire_reset_state_t;
typedef enum onewire_rcmd_state_enum_t     onewire_rcmd_state_t;
typedef enum onewire_signal_state_enum_t   onewire_signal_state_t;

typedef enum onewire_wire_enum_t           onewire_wire_t;
typedef enum onewire_trans_enum_t          onewire_trans_t;
typedef enum onewire_mode_enum_t           onewire_mode_t;
typedef enum onewire_command_enum_t        onewire_command_t;
typedef enum onewire_timing_enum_t         onewire_timing_t;

enum ds2411_readrom_state_enum_t {
  DS2411_READROM_INIT         = 0,
  DS2411_READROM_READ         ,
};

enum ds2411_searchrom_state_enum_t {
  DS2411_SEARCHROM_INIT       = 0,
  DS2411_SEARCHROM_hoho
};

typedef enum ds2411_readrom_state_enum_t   ds2411_readrom_state_t;
typedef enum ds2411_searchrom_state_enum_t ds2411_searchrom_state_t;
/***************************************************/
/***************************************************/
/***************************************************/

struct ds2411_dev_t 
{
  /****************************/
  /* 1 wire protocol handling */
  /****************************/
  onewire_mode_t         speed;            /* communication speed mode                           */

  /* onewire main protocol state */
  uint64_t               time_in;          /* Time when the DS2411 entered in the current state  */
  onewire_lvl0_state_t   state;            /* Current state of 1wire protocol                    */
  onewire_reset_state_t  state_reset;      /* reset / presence automata state                    */
  onewire_rcmd_state_t   state_rcmd;       /* read command automata                              */
  onewire_command_t      command;          /* current command                                    */

  /* write/read automata */
  onewire_signal_state_t state_signal;     /* signal state : write0, write1, read automata       */ 
  uint8_t                buf;              /* value read so far (i.e. from bit 0 to current_bit) */
  uint8_t                buf_bitnum;       /* bit number that we try to read/write               */

  /* write 0 or write 1 automata helpers */
  uint64_t               write_time_remain;/* used for write_one and write_zero                  */
  unsigned int           write_bit;        /* used for write_one and write_zero                  */

  /* wire interface with MCU */
  onewire_wire_t         wire;             /* wire state                                         */
  int                    read;             /* speed read update: 1 if something has to be read   */
  onewire_trans_t        read_trans;       /* 0: high to low, 1: low to high, 2: none            */
  int                    write;            /* speed write updates: 1 if we follow a write        */
  onewire_trans_t        write_trans;      /* 0: high to low, 1: low to high, 2: none            */

  /************************/
  /* DS2411 specific part */
  /************************/
  ds2411_serial_number_t   id;             /* uniq serial number of the DS2411                   */
  ds2411_readrom_state_t   state_readrom;
  int                      state_readrom_bytenumber;
  ds2411_searchrom_state_t state_searchrom;
};

#define DS2411_DATA          ((struct ds2411_dev_t*)machine.device[dev].data)

#define DS2411_SPEED         (DS2411_DATA->speed)
#define DS2411_COMMAND       (DS2411_DATA->command)

#define DS2411_LVL0_STATE    (DS2411_DATA->state)
#define DS2411_RESET_STATE   (DS2411_DATA->state_reset)
#define DS2411_RCMD_STATE    (DS2411_DATA->state_rcmd)
#define DS2411_SIGNAL_STATE  (DS2411_DATA->state_signal)

#define DS2411_WIRE_STATE    (DS2411_DATA->wire)
#define DS2411_READ_VALID    (DS2411_DATA->read)
#define DS2411_READ_TRANS    (DS2411_DATA->read_trans)
#define DS2411_WRITE_VALID   (DS2411_DATA->write)
#define DS2411_WRITE_TRANS   (DS2411_DATA->write_trans)
#define DS2411_TIME_IN       (DS2411_DATA->time_in)

#define DS2411_WTIME_REMAIN  (DS2411_DATA->write_time_remain)
#define DS2411_WBIT          (DS2411_DATA->write_bit)

#define DS2411_ID            (DS2411_DATA->id)
#define DS2411_BUF           (DS2411_DATA->buf)
#define DS2411_BUF_BITNUM    (DS2411_DATA->buf_bitnum)

#define DS2411_READROM_STATE   (DS2411_DATA->state_readrom)
#define DS2411_READROM_BYTENUM (DS2411_DATA->state_readrom_bytenumber)
#define DS2411_SEARCHROM_STATE (DS2411_DATA->state_searchrom)

/*                           7  6  5  4  3  2  1  0 */
#define SERIAL_DEFAULT_ID  "0f:07:06:05:04:03:02:01"
#define SERIAL_MASK_STR    "xx:xx:xx:xx:xx:xx:xx:xx"
#define SERIAL_ID_STR      "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x"

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

static uint8_t onewire_crc8_byte( uint8_t crc, uint8_t byte )
{
  int i;
  crc ^= byte;
  HW_DMSG_CRC("ds2411:crc:     0: crc 0x%02x\n",crc);
  for( i=0; i<8; i++ )
    {
      if( crc & 1 )
        crc = (crc >> 1) ^ 0x8c;
      else
        crc >>= 1;
      HW_DMSG_CRC("ds2411:crc:     %d: crc 0x%02x\n",i+1,crc);
    }
  return crc;
}

static uint8_t onewire_crc8_bytes( uint8_t crc, uint8_t* bytes, uint8_t len )
{
  uint8_t* end = bytes+len;
  while( bytes != end )
    {
      crc = onewire_crc8_byte( crc, *bytes );
      HW_DMSG_CRC("ds2411:crc: byte 0x%02x crc 0x%02x\n",*bytes,crc);
      bytes++;
    }
  return crc;
}

/***************************************************/
/***************************************************/
/***************************************************/

static uint8_t ds2411_crc(ds2411_serial_number_t *id)
{
  uint8_t crc = 0;
  crc = onewire_crc8_bytes(crc, id->raw, 7);
  return crc;
}

/***************************************************/
/***************************************************/
/***************************************************/

static int ds2411_check_crc(ds2411_serial_number_t *id)
{
  return ds2411_crc(id) == id->fields.crc;
}

/***************************************************/
/***************************************************/
/***************************************************/
#if defined(DEBUG_SERIAL)
static char* ds2411_id_to_str(ds2411_serial_number_t *id)
{
  static char idstr[] = SERIAL_MASK_STR;
  sprintf(idstr,SERIAL_ID_STR,
	  id->fields.crc, 
	  id->fields.serial5, id->fields.serial4,
	  id->fields.serial3, id->fields.serial2,
	  id->fields.serial1, id->fields.serial0,
	  id->fields.family);
  return idstr;
}
#endif
/***************************************************/
/***************************************************/
/***************************************************/

static ds2411_serial_number_t ds2411_str_to_id(char *serial)
{
  int n;
  ds2411_serial_number_t id;
  unsigned int i0,i1,i2,i3,i4,i5,i6,i7;

  if (serial == NULL)
    serial = SERIAL_DEFAULT_ID;

  bzero(&id, sizeof(ds2411_serial_number_t));

  n = sscanf(serial,SERIAL_ID_STR, &i0,&i1,&i2,&i3,&i4,&i5,&i6,&i7);
  if (n == DS2411_REG_NUMBER_LEN)
    {
      id.fields.crc     = i0;
      id.fields.serial5 = i1;
      id.fields.serial4 = i2;
      id.fields.serial3 = i3;
      id.fields.serial2 = i4;
      id.fields.serial1 = i5;
      id.fields.serial0 = i6; 
      id.fields.family  = i7;
      
      HW_DMSG_SER("DS2411: id = %s\n", ds2411_id_to_str(&id));
      
      if (! ds2411_check_crc(&id))
	{
	  WARNING("DS2411: crc is not valid, should be 0x%x\n",ds2411_crc(&id));
	}
    }
  return id;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds2411_device_size(void)
{
  return sizeof(struct ds2411_dev_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds2411_delete(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds2411_reset(int dev)
{
  ds2411_serial_number_t id;
  id = DS2411_ID;

  bzero(DS2411_DATA,sizeof(*(DS2411_DATA)));

  DS2411_ID           = id;
  DS2411_LVL0_STATE   = ONEWIRE_LVL0_RESET;
  DS2411_TIME_IN      = MACHINE_TIME_GET_NANO();
  DS2411_WIRE_STATE   = ONEWIRE_INPUT_HIGH;

  DS2411_READ_VALID   = 1;
  DS2411_WIRE_STATE   = 1;

  return 0;
} 

/***************************************************/
/***************************************************/
/***************************************************/

int ds2411_power_down(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds2411_power_up(int dev)
{
  return ds2411_reset(dev);
}

/***************************************************/
/***************************************************/
/***************************************************/

/* read from MCU */
void ds2411_read(int dev, uint32_t *mask, uint32_t *val)
{
  if (DS2411_READ_VALID) 
    {
      *mask             = 1;
      *val              = DS2411_WIRE_STATE;
      DS2411_READ_VALID = 0;
      HW_DMSG_WIRE("ds2411: set wire state %s [%"PRId64"]\n", 
		   (*val & DS2411_D) ? "HIGH":"LOW", MACHINE_TIME_GET_NANO());  
    }
  else
    {
      * mask            = 0;
    }
} 

/***************************************************/
/***************************************************/
/***************************************************/

/* write from MCU */
void ds2411_write(int dev, uint32_t mask, uint32_t val)
{
  if (mask & DS2411_D) 
    {
      /* 1 is HIGH, 0 is LOW */
      if (DS2411_WIRE_STATE != (val & DS2411_D))
	{
	  DS2411_WRITE_TRANS = val & DS2411_D; 
	  DS2411_WRITE_VALID = 1;
	  DS2411_WIRE_STATE  = val & DS2411_D;
	  HW_DMSG_WIRE("ds2411: write from MCU %s [+%"PRId64"]\n", 
		       (val & DS2411_D) ? "HIGH":"LOW", MACHINE_TIME_GET_NANO() - DS2411_TIME_IN);  
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

#define CANCEL_OPERATION(dev)                                          \
do {                                                                   \
  DS2411_READ_VALID = 1;                                               \
  DS2411_READ_TRANS = ONEWIRE_TRANS_HIGH;                              \
  DS2411_WIRE_STATE = ONEWIRE_INPUT_HIGH;                              \
} while (0)


#define GO_WAIT_RESET(dev)                                             \
do {                                                                   \
  CANCEL_OPERATION(dev);                                               \
  DS2411_LVL0_STATE  = ONEWIRE_LVL0_RESET;                             \
  DS2411_RESET_STATE = ONEWIRE_RESET_WAIT;                             \
} while(0)


#define RESET_ON_BAD_WIRE_STATE(dev,expected,time)                     \
do {                                                                   \
  if (DS2411_WIRE_STATE != expected) {                                 \
   GO_WAIT_RESET(dev);                                                 \
   HW_DMSG_DS2411("ds2411:    ====== bad wire state [%" PRId64 "]\n",time);   \
  }                                                                    \
} while(0)

/***************************************************/
/***************************************************/
/***************************************************/

/* Handles onewire signals during a RESET and PRESENCE PULSE */
static void update_onewire_reset_state(int dev, uint64_t current_time, uint64_t time_in_state)
{
  switch (DS2411_RESET_STATE)
    {
    case ONEWIRE_RESET_WAIT: /* IDLE */
      if (DS2411_WRITE_TRANS == ONEWIRE_TRANS_LOW)
	{
	  DS2411_RESET_STATE = ONEWIRE_RESET_PULSE;
	  DS2411_TIME_IN     = current_time;
	  HW_DMSG_DSH("ds2411: GO RESET_PULSE\n");
	}
      break;

    case ONEWIRE_RESET_PULSE: /* waiting for HIGH */
      if (DS2411_WRITE_VALID && DS2411_WRITE_TRANS == ONEWIRE_TRANS_HIGH)
	{
	  if (time_in_state < ONEWIRE_RSTL_MIN)
	    {
	      GO_WAIT_RESET(dev);
	      HW_DMSG_DS2411("ds2411:    reset_pulse canceled, up too early [+%"PRId64"]\n",time_in_state);
	    }
	  else if (time_in_state > ONEWIRE_RSTL_MAX)
	    {
	      GO_WAIT_RESET(dev);
	      HW_DMSG_DS2411("ds2411:    reset_pulse canceled, up too late [+%"PRId64"]\n",time_in_state);
	    }
	  else
	    {
	      DS2411_RESET_STATE = ONEWIRE_PRESENCE_PULSE;
	      DS2411_TIME_IN     = current_time; 
	      HW_DMSG_DSH("ds2411: GO PRESENCE_PULSE MSP [+%"PRId64"]\n",time_in_state);
	    }
	}
      break;

    case ONEWIRE_PRESENCE_PULSE: /* waiting MSP before sending PRESENCE_PULSE : 6µs */
      if (time_in_state > ONEWIRE_MSP_MIN && DS2411_WIRE_STATE == ONEWIRE_INPUT_HIGH)
	{
	  DS2411_RESET_STATE = ONEWIRE_PRESENCE_PULSE_END;
	  DS2411_TIME_IN     = current_time;
	  DS2411_READ_VALID  = 1;
	  DS2411_READ_TRANS  = ONEWIRE_TRANS_LOW;
	  DS2411_WIRE_STATE  = ONEWIRE_INPUT_LOW;
	  HW_DMSG_DSH("ds2411: GO PRESENCE_PULSE, wire low [+%"PRId64"]\n",time_in_state);
	}
      else { RESET_ON_BAD_WIRE_STATE(dev,ONEWIRE_INPUT_HIGH,time_in_state); } 
      break;

    case ONEWIRE_PRESENCE_PULSE_END:
      if (time_in_state > ONEWIRE_PDL_MAX && DS2411_WIRE_STATE == ONEWIRE_INPUT_LOW)
	{
	  DS2411_RESET_STATE = ONEWIRE_PRESENCE_PULSE_REC;
	  DS2411_TIME_IN     = current_time;
	  DS2411_READ_VALID  = 1;
	  DS2411_READ_TRANS  = ONEWIRE_TRANS_HIGH;
	  DS2411_WIRE_STATE  = ONEWIRE_INPUT_HIGH;
	  HW_DMSG_DSH("ds2411: GO PRESENCE_PULSE_REC, wire high [+%"PRId64"]\n",time_in_state);
	}
      else { RESET_ON_BAD_WIRE_STATE(dev,ONEWIRE_INPUT_LOW,time_in_state); } 
      break;

    case ONEWIRE_PRESENCE_PULSE_REC:
      if (time_in_state > ONEWIRE_REC && DS2411_WIRE_STATE == ONEWIRE_INPUT_HIGH)
	{
	  DS2411_LVL0_STATE  = ONEWIRE_LVL0_READ_COMMAND;
	  DS2411_RCMD_STATE  = ONEWIRE_RCMD_INIT;
	  
	  DS2411_RESET_STATE = ONEWIRE_RESET_WAIT;

	  DS2411_TIME_IN     = current_time;
	  HW_DMSG_DSH("ds2411: GO COMMAND, wire high [+%"PRId64"]\n",time_in_state);
	}
      else { RESET_ON_BAD_WIRE_STATE(dev,ONEWIRE_INPUT_HIGH,time_in_state); } 
      break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

/* Handles onewire signals during Write0, Write1, keep track of RESET PULSE */
void update_onewire_write_signal(int dev, uint64_t current_time, uint64_t time_in_state)
{
  switch (DS2411_SIGNAL_STATE)
    {
    case ONEWIRE_WAITING_LOW:
      if (DS2411_WRITE_VALID && (DS2411_WIRE_STATE == ONEWIRE_INPUT_LOW))
        { /* starting something */
          DS2411_TIME_IN      = current_time;
          DS2411_SIGNAL_STATE = ONEWIRE_WAITING_HIGH;
          HW_DMSG_DSH("ds2411: GO SIGNAL_WAITING_HIGH (start write) [%"PRId64"]\n",
		      MACHINE_TIME_GET_NANO());
        }
      break;
      
    case ONEWIRE_WAITING_HIGH:
      if (DS2411_WRITE_VALID && (DS2411_WIRE_STATE == ONEWIRE_INPUT_HIGH))
        {
           if (time_in_state >= ONEWIRE_W1L_MIN && time_in_state <= ONEWIRE_W1L_MAX)
             { /* write 1 or read */
                DS2411_TIME_IN      = current_time;
                DS2411_SIGNAL_STATE = ONEWIRE_WRITE_ENDSLOT;
		DS2411_WTIME_REMAIN = ONEWIRE_SLOT_MAX - time_in_state;
		DS2411_WBIT         = 1;
                HW_DMSG_DSH("ds2411: GO WRITE_ENDSLOT (1) [+%" PRId64 "]\n",time_in_state);
              }
            else if (time_in_state >= ONEWIRE_W0L_MIN && time_in_state <= ONEWIRE_W0L_MAX)
              { /* write 0 */
                DS2411_TIME_IN      = current_time;
                DS2411_SIGNAL_STATE = ONEWIRE_WRITE_ENDSLOT;
		DS2411_WTIME_REMAIN = ONEWIRE_SLOT_MAX - time_in_state;
		DS2411_WBIT         = 0;
                HW_DMSG_DSH("ds2411: GO WRITE_ENDSLOT (0) [+%" PRId64 "]\n",time_in_state);
              }
            else if (time_in_state >= ONEWIRE_RSTL_MIN && time_in_state <= ONEWIRE_RSTL_MAX)
              { /* we have reset request */
                DS2411_TIME_IN      = current_time;
                DS2411_SIGNAL_STATE = ONEWIRE_WAITING_LOW;
                DS2411_LVL0_STATE   = ONEWIRE_LVL0_RESET;
                DS2411_RESET_STATE  = ONEWIRE_PRESENCE_PULSE;
                HW_DMSG_DSH("ds2411: GO PRESENCE_PULSE (during write) [+%" PRId64 "]\n",time_in_state);
              }
            else
              {
                GO_WAIT_RESET(dev);
                HW_DMSG_DSH("ds2411: ** HIGH at wrong time during WRITE [+%" PRId64 "]\n",time_in_state);
              }
         }
      break;

    case ONEWIRE_WRITE_ENDSLOT: /* wait write end slot */
      if (time_in_state >= DS2411_WTIME_REMAIN)
	{
	  DS2411_TIME_IN      = current_time;
	  DS2411_BUF         |= (DS2411_WBIT << DS2411_BUF_BITNUM);
	  DS2411_BUF_BITNUM  += 1;
	  DS2411_SIGNAL_STATE = ONEWIRE_WAITING_LOW;
	  HW_DMSG_DS2411("ds2411:    ** WRITE %d ** [0x%02x,%d]\n",DS2411_WBIT,DS2411_BUF,DS2411_BUF_BITNUM);
	  HW_DMSG_DSH("ds2411: GO WAITING_LOW [%" PRId64 "]\n",time_in_state);
	}
      break;

    case ONEWIRE_READ_VAL:
    case ONEWIRE_READ_HOLD:
    case ONEWIRE_READ_REC:
      ERROR("ds2411: ERROR, should no be in ONEWIRE_READ state\n");
      break;
  }
}

/***************************************************/
/***************************************************/
/***************************************************/

/* Handles onewire state during READ COMMAND (8 bits) */
static void update_onewire_read_command_state(int dev, uint64_t current_time, uint64_t time_in_state)
{
  switch (DS2411_RCMD_STATE)
    {
    case ONEWIRE_RCMD_INIT:
      DS2411_BUF          = 0;
      DS2411_BUF_BITNUM   = 0;
      DS2411_RCMD_STATE   = ONEWIRE_RCMD_READ;
      DS2411_SIGNAL_STATE = ONEWIRE_WAITING_LOW;
      /* fall through */
    case ONEWIRE_RCMD_READ: 
      
      update_onewire_write_signal(dev,current_time,time_in_state);

      if (DS2411_BUF_BITNUM == 8) /* the command is complete */
	{
	  DS2411_COMMAND    = DS2411_BUF;
	  DS2411_BUF_BITNUM = 0;
	  DS2411_RCMD_STATE = ONEWIRE_RCMD_INIT;
	  switch (DS2411_BUF)
	    {
	    case ONEWIRE_READ_ROM:          /* 0x33 */
	      DS2411_LVL0_STATE     = ONEWIRE_LVL0_DEVICE;
	      DS2411_READROM_STATE  = DS2411_READROM_INIT;
	      HW_DMSG_DS2411("ds2411: *****************\n");
	      HW_DMSG_DS2411("ds2411: READ_ROM received\n");
	      HW_DMSG_DS2411("ds2411: *****************\n");
	      break;
	    case ONEWIRE_SEARCH_ROM:        /* 0xF0 */
	      ERROR("ds2411: **************************\n");
	      ERROR("ds2411: Search ROM not implemented\n");
	      ERROR("ds2411: **************************\n");
	      GO_WAIT_RESET(dev);
	      break;
	    case ONEWIRE_OVERDRIVE_SKIP_ROM: /* 3C */
	      ERROR("ds2411: ****************************\n");
	      ERROR("ds2411: Overdrive mode not supported\n");
	      ERROR("ds2411: ****************************\n");
	      GO_WAIT_RESET(dev);
	      break;
	    default:
	      ERROR("ds2411: ****************************\n");
	      ERROR("ds2411: Unknown command 0x%02x      \n",DS2411_BUF);
	      ERROR("ds2411: ****************************\n");
	      GO_WAIT_RESET(dev);
	      break;
	    }
	}
      break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

/* Handles onewire signals during READ, keep track of RESET pulse */
void update_onewire_read_signal(int dev, uint64_t current_time, uint64_t time_in_state)
{
  switch (DS2411_SIGNAL_STATE)
    {
    case ONEWIRE_WAITING_LOW:
      if (DS2411_WRITE_VALID && (DS2411_WIRE_STATE == ONEWIRE_INPUT_LOW))
        { /* starting something */
          DS2411_TIME_IN      = current_time;
          DS2411_SIGNAL_STATE = ONEWIRE_WAITING_HIGH;
          HW_DMSG_DSH("ds2411: GO SIGNAL_WAITING_HIGH (start read)\n");
        }
      break;
      
    case ONEWIRE_WAITING_HIGH:
      if (DS2411_WRITE_VALID && (DS2411_WIRE_STATE == ONEWIRE_INPUT_HIGH))
        {
           if (time_in_state >= ONEWIRE_RL_MIN && time_in_state <= ONEWIRE_RL_MAX)
             { /* write 1 or read */
                DS2411_TIME_IN      = current_time;
                DS2411_SIGNAL_STATE = ONEWIRE_READ_VAL;
		/* DS2411_WTIME_REMAIN = ONEWIRE_RL_MAX - time_in_state; */
		DS2411_WTIME_REMAIN = 0; 
		DS2411_WBIT         = 1;
                HW_DMSG_DSH("ds2411: GO READ_VAL, waiting end of RL time [+%"PRId64"]\n",time_in_state);
              }
            else if (time_in_state >= ONEWIRE_RSTL_MIN && time_in_state <= ONEWIRE_RSTL_MAX)
              { /* we have reset request */
                DS2411_TIME_IN      = current_time;
                DS2411_SIGNAL_STATE = ONEWIRE_WAITING_LOW;
                DS2411_LVL0_STATE   = ONEWIRE_LVL0_RESET;
                DS2411_RESET_STATE  = ONEWIRE_PRESENCE_PULSE;
                HW_DMSG_DSH("ds2411: GO PRESENCE_PULSE (during read) [+%"PRId64"]\n",time_in_state);
              }
            else
              {
                GO_WAIT_RESET(dev);
                HW_DMSG_DS2411("ds2411: ** HIGH at wrong time during READ [+%"PRId64"]\n",time_in_state);
              }
         }
      break;

    case ONEWIRE_READ_VAL: /* End of RL */
      if (time_in_state >= DS2411_WTIME_REMAIN) /* ok, now we write */
	{
	  DS2411_TIME_IN      = current_time;
	  DS2411_SIGNAL_STATE = ONEWIRE_READ_HOLD;
	  DS2411_WBIT         = (DS2411_BUF & (1 << DS2411_BUF_BITNUM)) && 1;
	  if (DS2411_WBIT == 0) /* LSB first */
	    {
	      DS2411_READ_VALID = 1;
	      DS2411_READ_TRANS = ONEWIRE_TRANS_LOW;
	      DS2411_WIRE_STATE = ONEWIRE_INPUT_LOW;
	    }
	  else
	    {
	      DS2411_READ_VALID = 1;
	      DS2411_READ_TRANS = ONEWIRE_TRANS_HIGH;
	      DS2411_WIRE_STATE = ONEWIRE_INPUT_HIGH;
	    }
	  HW_DMSG_DSH("ds2411: GO READ_HOLD, value on wire is %s [+%"PRId64"]\n",
		      DS2411_WBIT ? "HIGH": "LOW" ,time_in_state);
	}
      else if (DS2411_WRITE_VALID)
	{
	  ERROR("ds2411: MCU should not change value while end of RL [+%"PRId64"]\n",time_in_state);
	  GO_WAIT_RESET(dev);
	}
      break;

    case ONEWIRE_READ_HOLD:
      if (time_in_state >= ONEWIRE_MSR_MAX)
	{
	  if (DS2411_WBIT == 0 && DS2411_WIRE_STATE == ONEWIRE_INPUT_LOW)
	    {
	      DS2411_READ_VALID = 1;
	      DS2411_READ_TRANS = ONEWIRE_TRANS_HIGH;
	      DS2411_WIRE_STATE = ONEWIRE_INPUT_HIGH;
	    }
	  else if (DS2411_WBIT == 1 && DS2411_WIRE_STATE == ONEWIRE_INPUT_HIGH)
	    {
	      /* so far, so good */
	    }
	  else
	    {
	      ERROR("ds2411: WIRE is not in good state at end of READ_HOLD\n");
	      GO_WAIT_RESET(dev);
	    }
	  DS2411_SIGNAL_STATE = ONEWIRE_READ_REC;
	  HW_DMSG_DSH("ds2411: GO READ_REC [+%"PRId64"]\n",time_in_state);
	}
      else if (DS2411_WRITE_VALID && (DS2411_WIRE_STATE != DS2411_WBIT))
	{
	  ERROR("ds2411: MCU should not change value while MRS [+%"PRId64"]\n",time_in_state);
	  GO_WAIT_RESET(dev);
	}
      break;

    case ONEWIRE_READ_REC:
      if (time_in_state >= ONEWIRE_REC)
	{
	  HW_DMSG_DS2411("ds2411:    ** READ %d done ** [0x%02x,%d]\n",DS2411_WBIT,DS2411_BUF,DS2411_BUF_BITNUM);
	  DS2411_BUF_BITNUM  += 1;
	  DS2411_SIGNAL_STATE = ONEWIRE_WAITING_LOW;
	}
      break;

    case ONEWIRE_WRITE_ENDSLOT: /* wait write end slot */
      ERROR("ds2411: ERROR, should no be in ONEWIRE_WRITE state\n");
      GO_WAIT_RESET(dev);
      break;
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/*** DS2411 specific protocol part ********************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

static void update_ds2411_state_readrom(int dev, uint64_t current_time, uint64_t time_in_state)
{
  switch (DS2411_READROM_STATE)
    {
    case DS2411_READROM_INIT:
      /* ds2411 serial state */
      DS2411_READROM_BYTENUM = 0;
      DS2411_READROM_STATE   = DS2411_READROM_READ;
      /* read state */
      DS2411_BUF             = DS2411_ID.raw[0];
      DS2411_BUF_BITNUM      = 0;
      DS2411_SIGNAL_STATE    = ONEWIRE_WAITING_LOW;
      HW_DMSG_DSTATE("ds2411: GO READROM_READ : 0x%02x (%d)\n",DS2411_BUF,DS2411_READROM_BYTENUM);
      HW_DMSG_SER("ds2411:serial: sending 0x%02x\n",DS2411_BUF);
      /* fall through */

    case DS2411_READROM_READ:

      update_onewire_read_signal(dev,current_time,time_in_state);
      /*
       * Family : 1 byte
       * Serial : 6 bytes
       * CRC    : 1 byte
       */
      if (DS2411_BUF_BITNUM == 8) /* byte has been read */
	{
	  if (DS2411_READROM_BYTENUM < 7) /* still something to read */
	    {
	      DS2411_READROM_BYTENUM += 1;
	      DS2411_BUF              = DS2411_ID.raw[DS2411_READROM_BYTENUM];
	      DS2411_BUF_BITNUM       = 0;
	      DS2411_SIGNAL_STATE     = ONEWIRE_WAITING_LOW;
	      HW_DMSG_SER("ds2411:serial: sending 0x%02x\n",DS2411_BUF);
	      HW_DMSG_DSTATE("ds2411: READ ROM next byte : 0x%02x (%d)\n",
			     DS2411_BUF,DS2411_READROM_BYTENUM);
	    }
	  else
	    {
	      DS2411_READROM_BYTENUM  = 0;
	      HW_DMSG_DSTATE("ds2411: READ ROM finished\n");
	      GO_WAIT_RESET(dev);
	    }
	}
      break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void update_ds2411_state(int dev, uint64_t current_time, uint64_t time_in_state)
{
  switch (DS2411_COMMAND)
    {
    case ONEWIRE_READ_ROM:
      update_ds2411_state_readrom(dev,current_time,time_in_state);
      break;
    case ONEWIRE_SEARCH_ROM:
      ERROR("ds2411: **************************\n");
      ERROR("ds2411: Search ROM not implemented\n");
      ERROR("ds2411: **************************\n");
      GO_WAIT_RESET(dev);
      break;
    default:
      ERROR("ds2411: ****************************\n");
      ERROR("ds2411: Unknown command 0x%02x      \n",DS2411_BUF);
      ERROR("ds2411: ****************************\n");
      GO_WAIT_RESET(dev);
      break;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds2411_dummy_update(int UNUSED dev)
{
  return 0;
}

int ds2411_update(int dev)
{
  uint64_t current_time;
  uint64_t time_in_state;

  current_time  = MACHINE_TIME_GET_NANO();
  time_in_state = current_time - DS2411_TIME_IN;

  /* stay in WAIT_RESET state */
  if (DS2411_WRITE_VALID == 0)
    {
      if (DS2411_LVL0_STATE == ONEWIRE_LVL0_RESET && 
	  DS2411_RESET_STATE == ONEWIRE_RESET_WAIT)
	{
	  return 0;
	}

      if (DS2411_WIRE_STATE == ONEWIRE_INPUT_LOW && 
	  current_time >= (DS2411_TIME_IN + ONEWIRE_RSTL_MAX))
	{
	  GO_WAIT_RESET(dev);
	  HW_DMSG_DS2411("ds2411:    ====== time out, going reset [+%"PRId64"] ======= \n",time_in_state);
	  return 0;
	}
    }

  switch (DS2411_LVL0_STATE)
    {
      /********************************/
      /* RESET_PULSE / PRESENCE_PULSE */
      /********************************/
    case ONEWIRE_LVL0_RESET:
      update_onewire_reset_state(dev,current_time,time_in_state);
      break;

      /********************************/
      /* READ_COMMAND                 */
      /********************************/
    case ONEWIRE_LVL0_READ_COMMAND:
      update_onewire_read_command_state(dev,current_time,time_in_state);
      break;
      
      /********************************/
      /* DEVICE SPECIFIC              */
      /********************************/
    case ONEWIRE_LVL0_DEVICE:
      update_ds2411_state(dev,current_time,time_in_state);
      break;
    }

  DS2411_WRITE_VALID = 0;
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int  ds2411_ui_draw     (int UNUSED dev)                 
{ 
  return 0;       
}

void ds2411_ui_get_size (int UNUSED dev, int *w, int *h) 
{ 
  *w = 0; *h = 0; 
}

void ds2411_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y) 
{
}

void ds2411_ui_get_pos  (int UNUSED dev, int *x, int *y) 
{ 
  *x = 0; *y = 0; 
}

/***************************************************/
/***************************************************/
/***************************************************/

int
ds2411_device_create(int dev, char *serial)
{
  machine.device[dev].delete       = ds2411_delete;
  machine.device[dev].reset        = ds2411_reset;

  machine.device[dev].power_up     = ds2411_power_up;
  machine.device[dev].power_down   = ds2411_power_down;

  machine.device[dev].read         = ds2411_read;
  machine.device[dev].write        = ds2411_write;

  machine.device[dev].update       = ds2411_update;

  machine.device[dev].ui_draw      = ds2411_ui_draw;
  machine.device[dev].ui_get_size  = ds2411_ui_get_size;
  machine.device[dev].ui_set_pos   = ds2411_ui_set_pos;
  machine.device[dev].ui_get_pos   = ds2411_ui_get_pos;

  machine.device[dev].state_size   = ds2411_device_size();
  machine.device[dev].name         = "ds2411 serial number";

  DS2411_ID = ds2411_str_to_id(serial);

  tracer_event_add_id(TRACER_DS2411,"ds2411", 32);

  return 0;
}
  
/***************************************************/
/***************************************************/
/***************************************************/
