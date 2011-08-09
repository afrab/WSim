
/**
 *  \file   ds1722_dev.c
 *  \brief  Dallas Maxim DS1722 digital thermometer
 *  \author Antoine Fraboulet
 *  \date   2011
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/ds1722/ds1722_dev.h"
#include "src/options.h"

/***************************************************/
/***************************************************/

tracer_id_t TRACER_DS1722_STATE;
tracer_id_t TRACER_DS1722_STROBE;

/***************************************************/
/***************************************************/

#define DS1722_DEBUG    1

#if DS1722_DEBUG != 0
#    define DS1722_DBG(x...) HW_DMSG_DEV(DS1722NAME ":" x)
#else
#    define DS1722_DBG(x...) do { } while (0)
#endif

#define DS1722NAME      "ds1722"

/***************************************************/
/** DS1722 internal data ***************************/
/***************************************************/

enum ds1722_state_t {
  DS1722_STATE_POWERDOWN  = 0,
  DS1722_STATE_ONESHOT    = 1,
  DS1722_STATE_CONTINUOUS = 2
};

enum ds1722_command_t {
  DS1722_CMD_NONE         = 0,
  DS1722_CMD_READ_CONF    = 1,
  DS1722_CMD_READ_LSB     = 2,
  DS1722_CMD_READ_MSB     = 3,
  DS1722_CMD_WRITE_CONF   = 4
};

#define READ_DUMMY 0

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed))  ds1722_conf_t {
  uint8_t
    fixed1:1,
    fuxed2:1,
    fuxed3:1,
    oneshot:1,
    res:3,
    sd:1;
};
#else
struct __attribute__ ((packed)) ds1722_conf_t {
  uint8_t
    sd:1,
    res:3,
    oneshot:1,
    fixed3:1,
    fixed2:1,
    fixed1:1;
};
#endif

#define DS1722_ADDR_READ_CONF         0x00
#define DS1722_ADDR_READ_LSB          0x01
#define DS1722_ADDR_READ_MSB          0x02
#define DS1722_ADDR_WRITE_CONF        0x80

struct ds1722_t 
{
  uint8_t    state;             /* device state           */
  uint8_t    cs;                /* chip select            */
  uint8_t    sermode;           /* SPI / 3-wire           */
  uint8_t    resolution;        /*                        */

  uint8_t    readok;
  uint8_t    readval;
  uint8_t    command;

  union {
    struct ds1722_conf_t b;     /* configuration register */
    uint8_t              s;
  } config;

  uint16_t             thm;     /* 01h LSB, 02h MSB       */

  uint8_t    sample_start;
  wsimtime_t sample_timer;      /* end of current conversion */
};

#define DS1722_DATA         ((struct ds1722_t*)(machine.device[dev].data))
#define DS1722_STATE        (DS1722_DATA->state)
#define DS1722_CS           (DS1722_DATA->cs)
#define DS1722_SERMODE      (DS1722_DATA->sermode)
#define DS1722_RES          (DS1722_DATA->resolution)
#define DS1722_CONF         (DS1722_DATA->config)
#define DS1722_THM          (DS1722_DATA->thm)

#define DS1722_SHUTDOWN_BIT (DS1722_CONF.b.sd)
#define DS1722_ONESHOT      (DS1722_CONF.b.oneshot)

#define DS1722_THM_DEFAULT  0x1910 /* +25.0625 °C */
#define DS1722_THM_LSB      (DS1722_THM & 0xf0)
#define DS1722_THM_MSB      ((DS1722_THM >> 8) & 0xff)

/*
 * 8-bit conversions     67.5 ms
 * 9-bit conversions    125.0 ms
 * 10-bit conversions   250.0 ms
 * 11-bit conversions   500.0 ms
 * 12-bit conversions  1000.0 ms
 */

#define MICRO *1000       /* scale to ns */
#define MILLI *1000*1000

wsimtime_t ds1722_conversion_timings[] = {
  67500 MICRO,
  125   MILLI,
  250   MILLI,
  500   MILLI,
  1000  MILLI
};

/***************************************************/
/** Flash external entry points ********************/
/***************************************************/

int  ds1722_reset       (int dev);
int  ds1722_delete      (int dev);
int  ds1722_power_up    (int dev);
int  ds1722_power_down  (int dev);
void ds1722_read        (int dev, uint32_t *mask, uint32_t *value);
void ds1722_write       (int dev, uint32_t  mask, uint32_t  value);
int  ds1722_update      (int dev);
int  ds1722_ui_draw     (int dev);
void ds1722_ui_get_size (int dev, int *w, int *h);
void ds1722_ui_set_pos  (int dev, int  x, int  y);
void ds1722_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

#define MAXNAME 1024

static struct moption_t ds1722_opt1 = {
  .longname    = "ds1722_opt1",
  .type        = required_argument,
  .helpstring  = "ds1722_opt1",
  .value       = NULL
};

static struct moption_t ds1722_opt2 = {
  .longname    = "ds1722_opt2",
  .type        = required_argument,
  .helpstring  = "ds1722_opt2",
  .value       = NULL
};


int ds1722_add_options(int UNUSED dev_num, int dev_id, const char UNUSED *dev_name)
{
  if (dev_id >= 1)
    {
      ERROR("ds1722: too much devices, please rewrite option handling\n");
      return -1;
    }

  options_add( &ds1722_opt1 );
  options_add( &ds1722_opt2 );
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds1722_device_size()
{
  return sizeof(struct ds1722_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds1722_device_create(int dev, int UNUSED id)
{
  machine.device[dev].reset         = ds1722_reset;
  machine.device[dev].delete        = ds1722_delete;
  machine.device[dev].power_up      = ds1722_power_up;
  machine.device[dev].power_down    = ds1722_power_down;

  machine.device[dev].read          = ds1722_read;
  machine.device[dev].write         = ds1722_write;
  machine.device[dev].update        = ds1722_update;

  machine.device[dev].ui_draw       = ds1722_ui_draw;
  machine.device[dev].ui_get_size   = ds1722_ui_get_size;
  machine.device[dev].ui_set_pos    = ds1722_ui_set_pos;

  machine.device[dev].state_size    = ds1722_device_size();
  machine.device[dev].name          = DS1722NAME " digital thermometer";

  TRACER_DS1722_STATE  = tracer_event_add_id(8, "state"    , DS1722NAME);
  TRACER_DS1722_STROBE = tracer_event_add_id(8, "function" , DS1722NAME);

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

static int ds1722_update_res(int dev)
{
  uint8_t cnf  = DS1722_CONF.s;
  uint8_t bits = (cnf >> 1) & 0x7; 
  uint8_t res  = 8 + bits;
  if (res > 12)
    {
      res = 12;
    }
  return res;
}

/***************************************************/
/***************************************************/
/***************************************************/

uint16_t ds1722_get_new_sample(int UNUSED dev)
{
  /* TODO: 
   *   - get new samples from a file or from wsnet
   *   - take care of sampling resolution 
   */
  return DS1722_THM_DEFAULT; 
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds1722_reset(int dev)
{
  DS1722_DBG("reset\n");
  DS1722_STATE    = DS1722_STATE_POWERDOWN;       /* shutdown       */
  DS1722_DATA->cs = 0;
  DS1722_CONF.s   = 0x03;                         /* R0=1, SD=1     */
  DS1722_RES      = ds1722_update_res(dev);       /* 9 bits default */
  DS1722_THM      = ds1722_get_new_sample(dev);   
  
  DS1722_DATA->command = DS1722_CMD_NONE;
  DS1722_DATA->readok  = 0;
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds1722_delete(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds1722_power_up(int UNUSED dev)
{
  DS1722_DBG("power up\n");
  return 0;
}

int ds1722_power_down(int UNUSED dev)
{
  DS1722_DBG("power down\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void ds1722_read(int dev, uint32_t *mask, uint32_t *value)
{
  if (DS1722_CS && DS1722_DATA->readok)
    {
      *mask  = DS1722_D_MASK;
      *value = DS1722_DATA->readval;
      DS1722_DATA->readok = 0;
      DS1722_DBG("read: value 0x%02x\n",*value);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void ds1722_write_conf(int dev, uint8_t data)
{
  union {
    struct ds1722_conf_t b;
    uint8_t              s;
  } conf;

  conf.s = data;
  DS1722_DBG("write:conf: new config 0x%02x\n", conf.s);

  if (conf.b.sd != DS1722_SHUTDOWN_BIT)
    {
      DS1722_DBG("write:conf:    SD    = %d\n",conf.b.sd);
      if (conf.b.sd == 0)
	{
	  DS1722_DBG("write:conf:    SD start continuous sampling\n");
	  DS1722_DATA->sample_start = 1;
	}
    }

  if (conf.b.oneshot != DS1722_ONESHOT)
    {
      DS1722_DBG("write:conf:    1shot = %d\n",conf.b.oneshot);
      if (conf.b.sd == 0 && conf.b.oneshot == 1) /* ignore */
	{
	  DS1722_DBG("write:conf:    1shot ignored (sd == 0)\n");
	}
      if (conf.b.sd == 1 && conf.b.oneshot == 1) /* start */
	{
	  DS1722_DBG("write:conf:    1shot started\n");
	  DS1722_DATA->sample_start = 1;
	}
    }

  if (conf.b.res != DS1722_CONF.b.res)
    {
      DS1722_DBG("write:conf:    bits  = 0x%x\n",conf.b.res);
    }

  DS1722_CONF.s = data;
  DS1722_RES    = ds1722_update_res(dev);
  DS1722_DBG("write:conf:    res   = %d bits\n",DS1722_RES);
}

/***************************************************/
/***************************************************/
/***************************************************/

static void WARN_MODEL_INCOMPLETE()
{
  static int done = 0;
  if (!done)
    {
      WARNING("==============================================\n");
      WARNING("=== DS1722 model is incomplete, only basic ===\n");
      WARNING("=== sampling is available. Themperature is ===\n");
      WARNING("=== fixed at this time.                    ===\n");
      WARNING("===                                        ===\n");
      WARNING("=== If you need more properties, please    ===\n");
      WARNING("=== send a request to wsim-dev             ===\n");
      WARNING("==============================================\n");
      done = 1;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void ds1722_write(int dev, uint32_t mask, uint32_t value)
{
  /* DS1722_DBG("write: mask=0x%04x, val=0x%04x cs=%d\n",mask,value,DS1722_CS); */

  /*************/
  /** CS      **/
  /*************/
  if ((mask & DS1722_CS_MASK))
    {
      uint32_t cs = (value & DS1722_CS_MASK);
      if (cs)
	{
	  if (DS1722_CS != cs)
	    {
	      DS1722_DBG("write: chip select CS on\n");
	      
	      WARN_MODEL_INCOMPLETE();
	    }
	}
      else
	{
	  if (DS1722_CS != cs)
	    {
	      DS1722_DBG("write: chip select CS off\n");
	    }
	  DS1722_DATA->command = DS1722_CMD_NONE;
	  DS1722_DATA->readok  = 0;
	}
      DS1722_DATA->cs = (cs != 0);
    }

  /*************/
  /** SERMODE **/
  /*************/
  if ((mask & DS1722_SER_MASK))
    {
      uint32_t sermode = (value & DS1722_SER_MASK);
      if (sermode) /* SPI mode */
	{
	  if (DS1722_SERMODE != sermode)
	    {
	      DS1722_DBG("write: SPI mode turned on\n");
	    }
	}
      else         /* 3-wire mode */
	{
	  if (DS1722_SERMODE != sermode)
	    {
	      WARNING(DS1722NAME ":write: 3-wire mode not supported\n");
	      DS1722_DBG("write: SPI mode turned off :: 3-wire mode not supported\n");
	    }
	}
      DS1722_SERMODE = (sermode != 0);
    }

  /*************/
  /** DATA    **/
  /*************/
  if ((mask & DS1722_D_MASK) && DS1722_CS)
    {
      uint8_t data = (value & DS1722_D_MASK);
      
      if (DS1722_DATA->command == DS1722_CMD_NONE)
	{
	  switch (data)
	    {
	    case DS1722_ADDR_READ_CONF:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = READ_DUMMY;
	      DS1722_DATA->command = DS1722_CMD_READ_CONF;
	      DS1722_DBG("write:data 0x%02x :: command READ_CONF\n",data);
	      break;
	    case DS1722_ADDR_READ_LSB:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = READ_DUMMY;
	      DS1722_DATA->command = DS1722_CMD_READ_LSB;
	      DS1722_DBG("write:data 0x%02x :: command READ_LSB\n",data);
	      break;
	    case DS1722_ADDR_READ_MSB:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = READ_DUMMY;
	      DS1722_DATA->command = DS1722_CMD_READ_MSB;
	      DS1722_DBG("write:data 0x%02x :: command READ_MSB\n",data);
	      break;
	    case DS1722_ADDR_WRITE_CONF:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = READ_DUMMY;
	      DS1722_DATA->command = DS1722_CMD_WRITE_CONF;
	      DS1722_DBG("write:data 0x%02x :: command WRITE_CONF\n",data);
	      break;
	    default:
	      WARNING("write:data 0x%02x address unknown\n",data);
	      break;
	    }
	}
      else /* command */
	{
	  switch (DS1722_DATA->command)
	    {
	    case DS1722_CMD_NONE:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = READ_DUMMY;
	      ERROR(DS1722NAME ":write:command = NONE, value 0x%02x :: should not come here\n");
	      break;
	    case DS1722_CMD_READ_CONF:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = DS1722_CONF.s;
	      DS1722_DBG("write:data 0x%02x :: command READ_CONF :: value 0x%02x\n",data,DS1722_DATA->readval);
	      break;
	    case DS1722_CMD_READ_LSB:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = DS1722_THM_LSB;
	      DS1722_DBG("write:data 0x%02x :: command READ_LSB :: value 0x%02x\n",data,DS1722_DATA->readval);
	      break;
	    case DS1722_CMD_READ_MSB:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = DS1722_THM_MSB;
	      DS1722_DBG("write:data 0x%02x :: command READ_MSB :: value 0x%02x\n",data,DS1722_DATA->readval);
	      break;
	    case DS1722_CMD_WRITE_CONF:
	      DS1722_DATA->readok  = 1;
	      DS1722_DATA->readval = READ_DUMMY;
	      ds1722_write_conf(dev,data);
	      break;
	    default:
	      ERROR("write: unkonwn command\n");
	      break;
	    }
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

wsimtime_t ds1722_get_sampling_time(int dev)
{
  int s = DS1722_RES - 8;
  return ds1722_conversion_timings[s];
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds1722_update(int UNUSED dev)
{
  if (DS1722_DATA->sample_start)
    {
      wsimtime_t delay = ds1722_get_sampling_time(dev);
      DS1722_DATA->sample_timer = MACHINE_TIME_GET_NANO() + delay;
      DS1722_DATA->sample_start = 0;
      DS1722_DBG("update: start sampling, resolution %d bits, delay %d ms\n",
                 DS1722_RES, delay / (1000*1000));
    }

  if (MACHINE_TIME_GET_NANO() >= DS1722_DATA->sample_timer)
    {
      /******************/
      /* GET NEW SAMPLE */
      /******************/
      DS1722_THM = ds1722_get_new_sample(dev);

      if (DS1722_SHUTDOWN_BIT == 0) /* continuous sampling */
	{
	  DS1722_DATA->sample_start = 1;
	  DS1722_DBG("update: restart sampling, continuous mode\n");
	}

      if ((DS1722_SHUTDOWN_BIT == 0) && (DS1722_ONESHOT)) /* oneshot sampling */
	{
	  DS1722_ONESHOT = 0;
	}
    }

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ds1722_ui_draw      (int UNUSED dev)
{
  return 0;
}

void ds1722_ui_get_size (int UNUSED dev, int *w, int *h)
{
  *w = 0;
  *h = 0;
}

void ds1722_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y)
{
  
}

void ds1722_ui_get_pos  (int UNUSED dev, int *x, int *y)
{
  *x = 0;
  *y = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/
