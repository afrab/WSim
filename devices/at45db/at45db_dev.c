
/**
 *  \file   at45db_dev.c
 *  \brief  at45db flash memory module
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/at45db/at45db_dev.h"
#include "src/options.h"

#define DEBUG_ME_HARDER

#ifdef _DEBUG
#define HW_DMSG_AT45(x...) HW_DMSG(x)
#else
#define HW_DMSG_AT45(x...) do {} while(0)
#endif


enum at45_power_state_t {
  AT45_POWER_DEEP_DOWN    = 1,
  AT45_POWER_STANDBY      = 2,
  AT45_POWER_ACTIVE       = 3
};


enum at45_opcode_t {
  AT45_OP_NOP       = 0x00u,  /* nop                   */
  AT45_OP_WREN      = 0x06u,  /* write enable          */
  AT45_OP_WRDI      = 0x04u,  /* write disable         */
  AT45_OP_RDSR      = 0x05u,  /* read status register  */
  AT45_OP_WRSR      = 0x01u,  /* write status register */   
  AT45_OP_READ      = 0x03u,  /* read                  */
  AT45_OP_FAST_READ = 0x0Bu,  /* fast read             */
  AT45_OP_PP        = 0x02u,  /* page program          */
  AT45_OP_SE        = 0xd8u,  /* sector erase          */
  AT45_OP_BE        = 0xc7u,  /* bulk erase            */
  AT45_OP_DP        = 0xb9u,  /* deep power down       */
  AT45_OP_RES       = 0xabu   /* release from DP       */
};


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed))  at45_status_t {
  uint8_t
    srwd:1,
    unused1:1,
    unused2:1,
    bp2:1,
    bp1:1,
    bp0:1,
    wel:1,
    wip:1;
};
#else
struct __attribute__ ((packed)) at45_status_t {
  uint8_t
    wip:1,
    wel:1,
    bp0:1,
    bp1:1,
    bp2:1,
    unused2:1,
    unused1:1,
    srwd:1;
};
#endif

#if defined(DEBUG)
const char* str_cmd(int n)
{
  switch (n)
    {
    case AT45_OP_WREN:      return "WREN";
    case AT45_OP_WRDI:      return "WRDI";
    case AT45_OP_RDSR:      return "RDSR";
    case AT45_OP_WRSR:      return "WRSR";
    case AT45_OP_READ:      return "READ";
    case AT45_OP_FAST_READ: return "FAST READ";
    case AT45_OP_PP:        return "PAGE PROGRAM";
    case AT45_OP_SE:        return "SECTOR ERASE";
    case AT45_OP_BE:        return "BULK ERASE";
    case AT45_OP_DP:        return "DEEP POWER";
    case AT45_OP_RES:       return "RES";
    default:               return "UNKNOWN";
    }
  return "UNKNOWN";
}
#endif

/***************************************************/
/** Flash internal data ****************************/
/***************************************************/

/**
 * 8 MB of Flash Memory
 * Page Program (up to 256 Bytes) in 1.4ms (typical)
 * Sector Erase (512 Kbit) in 1s (typical)
 * Bulk Erase (8 Mbit) in 10s (typical)
 * 2.7 to 3.6V Single Supply Voltage
 * SPI Bus Compatible Serial Interface
 * 40MHz Clock Rate (maximum)
 * Deep Power-down Mode 1µA (typical)
 * Electronic Signature (13h)
 * Packages
 * ­ ECOPACK® (RoHS compliant)
 */

#define AT45_FLASH_SIZE           0x100000
#define AT45_SECTOR_MAX           16
#define AT45_SECTOR_SIZE          65536
#define AT45_PAGE_MAX             4096
#define AT45_PAGE_SIZE            264

#define AT45_MAX_COMMAND_DATA     100
#define AT45_ELECTRONIC_SIGNATURE 0x13

struct at45db_t 
{
  union {
    struct at45_status_t b;
    uint8_t             s;
  } status_register;

  uint8_t select_bit;           /* chip select   */
  uint8_t write_protect_bit;    /* write protect */

  uint8_t buffer1[AT45_PAGE_SIZE];
  uint8_t buffer2[AT45_PAGE_SIZE];

  union {
    uint8_t raw   [AT45_FLASH_SIZE];
    uint8_t sector[AT45_SECTOR_MAX][AT45_SECTOR_SIZE];
    uint8_t page  [AT45_PAGE_MAX][AT45_PAGE_SIZE];
  } mem;

  enum at45_power_state_t  power_mode;
  enum at45_opcode_t       command;

  int32_t                  command_needed_data;
  uint32_t                 command_pointer;

  uint8_t                  command_need_to_complete;
  uint8_t                  command_stored_data[AT45_MAX_COMMAND_DATA];
  uint8_t                  dummy_write_for_read;
  uint8_t                  command_read_byte;

  /* data just written */
  uint8_t data_buffer;
  uint8_t data_buffer_ok;

  /* data to be read */
  uint8_t data_ready;
  uint8_t data_val;
  
  /* busy timing */
  uint64_t end_of_busy_time;

  /* clock pin : unused */
  uint8_t clock;

  /* file names */
  char *file_init;
  char *file_dump;
};

#define AT45_DATA        ((struct at45db_t*)(machine.device[dev].data))
#define AT45_MEMRAW      (AT45_DATA->mem.raw   )
#define AT45_MEMSECTOR   (AT45_DATA->mem.sector)
#define AT45_MEMPAGE     (AT45_DATA->mem.page  )
#define AT45_INIT        (AT45_DATA->file_init )
#define AT45_DUMP        (AT45_DATA->file_dump )

/***************************************************/
/** Flash external entry points ********************/
/***************************************************/

int  at45db_reset       (int dev);
int  at45db_delete      (int dev);
int  at45db_power_up    (int dev);
int  at45db_power_down  (int dev);
void at45db_read        (int dev, uint32_t *mask, uint32_t *value);
void at45db_write       (int dev, uint32_t  mask, uint32_t  value);
int  at45db_update      (int dev);
int  at45db_ui_draw     (int dev);
void at45db_ui_get_size (int dev, int *w, int *h);
void at45db_ui_set_pos  (int dev, int  x, int  y);
void at45db_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

#define MAXNAME 1024

static struct moption_t flash_init_opt = {
  .longname    = "flash_init",
  .type        = required_argument,
  .helpstring  = "Flash init image",
  .value       = NULL
};

static struct moption_t flash_dump_opt = {
  .longname    = "flash_dump",
  .type        = required_argument,
  .helpstring  = "Flash dump image",
  .value       = NULL
};


int at45db_add_options(int UNUSED dev_num, int dev_id, const char UNUSED *dev_name)
{
  if (dev_id >= 1)
    {
      ERROR("at45db: too much devices, please rewrite option handling\n");
      return -1;
    }

  options_add( &flash_init_opt  );
  options_add( &flash_dump_opt  );
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_device_size()
{
  return sizeof(struct at45db_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_flash_load(int dev, const char *name)
{
  FILE *f;

  if ((f = fopen(name, "r")) == NULL)
    {
      return 1;
    }
  
  HW_DMSG_AT45("at45db: loading file %s into flash\n",name);

  if (fread(AT45_MEMRAW,1,AT45_FLASH_SIZE,f) != AT45_FLASH_SIZE)
    {
      HW_DMSG_AT45("at45db: error while loading file %s to flash\n",name);
      ERROR      ("at45db: error while loading file %s to flash\n",name);
      return 2;
    }

  HW_DMSG_AT45("at45db: loading file %s into flash -- ok\n",name);

  fclose(f);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_flash_dump(int dev, const char *name)
{
  FILE *f;

  if ((f = fopen(name, "r")) == NULL)
    {
      return 1;
    }
  
  HW_DMSG_AT45("at45db: saving file %s into flash\n",name);

  if (fwrite(AT45_MEMRAW,1,AT45_FLASH_SIZE,f) != AT45_FLASH_SIZE)
    {
      HW_DMSG_AT45("at45db: error while dumping flash to file %s\n",name);
      ERROR      ("at45db: error while dumping flash to file %s\n",name);
      return 2;
    }

  HW_DMSG_AT45("at45db: saving flash into file %s -- ok\n",name);

  fclose(f);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_device_create(int dev, int UNUSED id)
{
  machine.device[dev].reset         = at45db_reset;
  machine.device[dev].delete        = at45db_delete;
  machine.device[dev].power_up      = at45db_power_up;
  machine.device[dev].power_down    = at45db_power_down;

  machine.device[dev].read          = at45db_read;
  machine.device[dev].write         = at45db_write;
  machine.device[dev].update        = at45db_update;

  machine.device[dev].ui_draw       = at45db_ui_draw;
  machine.device[dev].ui_get_size   = at45db_ui_get_size;
  machine.device[dev].ui_set_pos    = at45db_ui_set_pos;

  machine.device[dev].state_size    = at45db_device_size();
  machine.device[dev].name          = "at45db flash memory";

  AT45_INIT = flash_init_opt.value;
  AT45_DUMP = flash_dump_opt.value;

  if (AT45_INIT == NULL || at45db_flash_load(dev, AT45_INIT))
    {
      HW_DMSG_AT45("at45db: flash memory init to 0xff\n");
      memset(AT45_MEMRAW,0xff,AT45_FLASH_SIZE);
    }

  tracer_event_add_id(TRACER_AT45DB, "at45db" , 64);

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_reset(int dev)
{
  HW_DMSG_AT45("at45db: flash reset\n");
  AT45_DATA->command                  = AT45_OP_NOP;
  AT45_DATA->command_need_to_complete = 0;
  AT45_DATA->status_register.s        = 0;
  AT45_DATA->power_mode               = AT45_POWER_STANDBY;

  tracer_event_record(TRACER_AT45DB, AT45_POWER_STANDBY);
  etracer_slot_event(ETRACER_PER_ID_AT45DB,
		     ETRACER_PER_EVT_MODE_CHANGED,
		     ETRACER_AT45DB_POWER_STANDBY, 0);

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_delete(int dev)
{
  if (AT45_DUMP)
    {
      at45db_flash_dump(dev,AT45_DUMP);
    }

  if (AT45_INIT)
    {
      free(AT45_INIT);
    }

  if (AT45_DUMP)
    {
      free(AT45_DUMP);
    }

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_error_dump_internal_state(int UNUSED dev)
{
#if defined(DEBUG_ME_HARDER)
  HW_DMSG_AT45("at45db: Power mode %d\n",AT45_DATA->power_mode);
  HW_DMSG_AT45("at45db: Command 0x%02x\n",AT45_DATA->command);
  HW_DMSG_AT45("at45db: need complete %d\n",AT45_DATA->command_need_to_complete);
  HW_DMSG_AT45("at45db: select %d write protect %d\n",
	      AT45_DATA->select_bit,AT45_DATA->write_protect_bit);
  HW_DMSG_AT45("at45db: status register wip  %d\n",AT45_DATA->status_register.b.wip); 
  HW_DMSG_AT45("at45db:                 wel  %d\n",AT45_DATA->status_register.b.wel); 
  HW_DMSG_AT45("at45db:                 bp0  %d\n",AT45_DATA->status_register.b.bp0);
  HW_DMSG_AT45("at45db:                 bp1  %d\n",AT45_DATA->status_register.b.bp1);
  HW_DMSG_AT45("at45db:                 bp2  %d\n",AT45_DATA->status_register.b.bp2);
  HW_DMSG_AT45("at45db:                 u2   %d\n",AT45_DATA->status_register.b.unused2);
  HW_DMSG_AT45("at45db:                 u1   %d\n",AT45_DATA->status_register.b.unused1);
  HW_DMSG_AT45("at45db:                 srwd %d\n",AT45_DATA->status_register.b.srwd);
#endif
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_power_up(int UNUSED dev)
{
  /* power up timing doc page 27 */
  HW_DMSG_AT45("at45db: flash power up\n");
  return 0;
}

int at45db_power_down(int UNUSED dev)
{
  HW_DMSG_AT45("at45db: flash power down\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

static void at45db_set_write_time(int dev, uint64_t nano)
{
  AT45_DATA->status_register.b.wip = 1;
  AT45_DATA->end_of_busy_time      = MACHINE_TIME_GET_NANO() + nano;
  HW_DMSG_AT45("at45db: flash busyflag delays %"PRIu64" cycles\n",AT45_DATA->end_of_busy_time);
}

/***************************************************/
/***************************************************/
/***************************************************/

#define NANO   (1)
#define MICRO  (1000)
#define MILLI  (1000 * 1000)
#define SECOND (1000 * 1000 * 1000)

#define AT45DB_Grade6

// device grade 6 : times in micro seconds
// 
#if defined(AT45DB_Grade6)
#  define Freq_max                   40
#  define TIME_WRITE_STATUS_REGISTER (5    * MILLI)
#  define TIME_PAGE_PROGRAM          (1400 * MICRO)
#  define TIME_SECTOR_ERASE          ( 1000000000ull) /*  1 s */
#  define TIME_BULK_ERASE            (10000000000ull) /* 10 s */
/*                                      aaabbbccc             */
#elif defined(AT45DB_Grade3)
#  define Freq_max                   25
#  define TIME_WRITE_STATUS_REGISTER (8    * MILLI)
#  define TIME_PAGE_PROGRAM          (1500 * MICRO)
#  define TIME_SECTOR_ERASE          ( 1000000000ull) /*  1 s */
#  define TIME_BULK_ERASE            (10000000000ull) /* 10 s */
#else
#  error "must define speed grade for Flash memory AT45DB"
#endif

/***************************************************/
/***************************************************/
/***************************************************/

static void at45db_erase_sector(int dev, uint32_t address)
{
  /*
    case 0: // unprotected 
    case 1: // sector 15 is protected
    case 2: // sectors 14 and 15 are protected
    case 3: // sectors 12 to  15 are protected
    case 4: // sectors  8 to  15 are protected
    case 5: // sectors 0 to 15 are protected (all)
    case 6: // sectors 0 to 15 are protected (all)
    case 7: // sectors 0 to 15 are protected (all)
  */

  int sector;
  int protbits;
  int protection_base[] = { 16, 15, 14, 12, 8, -1, -1, -1 }; 

  sector   = address / AT45_SECTOR_SIZE;
  protbits = AT45_DATA->status_register.b.bp1 << 2 |
             AT45_DATA->status_register.b.bp1 << 1 |
             AT45_DATA->status_register.b.bp0;

  at45db_set_write_time(dev,TIME_SECTOR_ERASE);
  HW_DMSG_AT45("at45db:  sector %d erase (address=0x%06x)\n",address / AT45_SECTOR_SIZE, address);

  if (sector >= protection_base[protbits])
    {
      ERROR      ("at45db: protection error : sector %d protected by bpx bits\n",sector);
      HW_DMSG_AT45("at45db: protection error : sector %d protected by bpx bits\n",sector);
    }

  memset(AT45_MEMSECTOR[address / AT45_SECTOR_SIZE],0xff,AT45_SECTOR_SIZE);
}

/***************************************************/
/***************************************************/
/***************************************************/

static void at45db_erase_bulk(int dev)
{
  int protbits;

  at45db_set_write_time(dev,TIME_BULK_ERASE);
  HW_DMSG_AT45("at45db:  bulk erase\n");

  protbits = AT45_DATA->status_register.b.bp1 << 2 |
             AT45_DATA->status_register.b.bp1 << 1 |
             AT45_DATA->status_register.b.bp0;
  if (protbits)
    {
      ERROR("at45db: protection error = bulk erase with bp0|bp1|bp2 > 0\n");
      HW_DMSG_AT45("at45db: protection error = bulk erase with bp0|bp1|bp2 > 0\n");
    }
  memset(AT45_MEMRAW,0xff,AT45_FLASH_SIZE);
}

/***************************************************/
/***************************************************/
/***************************************************/

static void at45db_write_status_register(int dev, uint8_t value)
{
  /* WRSR has no effect on b6, b5, b1 and b0 */
  AT45_DATA->status_register.s |= (~0x63 & value);
  at45db_set_write_time(dev,TIME_WRITE_STATUS_REGISTER);	      
}

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * read is done only on a junk write
 *
 */

void at45db_read(int dev, uint32_t *mask, uint32_t *value)
{
  if ((AT45_DATA->select_bit == 1) && (AT45_DATA->power_mode == AT45_POWER_ACTIVE))
    {

      if ((AT45_DATA->dummy_write_for_read == 1) && (AT45_DATA->command_read_byte == 0))
	{
	  AT45_DATA->command_read_byte    = 0;
	  AT45_DATA->dummy_write_for_read = 0;
	  switch (AT45_DATA->command)
	    {
	    case AT45_OP_RDSR:
	      *mask  = AT45DB_D;
	      *value = AT45_DATA->status_register.s & 0xff; 
	      HW_DMSG_AT45("at45db: read status register = 0x%02x\n", *value);
	      etracer_slot_event(ETRACER_PER_ID_AT45DB, ETRACER_PER_EVT_WRITE_COMMAND, 
				 ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
	      
	      break;
	      
	    case AT45_OP_READ:
	      /* we need to slow down READ to adjust to f_R output frequency    */
	      /* f_R is 40MHz to 25MHz, should be ok until we get newer devices */
	      if (AT45_DATA->command_needed_data < 0)
		{
		  *mask  = AT45DB_D;
		  *value = AT45_MEMRAW[AT45_DATA->command_pointer];
		  HW_DMSG_AT45("at45db: read [0x%06x] = 0x%02x\n",AT45_DATA->command_pointer, *value);
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		  
		  AT45_DATA->command_pointer ++;
		  if (AT45_DATA->command_pointer == AT45_FLASH_SIZE)
		    {
		      AT45_DATA->command_pointer = 0;
		    }
		}
	      else
		{
		  *mask  = AT45DB_D;
		  *value = 0;
		  HW_DMSG_AT45("at45db: read (dummy read, flash sends 0, needed %d)\n",
			      AT45_DATA->command_needed_data);
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      break;
	      
	    case AT45_OP_FAST_READ:
	      /* fast read is driven by the READ_DEV_TO_SPI clock speed */
	      /* limited to f_C, same as above on frequencies           */
	      if (AT45_DATA->command_needed_data < 0)
		{
		  *mask  = AT45DB_D;
		  *value = AT45_MEMRAW[AT45_DATA->command_pointer];
		  HW_DMSG_AT45("at45db: fast read [0x%06x] = 0x%02x\n",AT45_DATA->command_pointer, *value);
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		  AT45_DATA->command_pointer ++;
		  if (AT45_DATA->command_pointer == AT45_FLASH_SIZE)
		    {
		      AT45_DATA->command_pointer = 0;
		    }
		}
	      else
		{
		  *mask  = AT45DB_D;
		  *value = 0;
		  HW_DMSG_AT45("at45db: fast read (dummy read, flash sends 0, needed %d)\n",
			      AT45_DATA->command_needed_data);
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      break;
	      
	    case AT45_OP_RES:
	      if (AT45_DATA->command_needed_data < 0)
		{
		  *mask  = AT45DB_D;
		  *value = AT45_ELECTRONIC_SIGNATURE;
		  HW_DMSG_AT45("at45db: read electronic signature = 0x%x\n",*value);
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      else
		{
		  *mask  = AT45DB_D;
		  *value = 0;
		  HW_DMSG_AT45("at45db: read electronic signature (dummy read, flash sends 0, needed %d)\n",
			      AT45_DATA->command_needed_data);
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      break;
	      
	    default:
	      HW_DMSG_AT45("at45db: read dummy in write response\n");
	      *mask  = AT45DB_D;
	      *value = 0;
	      etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				 ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
	      break;
	    }
	} // if ((AT45_DATA->dummy_write_for_read == 1) && (AT45_DATA->command_read_byte == 0))
      else if (AT45_DATA->command_read_byte == 1)
	{
	  AT45_DATA->command_read_byte    = 0;
	  AT45_DATA->dummy_write_for_read = 0;
	  *mask  = AT45DB_D;
	  *value = 0;
	  HW_DMSG_AT45("at45db: read value that corresponds to command\n");
	  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
			     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
	}
      else 
	{
	  /* this is a hit for updates where CS and command are enabled */
	  *mask  = 0;
	  *value = 0;
	}
    }
  else //   if ((AT45_DATA->select_bit == 1) && (AT45_DATA->power_mode == AT45_POWER_ACTIVE))
    {
      *mask  = 0;
      *value = 0;
    }

  if (*mask != 0)
    {
      HW_DMSG_AT45("at45db:    read data [val=0x%02x,mask=0x%04x] \n", *value, *mask);
    }

}

/***************************************************/
/***************************************************/
/***************************************************/

#define CHECK_WIP                                                                       \
do {                                                                                    \
  if (AT45_DATA->status_register.b.wip == 1)                                             \
    {                                                                                   \
      ERROR      ("at45db: ===================================================\n");     \
      ERROR      ("at45db: == AT45DB Command issued while Write In Progress ==\n");     \
      ERROR      ("at45db: ===================================================\n");     \
      HW_DMSG_AT45("at45db: ===================================================\n");     \
      HW_DMSG_AT45("at45db: == AT45DB Command issued while Write In Progress ==\n");     \
      HW_DMSG_AT45("at45db: ===================================================\n");     \
    }                                                                                   \
} while(0)

#define CHECK_WEL                                                                       \
do {                                                                                    \
  if (AT45_DATA->status_register.b.wel == 0)                                             \
    {                                                                                   \
      ERROR      ("at45db: ======================================================\n");  \
      ERROR      ("at45db: == AT45DB Command issued without Write Enable (WEL) ==\n");  \
      ERROR      ("at45db: ======================================================\n");  \
      HW_DMSG_AT45("at45db: ======================================================\n");  \
      HW_DMSG_AT45("at45db: == AT45DB Command issued without Write Enable (WEL) ==\n");  \
      HW_DMSG_AT45("at45db: ======================================================\n");  \
    }                                                                                   \
} while(0)

#define CHECK_COMMAND_ARGS                                                              \
do {                                                                                    \
  if (AT45_DATA->command_needed_data > 0)                                                \
    {                                                                                   \
      ERROR      ("at45db: ===============================================\n");         \
      ERROR      ("at45db: == AT45DB Command terminated too early by CS ==\n");         \
      ERROR      ("at45db: ===============================================\n");         \
      HW_DMSG_AT45("at45db: ===============================================\n");         \
      HW_DMSG_AT45("at45db: == AT45DB Command terminated too early by CS ==\n");         \
      HW_DMSG_AT45("at45db: ===============================================\n");         \
    }                                                                                   \
} while(0)

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_write_spidata(int dev, uint32_t mask, uint32_t value)
{

  if (AT45_DATA->command_need_to_complete == 0)
    {
      AT45_DATA->data_buffer    = (value & AT45DB_D);
      AT45_DATA->data_buffer_ok = 1;
      AT45_DATA->dummy_write_for_read = 1;
      //HW_DMSG_AT45("at45db:    write data to flash 0x%02x\n",AT45_DATA->data_buffer);
      
      switch (AT45_DATA->power_mode)
	{
	case AT45_POWER_ACTIVE:
	  /*********************************************************/
	  /* stay active : take a look if we just have had a write */
	  /*   we start a new command by going out of NOP          */
	  /*********************************************************/
	  switch (AT45_DATA->command)
	    {
	    case AT45_OP_NOP:
	      /* ========== */
	      AT45_DATA->command_read_byte = 1;
	      AT45_DATA->command = AT45_DATA->data_buffer;
	      switch (AT45_DATA->data_buffer)
		{
		case AT45_OP_WREN:
		  HW_DMSG_AT45("at45db:    =====================\n");
		  HW_DMSG_AT45("at45db:    starting WREN command\n");
		  HW_DMSG_AT45("at45db:    =====================\n");
		  CHECK_WIP;
		  AT45_DATA->command_need_to_complete   = 1;
		  break;
		case AT45_OP_WRDI:
		  HW_DMSG_AT45("at45db:    =====================\n");
		  HW_DMSG_AT45("at45db:    starting WRDI command\n");
		  HW_DMSG_AT45("at45db:    =====================\n");
		  CHECK_WIP;
		  AT45_DATA->command_need_to_complete   = 1;
		  break;
		case AT45_OP_RDSR: 
		  HW_DMSG_AT45("at45db:    =====================\n");
		  HW_DMSG_AT45("at45db:    starting RDSR command\n");
		  HW_DMSG_AT45("at45db:    =====================\n");
		  AT45_DATA->command_need_to_complete   = 0;
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_AT45DB_POWER_READ, 0);
		  break;
		case AT45_OP_WRSR:
		  HW_DMSG_AT45("at45db:    =====================\n");
		  HW_DMSG_AT45("at45db:    starting WRSR command\n");
		  HW_DMSG_AT45("at45db:    =====================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  AT45_DATA->command_needed_data        = 1;
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_AT45DB_POWER_WRITE, 0);
		  break;
		case AT45_OP_READ: 
		  CHECK_WIP;
		  HW_DMSG_AT45("at45db:    =====================\n");
		  HW_DMSG_AT45("at45db:    starting READ command\n");
		  HW_DMSG_AT45("at45db:    =====================\n");
		  AT45_DATA->command_needed_data        = 3;
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_AT45DB_POWER_READ, 0);
		  break;
		case AT45_OP_FAST_READ:
		  CHECK_WIP;
		  HW_DMSG_AT45("at45db:    ==========================\n");
		  HW_DMSG_AT45("at45db:    starting FAST READ command\n");
		  HW_DMSG_AT45("at45db:    ==========================\n");
		  AT45_DATA->command_needed_data        = 4; /* 3 + dummy byte */
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_AT45DB_POWER_READ, 0);
		  break;
		case AT45_OP_PP:
		  HW_DMSG_AT45("at45db:    =============================\n");
		  HW_DMSG_AT45("at45db:    starting PAGE PROGRAM command\n");
		  HW_DMSG_AT45("at45db:    =============================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  AT45_DATA->command_needed_data        = 3;
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_AT45DB_POWER_WRITE, 0);
		  break;
		case AT45_OP_SE: 
		  HW_DMSG_AT45("at45db:    =============================\n");
		  HW_DMSG_AT45("at45db:    starting SECTOR ERASE command\n");
		  HW_DMSG_AT45("at45db:    =============================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  AT45_DATA->command_needed_data        = 3;
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_AT45DB_POWER_WRITE, 0);
		  break;
		case AT45_OP_BE:
		  HW_DMSG_AT45("at45db:    ===========================\n");
		  HW_DMSG_AT45("at45db:    starting BULK ERASE command\n");
		  HW_DMSG_AT45("at45db:    ===========================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  AT45_DATA->command_need_to_complete   = 1;
		  etracer_slot_event(ETRACER_PER_ID_AT45DB,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_AT45DB_POWER_WRITE, 0);
		  break;
		case AT45_OP_DP:
		  HW_DMSG_AT45("at45db:    ================================\n");
		  HW_DMSG_AT45("at45db:    starting DEEP POWER MODE command\n");
		  HW_DMSG_AT45("at45db:    ================================\n");
		  CHECK_WIP;
		  AT45_DATA->command_need_to_complete   = 1;
		  break;
		case AT45_OP_RES:
		  HW_DMSG_AT45("at45db:    ====================\n");
		  HW_DMSG_AT45("at45db:    starting RES command\n");
		  HW_DMSG_AT45("at45db:    ====================\n");
		  AT45_DATA->command_needed_data        = 3;
		  break;
		default:
		  HW_DMSG_AT45("at45db:    unknown command 0x%02x\n",AT45_DATA->data_buffer);
		  ERROR("at45db:    unknown command 0x%02x\n",AT45_DATA->data_buffer);
		  break;
		}
	      break; /* switch AT45_OP_NOP */

	      /***********************************************
	       * Instructions have already been started, we 
	       * keep on fetching data until end of instruction
	       * either by self -> command_need_to_complete or
	       * when CS goes low
	       ***********************************************/
	    case AT45_OP_RDSR:
	      AT45_DATA->dummy_write_for_read = 1;
	      break;
	      
	    case AT45_OP_WRSR: 
	      /* =========== */
	      if (AT45_DATA->command_needed_data > 0) 
		{
		  AT45_DATA->command_needed_data = 0;
		  AT45_DATA->command_stored_data[0] = AT45_DATA->data_buffer;
		} 
	      else 
		{
		  AT45_DATA->command_needed_data --;
		  AT45_DATA->command_need_to_complete = 1;
		}
	      break;
	      
	    case AT45_OP_READ:
	      /* =========== */
	      if (AT45_DATA->command_needed_data > 0) 
		{
		  AT45_DATA->command_stored_data[3 - AT45_DATA->command_needed_data] = AT45_DATA->data_buffer;
		  AT45_DATA->command_needed_data --;
		  if (AT45_DATA->command_needed_data == 0)
		    {
		      AT45_DATA->command_pointer = 
			(AT45_DATA->command_stored_data[0] << 16) | 
			(AT45_DATA->command_stored_data[1] <<  8) |
			(AT45_DATA->command_stored_data[2]      );
		      HW_DMSG_AT45("at45db:    read ready at address 0x%06x (page 0x%04x)\n",
				  AT45_DATA->command_pointer,AT45_DATA->command_pointer >> 8);
		    }
		}
	      else
		{
		  // address is ok
		  HW_DMSG_AT45("at45db:    READ dummy write for read\n");
		  AT45_DATA->command_needed_data --;
		  AT45_DATA->dummy_write_for_read = 1;
		}
	      break;
	      
	    case AT45_OP_FAST_READ:
	      /* =========== */
	      if (AT45_DATA->command_needed_data > 0) 
		{
		  AT45_DATA->command_stored_data[4 - AT45_DATA->command_needed_data] = AT45_DATA->data_buffer;
		  AT45_DATA->command_needed_data --;
		  if (AT45_DATA->command_needed_data == 0)
		    {
		      AT45_DATA->command_pointer = 
			(AT45_DATA->command_stored_data[0] << 16) | 
			(AT45_DATA->command_stored_data[1] <<  8) |
			(AT45_DATA->command_stored_data[2]      );
		      HW_DMSG_AT45("at45db:    FAST Read ready at address 0x%06x (page 0x%04x)\n",
				  AT45_DATA->command_pointer,AT45_DATA->command_pointer >> 8);
		    }
		} 
	      else 
		{
		  // address is ok
		  HW_DMSG_AT45("at45db:    FAST READ dummy write for read\n");
		  AT45_DATA->command_needed_data --;
		  AT45_DATA->dummy_write_for_read = 1;
		}
	      break;
	      
	    case AT45_OP_SE:
	      /* ========= */
	      if (AT45_DATA->command_needed_data > 0) 
		{
		  AT45_DATA->command_stored_data[3 - AT45_DATA->command_needed_data] = AT45_DATA->data_buffer;
		  AT45_DATA->command_needed_data --;
		} 
	      else 
		{
		  AT45_DATA->command_pointer = 
		    (AT45_DATA->command_stored_data[0] << 16) | 
		    (AT45_DATA->command_stored_data[1] <<  8) |
		    (AT45_DATA->command_stored_data[2]      );
		  HW_DMSG_AT45("at45db:    sector erase (linear=%x, sector %x)\n",
			      AT45_DATA->command_pointer, (AT45_DATA->command_pointer >> 16) & 0xff);
		  AT45_DATA->command_need_to_complete = 1;
		}
	      break;
	      
	    case AT45_OP_PP:
	      /* ========= */
	      if (AT45_DATA->command_needed_data > 0) 
		{
		  AT45_DATA->command_stored_data[3 - AT45_DATA->command_needed_data] = AT45_DATA->data_buffer;
		  AT45_DATA->command_needed_data --;
		  if (AT45_DATA->command_needed_data == 0)
		    {
		      AT45_DATA->command_pointer = 
			(AT45_DATA->command_stored_data[0] << 16) | 
			(AT45_DATA->command_stored_data[1] <<  8) |
			(AT45_DATA->command_stored_data[2]      );
		    }
		} 
	      else 
		{
		  // address is ok, we write at the counter address
		  uint16_t page_index;
		  uint16_t page_offset;
		  page_index  = AT45_DATA->command_pointer / AT45_PAGE_SIZE;
		  page_offset = AT45_DATA->command_pointer % AT45_PAGE_SIZE;
		  HW_DMSG_AT45("at45db:    page program (linear=%x, page %x, offset %x)\n",
			      AT45_DATA->command_pointer, page_index, page_offset);
		  AT45_MEMPAGE[page_index][page_offset] &= AT45_DATA->data_buffer;
		  page_offset ++;
		  if (page_offset == AT45_PAGE_SIZE)
		    {
		      AT45_DATA->command_pointer = page_index;
		    }
		  else 
		    {
		      AT45_DATA->command_pointer ++;
		    }
		}
	      break;
	      
	    case AT45_OP_RES:
	      /* ========== */
	      if (AT45_DATA->command_needed_data > 0) 
		{
		  HW_DMSG_AT45("at45db:    RES dummy write %d\n",3 - AT45_DATA->command_needed_data);
		  AT45_DATA->command_stored_data[3 - AT45_DATA->command_needed_data] = AT45_DATA->data_buffer;
		  AT45_DATA->command_needed_data --;
		}
	      else
		{
		  HW_DMSG_AT45("at45db:    RES dummy write for read\n");
		  AT45_DATA->command_needed_data --;
		  AT45_DATA->dummy_write_for_read = 1;
		}
	      break;
	      
	    default: /* switch command */
	      ERROR("at45db:    internal error command 0x%02x unknown\n",AT45_DATA->data_buffer);
	      HW_DMSG_AT45("at45db:    internal error command 0x%02x unknown\n",AT45_DATA->data_buffer);
	      at45db_error_dump_internal_state(dev);
	      break;
	    }
	  
	  break;
	  
	  /*********************************************************
	   * End active mode.
	   *
	   * next lines are for other power modes 
	   *********************************************************/
	  
	case AT45_POWER_STANDBY:
	  ERROR("at45db:    error in power standby, flash should not receive data\n");
	  HW_DMSG_AT45("at45db:    error in power standby, flash should not receive data\n");
	  break;
	  
	case AT45_POWER_DEEP_DOWN:
	  AT45_DATA->command = AT45_DATA->data_buffer;
	  if (AT45_DATA->command == AT45_OP_RES)
	    {
	      AT45_DATA->data_buffer_ok = 0;
	      AT45_DATA->power_mode = AT45_POWER_ACTIVE;
	      tracer_event_record(TRACER_AT45DB, AT45_POWER_ACTIVE);
	      etracer_slot_event(ETRACER_PER_ID_AT45DB,
				 ETRACER_PER_EVT_MODE_CHANGED,
				 ETRACER_AT45DB_POWER_ACTIVE, 0);
	      HW_DMSG_AT45("at45db:    Release from Deep Power Down mode\n");
	    }
	  break;
	} /* switch power_mode */
      
    }
  else /* ! need_to_complete */
    {
      if (AT45_DATA->command_need_to_complete == 1)
	{
	  ERROR("at45db:    write data [val=0x%02x,mask=0x%04x] while needed to complete\n",value & AT45DB_D,mask);
	  HW_DMSG_AT45("at45db:    write data [val=0x%02x,mask=0x%04x] while needed to complete [PC 0x%4x]\n",
		      value & AT45DB_D,mask, mcu_get_pc());
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * - A command is entered when the Opcode byte is written
 * - var command_needed_data is set to the number of argument
 *   bytes needed to start the command.
 * 
 * - at end we have command_need_to_complete = 1
 */

/*
 * _DATA
 *    switch (power_mode)
 *       case active: (waiting for a command)
 *           switch command
 *              case NOP: 
 *                   starting a command;
 *              case XXX: 
 *                   during a command;
 *           end
 *  
 *       case standby:
 *       case deep_power:
 *    end
 *
 * _CS
 *   if (1->0)
 *     switch (command)
 *        case XXX: end command
 *   else
 *     state = Active
 *   end
 *
 * _WRITE
 * _CLOCK
 */

void at45db_write(int dev, uint32_t mask, uint32_t value)
{
  HW_DMSG_AT45("at45db: write to flash 0x%04x mask 0x%04x\n",value, mask);
  
  /********/
  /* Data */
  /********/
  if ((mask & AT45DB_D))
    {
      if (AT45_DATA->select_bit == 1)
	{
	  etracer_slot_event(ETRACER_PER_ID_AT45DB,	 ETRACER_PER_EVT_WRITE_COMMAND, 
			     ETRACER_PER_ARG_WR_DST | ETRACER_ACCESS_LVL_SPI1, 0);
	  at45db_write_spidata(dev, mask, value);
	}
      else
	{
	}
    } /* data (mask & AT45DB_D) */


  /***************************
   * Control pins.
   * Chip Select
   ***************************/

#define END_COMMAND()                                                           \
do {                                                                            \
  HW_DMSG_AT45("at45db:    End command 0x%02x (%s)\n",                           \
	      AT45_DATA->command, str_cmd(AT45_DATA->command));                   \
  AT45_DATA->command_need_to_complete = 0;                                       \
  AT45_DATA->command = AT45_OP_NOP;                                               \
  HW_DMSG_AT45("at45db:    chip select : Active -> Standby (end of command)\n"); \
} while (0) 

#define END_COMMAND_GO_STANDBY()                                                \
do {                                                                            \
  END_COMMAND();                                                                \
  AT45_DATA->power_mode = AT45_POWER_STANDBY;                                     \
  tracer_event_record(TRACER_AT45DB, AT45_POWER_STANDBY);                        \
  etracer_slot_event(ETRACER_PER_ID_AT45DB,                                     \
		     ETRACER_PER_EVT_MODE_CHANGED,                              \
		     ETRACER_AT45DB_POWER_STANDBY, 0);                          \
} while (0)

  if (mask & AT45DB_S) /* chip select negated */
    {
      uint8_t bit_cs = ! (value & AT45DB_S);
      HW_DMSG_AT45("at45db:    flash write CS = %d\n",bit_cs);

      if ((AT45_DATA->select_bit == 1) && (bit_cs == 0)) 
	{ /* CS: 1 -> 0    CS goes low */
	  AT45_DATA->select_bit = bit_cs;

	  switch (AT45_DATA->command)
	    {
	    case AT45_OP_NOP:
	      break;
	    case AT45_OP_WREN: 
	      // AT45_DATA->command_need_to_complete is always set
	      AT45_DATA->status_register.b.wel    = 1; 
	      END_COMMAND_GO_STANDBY();
	      break;
	    case AT45_OP_WRDI:
	      // AT45_DATA->command_need_to_complete is always set
	      AT45_DATA->status_register.b.wel    = 0;
	      END_COMMAND_GO_STANDBY();
	      break;
	    case AT45_OP_RDSR:
	      // AT45_DATA->command_need_to_complete is NOT set
	      END_COMMAND_GO_STANDBY();
	      break;
	    case AT45_OP_WRSR:
		  // AT45_DATA->command_need_to_complete MUST be set
	      CHECK_COMMAND_ARGS;
	      if (AT45_DATA->command_need_to_complete == 1) {
		at45db_write_status_register(dev,AT45_DATA->command_stored_data[0]);
	      }
	      END_COMMAND_GO_STANDBY();
	      break;
	    case AT45_OP_READ:
	      // AT45_DATA->command_need_to_complete is NOT set
	      CHECK_COMMAND_ARGS;
	      END_COMMAND_GO_STANDBY();
	      break;
	    case AT45_OP_FAST_READ:
	      // AT45_DATA->command_need_to_complete is NOT set
	      CHECK_COMMAND_ARGS;
	      END_COMMAND_GO_STANDBY();
	      break;
	    case AT45_OP_PP: 
	      // AT45_DATA->command_need_to_complete MIGHT be set
	      // AT45_DATA->command_needed_data == 0 ?
	      CHECK_COMMAND_ARGS;
	      AT45_DATA->status_register.b.wel    = 0;
	      END_COMMAND();
	      /* wait busy flag, mode change in update */
	      break;
	    case AT45_OP_SE: 
	      // AT45_DATA->command_need_to_complete MUST be set
	      CHECK_COMMAND_ARGS;
	      at45db_erase_sector(dev,AT45_DATA->command_pointer); 
	      AT45_DATA->status_register.b.wel    = 0;
	      END_COMMAND();
	      /* wait busy flag, mode change in update */
	      break;
	    case AT45_OP_BE: 
	      // AT45_DATA->command_need_to_complete is always set
	      at45db_erase_bulk(dev);  
	      AT45_DATA->status_register.b.wel    = 0;
	      END_COMMAND();
	      /* wait busy flag, mode change in update */
	      break;
	    case AT45_OP_DP:
	      // AT45_DATA->command_need_to_complete is always set
	      END_COMMAND();
	      HW_DMSG_AT45("at45db:    Power mode changed to DEEP POWER DOWN\n");	
	      AT45_DATA->power_mode = AT45_POWER_DEEP_DOWN;
	      tracer_event_record(TRACER_AT45DB, AT45_POWER_DEEP_DOWN);
	      etracer_slot_event(ETRACER_PER_ID_AT45DB,
				 ETRACER_PER_EVT_MODE_CHANGED,
				 ETRACER_AT45DB_POWER_DEEP_DOWN, 0);
	      break;
	    case AT45_OP_RES:
	      // AT45_DATA->command_need_to_complete MIGHT be set
	      // AT45_DATA->command_needed_data is variable		 
	      END_COMMAND();
	      HW_DMSG_AT45("at45db:    Power mode changed to ACTIVE\n");
	      AT45_DATA->power_mode = AT45_POWER_ACTIVE;
	      tracer_event_record(TRACER_AT45DB, AT45_POWER_ACTIVE);
	      etracer_slot_event(ETRACER_PER_ID_AT45DB,
				 ETRACER_PER_EVT_MODE_CHANGED,
				 ETRACER_AT45DB_POWER_ACTIVE, 0);
	      break;
	    default:
	      ERROR("at45db:    Unterminated switch/case for end command CS=0\n");
	      HW_DMSG_AT45("at45db:    Unterminated switch/case for end command CS=0\n");
	      END_COMMAND();
	      break;
	    } /* switch (AT45_DATA->command) */
	} /* CS goes low */
      else if ((AT45_DATA->select_bit == 0) && (bit_cs == 1)) 
	{ /* CS: 0 -> 1,  CS goes high */
	  AT45_DATA->select_bit = bit_cs;
	  HW_DMSG_AT45("at45db:    flash write CS = 1 : Standby -> Active\n");
	  AT45_DATA->power_mode = AT45_POWER_ACTIVE;
	  tracer_event_record(TRACER_AT45DB, AT45_POWER_ACTIVE);
	  etracer_slot_event(ETRACER_PER_ID_AT45DB,
			     ETRACER_PER_EVT_MODE_CHANGED,
			     ETRACER_AT45DB_POWER_ACTIVE, 0);
	}
    } /* mask & AT45DB_S */ 

  /***************************
   * Control pins. WRITE PROTECT
   ***************************/

  if (mask & AT45DB_W) /* write protect netgated */
    {
      AT45_DATA->write_protect_bit  = ! (value & AT45DB_W);
      HW_DMSG_AT45("at45db:    flash write protect W = %d\n",AT45_DATA->write_protect_bit);
    }

  /***************************
   * Control pins. CLOCK
   ***************************/

  if (mask & AT45DB_C) /* clock */
    {
      ERROR("at45db:    clock pin should not be used during simulation\n");
      HW_DMSG_AT45("at45db:    clock pin should not be used during simulation\n");
      AT45_DATA->clock = (value >> AT45DB_C_SHIFT) & 0x1;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static int at45db_update_write_flag(int dev)
{
  if ((AT45_DATA->status_register.b.wip == 1) && (MACHINE_TIME_GET_NANO() >= AT45_DATA->end_of_busy_time))
    {
      AT45_DATA->status_register.b.wip = 0;
      /* AT45_DATA->status_register.b.wel = 0; ok pour WRSR */
      AT45_DATA->command = AT45_OP_NOP;
      HW_DMSG_AT45("at45db:    ====================================\n");
      HW_DMSG_AT45("at45db:    AT45DB display busyflag returns to 0\n");
      HW_DMSG_AT45("at45db:    ====================================\n");
      return 1;
    }
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_update(int dev)
{
  switch (AT45_DATA->power_mode)
    {
      /**********************/
      /* Active poser mode  */
      /**********************/
    case AT45_POWER_ACTIVE:
      /* end current command ? */
      if (at45db_update_write_flag(dev))
	{
	  /* busyflag returned to 0 : end of current command */
	  if ((AT45_DATA->select_bit == 0))
	    {
	      /* going from active to standby */
	      HW_DMSG_AT45("at45db: update: end of command wip=0: Active -> Standby\n");
	      tracer_event_record(TRACER_AT45DB, AT45_POWER_STANDBY);
	      etracer_slot_event(ETRACER_PER_ID_AT45DB,
				 ETRACER_PER_EVT_MODE_CHANGED,
				 ETRACER_AT45DB_POWER_STANDBY, 0);

	    }
	}
      break;

      /**********************/
      /* Standby power mode */
      /**********************/
    case AT45_POWER_STANDBY:
      /* nothing to do                             */
      /* state is changed in the write switch      */
      break;

      /**********************/
      /* Deep power down    */
      /**********************/
    case AT45_POWER_DEEP_DOWN:
      /* nothing to do                             */
      /* going out of DP is done in the write loop */
      break;
    }
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_ui_draw      (int UNUSED dev)
{
  return 0;
}

void at45db_ui_get_size (int UNUSED dev, int *w, int *h)
{
  w = 0;
  h = 0;
}

void at45db_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y)
{
  
}

void at45db_ui_get_pos  (int UNUSED dev, int *x, int *y)
{
  *x = 0;
  *y = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/
