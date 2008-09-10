
/**
 *  \file   m25p80_dev.c
 *  \brief  STmicro m25p flash memory modules
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/m25p80/m25p80_dev.h"
#include "src/options.h"

/***************************************************/
/***************************************************/

#if defined(BUILD_M25P80)
#  define M25PNAME      "m25p80"
#  define TRACER_M25P_STATE     TRACER_M25P80_STATE
#  define TRACER_M25P_STROBE    TRACER_M25P80_STROBE
#elif defined(BUILD_M25P10)
#  define M25PNAME      "m25p10"
#  define TRACER_M25P_STATE     TRACER_M25P10_STATE
#  define TRACER_M25P_STROBE    TRACER_M25P10_STROBE
#else
#  error "you must define a specific M25P model"
#endif

/***************************************************/
/***************************************************/
#undef DEBUG 
#ifdef DEBUG
#define MSG_DEVICES       2
#define DEBUG_ME_HARDER
#define HW_DMSG_M25(x...) VERBOSE(MSG_DEVICES,x)
#else
#define HW_DMSG_M25(x...) do {} while(0)
#endif

/***************************************************/
/***************************************************/

enum m25p_power_state_t {
  M25P_POWER_DEEP_DOWN    = 1,
  M25P_POWER_STANDBY      = 2,
  M25P_POWER_ACTIVE       = 3
};


enum m25p_opcode_t {
  M25P_OP_NOP       = 0x00u,  /* nop                   */
  M25P_OP_WREN      = 0x06u,  /* write enable          */
  M25P_OP_WRDI      = 0x04u,  /* write disable         */
  M25P_OP_RDSR      = 0x05u,  /* read status register  */
  M25P_OP_WRSR      = 0x01u,  /* write status register */   
  M25P_OP_READ      = 0x03u,  /* read                  */
  M25P_OP_FAST_READ = 0x0Bu,  /* fast read             */
  M25P_OP_PP        = 0x02u,  /* page program          */
  M25P_OP_SE        = 0xd8u,  /* sector erase          */
  M25P_OP_BE        = 0xc7u,  /* bulk erase            */
  M25P_OP_DP        = 0xb9u,  /* deep power down       */
  M25P_OP_RES       = 0xabu   /* release from DP       */
};


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed))  m25p_status_t {
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
struct __attribute__ ((packed)) m25p_status_t {
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
    case M25P_OP_WREN:      return "WREN";
    case M25P_OP_WRDI:      return "WRDI";
    case M25P_OP_RDSR:      return "RDSR";
    case M25P_OP_WRSR:      return "WRSR";
    case M25P_OP_READ:      return "READ";
    case M25P_OP_FAST_READ: return "FAST READ";
    case M25P_OP_PP:        return "PAGE PROGRAM";
    case M25P_OP_SE:        return "SECTOR ERASE";
    case M25P_OP_BE:        return "BULK ERASE";
    case M25P_OP_DP:        return "DEEP POWER";
    case M25P_OP_RES:       return "RES";
    default:               return "UNKNOWN";
    }
  return "UNKNOWN";
}
#endif


/***************************************************/
/** Flash internal data ****************************/
/***************************************************/

#define NANO   (1)
#define MICRO  (1000)
#define MILLI  (1000 * 1000)
#define SECOND (1000 * 1000 * 1000)

/* ====================== */
#if defined(BUILD_M25P80)
/* ====================== */
/**
 * 8 Mb of Flash Memory
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
#  define M25P_FLASH_SIZE           0x100000
#  define M25P_SECTOR_MAX           16
#  define M25P_SECTOR_SIZE          65536
#  define M25P_PAGE_MAX             4096
#  define M25P_PAGE_SIZE            256
#  define M25P_MAX_COMMAND_DATA     100
#  define M25P_ELECTRONIC_SIGNATURE 0x13
#  define M25P_Grade6
// device grade 6 : times in micro seconds
#if defined(M25P_Grade6)
#  define Freq_max                   40
#  define TIME_WRITE_STATUS_REGISTER (5    * MILLI)
#  define TIME_PAGE_PROGRAM          (1400 * MICRO)
#  define TIME_SECTOR_ERASE          ( 1000000000ull) /*  1 s */
#  define TIME_BULK_ERASE            (10000000000ull) /* 10 s */
/*                                      aaabbbccc             */
#elif defined(M25P_Grade3)
#  define Freq_max                   25
#  define TIME_WRITE_STATUS_REGISTER (8    * MILLI)
#  define TIME_PAGE_PROGRAM          (1500 * MICRO)
#  define TIME_SECTOR_ERASE          ( 1000000000ull) /*  1 s */
#  define TIME_BULK_ERASE            (10000000000ull) /* 10 s */
#else
#  error "must define speed grade for Flash memory M25P"
#endif

/* ====================== */
#elif defined(BUILD_M25P10)
/* ====================== */
/**
 * 1 Mbit of Flash memory
 * Page Program (up to 256 bytes) in 1.4 ms
 * Sector Erase (256 Kbit) in 0.65 s (typical)
 * Bulk Erase (1 Mbit) in 1.7 s (typical)
 * 2.3 to 3.6 V single supply voltage
 * SPI bus compatible serial interface
 * 50 MHz clock rate (maximum)
 * Deep Power-down mode 1 μA (typical)
 * Electronic signatures
 *  – JEDEC Standard two-byte signature
 *    (2011h)
 *  – RES instruction, one-byte signature (10h),
 *    for backward compatibility
 *
 */

/* TODO: JDEC identification not done */
#  define M25P_FLASH_SIZE           131872
#  define M25P_SECTOR_MAX           4
#  define M25P_SECTOR_SIZE          32768
#  define M25P_PAGE_MAX             512
#  define M25P_PAGE_SIZE            256
#  define M25P_MAX_COMMAND_DATA     100
#  define M25P_ELECTRONIC_SIGNATURE 0x10
#  define M25P_Grade6
#if defined(M25P_Grade6)
#  define Freq_max                   50
#  define TIME_WRITE_STATUS_REGISTER (15   * MILLI)
#  define TIME_PAGE_PROGRAM          (1400 * MICRO)
#  define TIME_SECTOR_ERASE          (650  * MILLI)
#  define TIME_BULK_ERASE            (1700 * MILLI)
#elif defined(M25P_Grade3)
#  define Freq_max                   50
#  define TIME_WRITE_STATUS_REGISTER (15   * MILLI)
#  define TIME_PAGE_PROGRAM          (1500 * MICRO)
#  define TIME_SECTOR_ERASE          (1000 * MILLI)
#  define TIME_BULK_ERASE            (4500 * MILLI)
#endif

#else
#error "must define a m�=25p device"
#endif

/***************************************************/
/** Flash internal data ****************************/
/***************************************************/

struct m25p_t 
{
  union {
    struct m25p_status_t b;
    uint8_t             s;
  } status_register;

  uint8_t select_bit;           /* chip select   */
  uint8_t hold_bit;             /* hold          */
  uint8_t write_protect_bit;    /* write protect */

  union {
    uint8_t raw   [M25P_FLASH_SIZE];
    uint8_t sector[M25P_SECTOR_MAX][M25P_SECTOR_SIZE];
    uint8_t page  [M25P_PAGE_MAX][M25P_PAGE_SIZE];
  } mem;

  enum m25p_power_state_t   power_mode;
  enum m25p_opcode_t        command;

  int32_t                  command_needed_data;
  uint32_t                 command_pointer;

  uint8_t                  command_need_to_complete;
  uint8_t                  command_stored_data[M25P_MAX_COMMAND_DATA];
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

#define M25P_DATA        ((struct m25p_t*)(machine.device[dev].data))
#define M25P_MEMRAW      (M25P_DATA->mem.raw   )
#define M25P_MEMSECTOR   (M25P_DATA->mem.sector)
#define M25P_MEMPAGE     (M25P_DATA->mem.page  )
#define M25P_INIT        (M25P_DATA->file_init )
#define M25P_DUMP        (M25P_DATA->file_dump )

/***************************************************/
/** Flash external entry points ********************/
/***************************************************/

int  m25p_reset       (int dev);
int  m25p_delete      (int dev);
int  m25p_power_up    (int dev);
int  m25p_power_down  (int dev);
void m25p_read        (int dev, uint32_t *mask, uint32_t *value);
void m25p_write       (int dev, uint32_t  mask, uint32_t  value);
int  m25p_update      (int dev);
int  m25p_ui_draw     (int dev);
void m25p_ui_get_size (int dev, int *w, int *h);
void m25p_ui_set_pos  (int dev, int  x, int  y);
void m25p_ui_get_pos  (int dev, int *x, int *y);

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


int m25p_add_options(int UNUSED dev_num, int dev_id, const char UNUSED *dev_name)
{
  if (dev_id >= 1)
    {
      ERROR("m25p: too much devices, please rewrite option handling\n");
      return -1;
    }

  options_add( &flash_init_opt  );
  options_add( &flash_dump_opt  );
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_device_size()
{
  return sizeof(struct m25p_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_flash_load(int dev, const char *name)
{
  FILE *f;

  if ((f = fopen(name, "r")) == NULL)
    {
      return 1;
    }
  
  HW_DMSG_M25(M25PNAME ": loading file %s into flash\n",name);

  if (fread(M25P_MEMRAW,1,M25P_FLASH_SIZE,f) != M25P_FLASH_SIZE)
    {
      HW_DMSG_M25(M25PNAME ": error while loading file %s to flash\n",name);
      ERROR      (M25PNAME ": error while loading file %s to flash\n",name);
      return 2;
    }

  HW_DMSG_M25(M25PNAME ": loading file %s into flash -- ok\n",name);

  fclose(f);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_flash_dump(int dev, const char *name)
{
  FILE *f;

  if ((f = fopen(name, "r")) == NULL)
    {
      return 1;
    }
  
  HW_DMSG_M25(M25PNAME ": saving file %s into flash\n",name);

  if (fwrite(M25P_MEMRAW,1,M25P_FLASH_SIZE,f) != M25P_FLASH_SIZE)
    {
      HW_DMSG_M25(M25PNAME ": error while dumping flash to file %s\n",name);
      ERROR      (M25PNAME ": error while dumping flash to file %s\n",name);
      return 2;
    }

  HW_DMSG_M25(M25PNAME ": saving flash into file %s -- ok\n",name);

  fclose(f);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_device_create(int dev, int UNUSED id)
{
  machine.device[dev].reset         = m25p_reset;
  machine.device[dev].delete        = m25p_delete;
  machine.device[dev].power_up      = m25p_power_up;
  machine.device[dev].power_down    = m25p_power_down;

  machine.device[dev].read          = m25p_read;
  machine.device[dev].write         = m25p_write;
  machine.device[dev].update        = m25p_update;

  machine.device[dev].ui_draw       = m25p_ui_draw;
  machine.device[dev].ui_get_size   = m25p_ui_get_size;
  machine.device[dev].ui_set_pos    = m25p_ui_set_pos;

  machine.device[dev].state_size    = m25p_device_size();
  machine.device[dev].name          = M25PNAME " flash memory";

  M25P_INIT = flash_init_opt.value;
  M25P_DUMP = flash_dump_opt.value;

#if defined(DEBUG_ME_HARDER)
  HW_DMSG_M25(M25PNAME ": =================================== \n");
  HW_DMSG_M25(M25PNAME ": 0000 CHSW dddd dddd == MASK         \n");
  HW_DMSG_M25(M25PNAME ":      C              : Clock         \n");
  HW_DMSG_M25(M25PNAME ":       H             : Hold          \n");
  HW_DMSG_M25(M25PNAME ":        S            : Chip Select   \n");
  HW_DMSG_M25(M25PNAME ":         W           : Write Protect \n");
  HW_DMSG_M25(M25PNAME ":           dddd dddd : SPI data      \n");
  HW_DMSG_M25(M25PNAME ": =================================== \n");
#endif

  if (M25P_INIT == NULL || m25p_flash_load(dev, M25P_INIT))
    {
      HW_DMSG_M25(M25PNAME ": flash memory init to 0xff\n");
      memset(M25P_MEMRAW,0xff,M25P_FLASH_SIZE);
    }

  tracer_event_add_id(TRACER_M25P_STATE,   8, M25PNAME"_state"    , "");
  tracer_event_add_id(TRACER_M25P_STROBE,  8, M25PNAME"_function" , "");

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_reset(int dev)
{
  HW_DMSG_M25(M25PNAME ": flash reset\n");
  M25P_DATA->command                  = M25P_OP_NOP;
  M25P_DATA->command_need_to_complete = 0;
  M25P_DATA->status_register.s        = 0;
  M25P_DATA->power_mode               = M25P_POWER_STANDBY;

  tracer_event_record(TRACER_M25P_STATE, M25P_POWER_STANDBY);
  etracer_slot_event(ETRACER_PER_ID_M25P,
		     ETRACER_PER_EVT_MODE_CHANGED,
		     ETRACER_M25P_POWER_STANDBY, 0);

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_delete(int dev)
{
  if (M25P_DUMP)
    {
      m25p_flash_dump(dev,M25P_DUMP);
    }

  if (M25P_INIT)
    {
      free(M25P_INIT);
    }

  if (M25P_DUMP)
    {
      free(M25P_DUMP);
    }

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void m25p_error_dump_internal_state(int UNUSED dev)
{
#if defined(DEBUG_ME_HARDER)
  HW_DMSG_M25(M25PNAME ": Power mode %d\n",M25P_DATA->power_mode);
  HW_DMSG_M25(M25PNAME ": Command 0x%02x\n",M25P_DATA->command);
  HW_DMSG_M25(M25PNAME ": need complete %d\n",M25P_DATA->command_need_to_complete);
  HW_DMSG_M25(M25PNAME ": select %d hold %d write protect %d\n",
	      M25P_DATA->select_bit,M25P_DATA->hold_bit,M25P_DATA->write_protect_bit);
  HW_DMSG_M25(M25PNAME ": status register wip  %d\n",M25P_DATA->status_register.b.wip); 
  HW_DMSG_M25(M25PNAME ":                 wel  %d\n",M25P_DATA->status_register.b.wel); 
  HW_DMSG_M25(M25PNAME ":                 bp0  %d\n",M25P_DATA->status_register.b.bp0);
  HW_DMSG_M25(M25PNAME ":                 bp1  %d\n",M25P_DATA->status_register.b.bp1);
  HW_DMSG_M25(M25PNAME ":                 bp2  %d\n",M25P_DATA->status_register.b.bp2);
  HW_DMSG_M25(M25PNAME ":                 u2   %d\n",M25P_DATA->status_register.b.unused2);
  HW_DMSG_M25(M25PNAME ":                 u1   %d\n",M25P_DATA->status_register.b.unused1);
  HW_DMSG_M25(M25PNAME ":                 srwd %d\n",M25P_DATA->status_register.b.srwd);
#endif
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_power_up(int UNUSED dev)
{
  /* power up timing doc page 27 */
  HW_DMSG_M25(M25PNAME ": flash power up\n");
  return 0;
}

int m25p_power_down(int UNUSED dev)
{
  HW_DMSG_M25(M25PNAME ": flash power down\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

static void m25p_set_write_time(int dev, uint64_t nano)
{
  M25P_DATA->status_register.b.wip = 1;
  M25P_DATA->end_of_busy_time      = MACHINE_TIME_GET_NANO() + nano;
  HW_DMSG_M25(M25PNAME ": flash busyflag for %"PRIu64"ns, end at %"PRIu64"ns\n", nano, M25P_DATA->end_of_busy_time);
}

/***************************************************/
/***************************************************/
/***************************************************/

static void m25p_erase_sector(int dev, uint32_t address)
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

  sector   = address / M25P_SECTOR_SIZE;
  protbits = M25P_DATA->status_register.b.bp1 << 2 |
             M25P_DATA->status_register.b.bp1 << 1 |
             M25P_DATA->status_register.b.bp0;

  m25p_set_write_time(dev,TIME_SECTOR_ERASE);
  HW_DMSG_M25(M25PNAME ":  sector %d erase (address=0x%06x)\n",address / M25P_SECTOR_SIZE, address);

  if (sector >= protection_base[protbits])
    {
      ERROR      (M25PNAME ": protection error : sector %d protected by bpx bits\n",sector);
      HW_DMSG_M25(M25PNAME ": protection error : sector %d protected by bpx bits\n",sector);
    }

  memset(M25P_MEMSECTOR[address / M25P_SECTOR_SIZE],0xff,M25P_SECTOR_SIZE);
}

/***************************************************/
/***************************************************/
/***************************************************/

static void m25p_erase_bulk(int dev)
{
  int protbits;

  m25p_set_write_time(dev,TIME_BULK_ERASE);
  HW_DMSG_M25(M25PNAME ":  bulk erase\n");

  protbits = M25P_DATA->status_register.b.bp1 << 2 |
             M25P_DATA->status_register.b.bp1 << 1 |
             M25P_DATA->status_register.b.bp0;
  if (protbits)
    {
      ERROR(M25PNAME ": protection error = bulk erase with bp0|bp1|bp2 > 0\n");
      HW_DMSG_M25(M25PNAME ": protection error = bulk erase with bp0|bp1|bp2 > 0\n");
    }
  memset(M25P_MEMRAW,0xff,M25P_FLASH_SIZE);
}

/***************************************************/
/***************************************************/
/***************************************************/

static void m25p_write_status_register(int dev, uint8_t value)
{
  /* WRSR has no effect on b6, b5, b1 and b0 */
  M25P_DATA->status_register.s |= (~0x63 & value);
  m25p_set_write_time(dev,TIME_WRITE_STATUS_REGISTER);	      
}

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * read is done only on a junk write
 *
 */

void m25p_read(int dev, uint32_t *mask, uint32_t *value)
{
  if ((M25P_DATA->select_bit == 1) && (M25P_DATA->power_mode == M25P_POWER_ACTIVE))
    {

      if ((M25P_DATA->dummy_write_for_read == 1) && (M25P_DATA->command_read_byte == 0))
	{
	  M25P_DATA->command_read_byte    = 0;
	  M25P_DATA->dummy_write_for_read = 0;
	  switch (M25P_DATA->command)
	    {
	    case M25P_OP_RDSR:
	      *mask  = M25P_D;
	      *value = M25P_DATA->status_register.s & 0xff; 
	      HW_DMSG_M25(M25PNAME ":    read status register = 0x%02x\n", *value);
	      etracer_slot_event(ETRACER_PER_ID_M25P, ETRACER_PER_EVT_WRITE_COMMAND, 
				 ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
	      
	      break;
	      
	    case M25P_OP_READ:
	      /* we need to slow down READ to adjust to f_R output frequency    */
	      /* f_R is 40MHz to 25MHz, should be ok until we get newer devices */
	      if (M25P_DATA->command_needed_data < 0)
		{
		  *mask  = M25P_D;
		  *value = M25P_MEMRAW[M25P_DATA->command_pointer];
		  HW_DMSG_M25(M25PNAME ":    mem data read [0x%06x] = 0x%02x\n",M25P_DATA->command_pointer, *value);
		  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		  
		  M25P_DATA->command_pointer ++;
		  if (M25P_DATA->command_pointer == M25P_FLASH_SIZE)
		    {
		      M25P_DATA->command_pointer = 0;
		    }
		}
	      else
		{
		  *mask  = M25P_D;
		  *value = 0;
		  HW_DMSG_M25(M25PNAME ":    read (dummy read, flash sends 0, needed %d)\n",
			      M25P_DATA->command_needed_data);
		  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      break;
	      
	    case M25P_OP_FAST_READ:
	      /* fast read is driven by the READ_DEV_TO_SPI clock speed */
	      /* limited to f_C, same as above on frequencies           */
	      if (M25P_DATA->command_needed_data < 0)
		{
		  *mask  = M25P_D;
		  *value = M25P_MEMRAW[M25P_DATA->command_pointer];
		  HW_DMSG_M25(M25PNAME ":    FAST READ [0x%06x] = 0x%02x\n",M25P_DATA->command_pointer, *value);
		  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		  M25P_DATA->command_pointer ++;
		  if (M25P_DATA->command_pointer == M25P_FLASH_SIZE)
		    {
		      M25P_DATA->command_pointer = 0;
		    }
		}
	      else
		{
		  *mask  = M25P_D;
		  *value = 0;
		  HW_DMSG_M25(M25PNAME ":    FAST READ (dummy read, flash sends 0, needed %d)\n",
			      M25P_DATA->command_needed_data);
		  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      break;
	      
	    case M25P_OP_RES:
	      if (M25P_DATA->command_needed_data < 0)
		{
		  *mask  = M25P_D;
		  *value = M25P_ELECTRONIC_SIGNATURE;
		  HW_DMSG_M25(M25PNAME ":    read electronic signature = 0x%x\n",*value);
		  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      else
		{
		  *mask  = M25P_D;
		  *value = 0;
		  HW_DMSG_M25(M25PNAME ":    read electronic signature (dummy read, flash sends 0, needed %d)\n",
			      M25P_DATA->command_needed_data);
		  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
		}
	      break;
	      
	    default:
	      HW_DMSG_M25(M25PNAME ": unknown command 0x%x, read dummy in write response\n",M25P_DATA->command & 0xff);
	      *mask  = M25P_D;
	      *value = 0;
	      etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
				 ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
	      break;
	    }
	} // if ((M25P_DATA->dummy_write_for_read == 1) && (M25P_DATA->command_read_byte == 0))
      else if (M25P_DATA->command_read_byte == 1)
	{
	  M25P_DATA->command_read_byte    = 0;
	  M25P_DATA->dummy_write_for_read = 0;
	  *mask  = M25P_D;
	  *value = 0;
	  HW_DMSG_M25(M25PNAME ":    read value that corresponds to command\n");
	  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
			     ETRACER_PER_ARG_WR_SRC | ETRACER_ACCESS_LVL_SPI1, 0);
	}
      else 
	{
	  /* this is a hit for updates where CS and command are enabled */
	  *mask  = 0;
	  *value = 0;
	}
    }
  else //   if ((M25P_DATA->select_bit == 1) && (M25P_DATA->power_mode == M25P_POWER_ACTIVE))
    {
      *mask  = 0;
      *value = 0;
    }

  if (*mask != 0)
    {
      HW_DMSG_M25(M25PNAME ":    m25 --> mcu [val=0x%02x,mask=0x%04x] \n", *value, *mask);
    }

}

/***************************************************/
/***************************************************/
/***************************************************/

#define CHECK_WIP                                                                       \
do {                                                                                    \
  if (M25P_DATA->status_register.b.wip == 1)                                             \
    {                                                                                   \
      ERROR      (M25PNAME ": ===================================================\n");     \
      ERROR      (M25PNAME ": == M25P Command issued while Write In Progress ==\n");     \
      ERROR      (M25PNAME ": ===================================================\n");     \
      HW_DMSG_M25(M25PNAME ": ===================================================\n");     \
      HW_DMSG_M25(M25PNAME ": == M25P Command issued while Write In Progress ==\n");     \
      HW_DMSG_M25(M25PNAME ": ===================================================\n");     \
    }                                                                                   \
} while(0)

#define CHECK_WEL                                                                       \
do {                                                                                    \
  if (M25P_DATA->status_register.b.wel == 0)                                             \
    {                                                                                   \
      ERROR      (M25PNAME ": ======================================================\n");  \
      ERROR      (M25PNAME ": == M25P Command issued without Write Enable (WEL) ==\n");  \
      ERROR      (M25PNAME ": ======================================================\n");  \
      HW_DMSG_M25(M25PNAME ": ======================================================\n");  \
      HW_DMSG_M25(M25PNAME ": == M25P Command issued without Write Enable (WEL) ==\n");  \
      HW_DMSG_M25(M25PNAME ": ======================================================\n");  \
    }                                                                                   \
} while(0)

#define CHECK_COMMAND_ARGS                                                              \
do {                                                                                    \
  if (M25P_DATA->command_needed_data > 0)                                                \
    {                                                                                   \
      ERROR      (M25PNAME ": ===============================================\n");         \
      ERROR      (M25PNAME ": == M25P Command terminated too early by CS ==\n");         \
      ERROR      (M25PNAME ": ===============================================\n");         \
      HW_DMSG_M25(M25PNAME ": ===============================================\n");         \
      HW_DMSG_M25(M25PNAME ": == M25P Command terminated too early by CS ==\n");         \
      HW_DMSG_M25(M25PNAME ": ===============================================\n");         \
    }                                                                                   \
} while(0)

/***************************************************/
/***************************************************/
/***************************************************/

void m25p_write_spidata(int dev, uint32_t UNUSED mask, uint32_t value)
{

  if (M25P_DATA->command_need_to_complete == 0)
    {
      M25P_DATA->data_buffer    = (value & M25P_D);
      M25P_DATA->data_buffer_ok = 1;
      M25P_DATA->dummy_write_for_read = 1;
      //HW_DMSG_M25(M25PNAME ":    write data to flash 0x%02x\n",M25P_DATA->data_buffer);
      
      switch (M25P_DATA->power_mode)
	{
	case M25P_POWER_ACTIVE:
	  /*********************************************************/
	  /* stay active : take a look if we just have had a write */
	  /*   we start a new command by going out of NOP          */
	  /*********************************************************/
	  switch (M25P_DATA->command)
	    {
	    case M25P_OP_NOP:
	      /* ========== */
	      M25P_DATA->command_read_byte = 1;
	      M25P_DATA->command = M25P_DATA->data_buffer;
	      switch (M25P_DATA->data_buffer)
		{
		case M25P_OP_WREN:
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  HW_DMSG_M25(M25PNAME ":    starting WREN command\n");
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  CHECK_WIP;
		  M25P_DATA->command_need_to_complete   = 1;
		  break;
		case M25P_OP_WRDI:
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  HW_DMSG_M25(M25PNAME ":    starting WRDI command\n");
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  CHECK_WIP;
		  M25P_DATA->command_need_to_complete   = 1;
		  break;
		case M25P_OP_RDSR: 
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  HW_DMSG_M25(M25PNAME ":    starting RDSR command\n");
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  M25P_DATA->command_need_to_complete   = 0;
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_READ, 0);
		  break;
		case M25P_OP_WRSR:
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  HW_DMSG_M25(M25PNAME ":    starting WRSR command\n");
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  M25P_DATA->command_needed_data        = 1;
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_WRITE, 0);
		  break;
		case M25P_OP_READ: 
		  CHECK_WIP;
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  HW_DMSG_M25(M25PNAME ":    starting READ command\n");
		  HW_DMSG_M25(M25PNAME ":    =====================\n");
		  M25P_DATA->command_needed_data        = 3;
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_READ, 0);
		  break;
		case M25P_OP_FAST_READ:
		  CHECK_WIP;
		  HW_DMSG_M25(M25PNAME ":    ==========================\n");
		  HW_DMSG_M25(M25PNAME ":    starting FAST READ command\n");
		  HW_DMSG_M25(M25PNAME ":    ==========================\n");
		  M25P_DATA->command_needed_data        = 4; /* 3 + dummy byte */
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_READ, 0);
		  break;
		case M25P_OP_PP:
		  HW_DMSG_M25(M25PNAME ":    =============================\n");
		  HW_DMSG_M25(M25PNAME ":    starting PAGE PROGRAM command\n");
		  HW_DMSG_M25(M25PNAME ":    =============================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  M25P_DATA->command_needed_data        = 3;
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_WRITE, 0);
		  break;
		case M25P_OP_SE: 
		  HW_DMSG_M25(M25PNAME ":    =============================\n");
		  HW_DMSG_M25(M25PNAME ":    starting SECTOR ERASE command\n");
		  HW_DMSG_M25(M25PNAME ":    =============================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  M25P_DATA->command_needed_data        = 3;
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_WRITE, 0);
		  break;
		case M25P_OP_BE:
		  HW_DMSG_M25(M25PNAME ":    ===========================\n");
		  HW_DMSG_M25(M25PNAME ":    starting BULK ERASE command\n");
		  HW_DMSG_M25(M25PNAME ":    ===========================\n");
		  CHECK_WIP;
		  CHECK_WEL;
		  M25P_DATA->command_need_to_complete   = 1;
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_WRITE, 0);
		  break;
		case M25P_OP_DP:
		  HW_DMSG_M25(M25PNAME ":    ================================\n");
		  HW_DMSG_M25(M25PNAME ":    starting DEEP POWER MODE command\n");
		  HW_DMSG_M25(M25PNAME ":    ================================\n");
		  CHECK_WIP;
		  M25P_DATA->command_need_to_complete   = 1;
		  break;
		case M25P_OP_RES:
		  HW_DMSG_M25(M25PNAME ":    ====================\n");
		  HW_DMSG_M25(M25PNAME ":    starting RES command\n");
		  HW_DMSG_M25(M25PNAME ":    ====================\n");
		  M25P_DATA->command_needed_data        = 3;
		  break;
		default:
		  HW_DMSG_M25(M25PNAME ":    unknown command 0x%02x\n",M25P_DATA->data_buffer);
		  ERROR(M25PNAME ":    unknown command 0x%02x\n",M25P_DATA->data_buffer);
		  break;
		}
	      break; /* switch M25P_OP_NOP */

	      /***********************************************
	       * Instructions have already been started, we 
	       * keep on fetching data until end of instruction
	       * either by self -> command_need_to_complete or
	       * when CS goes low
	       ***********************************************/
	    case M25P_OP_RDSR:
	      M25P_DATA->dummy_write_for_read = 1;
	      break;
	      
	    case M25P_OP_WRSR: 
	      /* =========== */
	      if (M25P_DATA->command_needed_data > 0) 
		{
		  M25P_DATA->command_needed_data = 0;
		  M25P_DATA->command_stored_data[0] = M25P_DATA->data_buffer;
		} 
	      else 
		{
		  M25P_DATA->command_needed_data --;
		  M25P_DATA->command_need_to_complete = 1;
		}
	      break;
	      
	    case M25P_OP_READ:
	      /* =========== */
	      if (M25P_DATA->command_needed_data > 0) 
		{
		  M25P_DATA->command_stored_data[3 - M25P_DATA->command_needed_data] = M25P_DATA->data_buffer;
		  M25P_DATA->command_needed_data --;
		  if (M25P_DATA->command_needed_data == 0)
		    {
		      M25P_DATA->command_pointer = 
			(M25P_DATA->command_stored_data[0] << 16) | 
			(M25P_DATA->command_stored_data[1] <<  8) |
			(M25P_DATA->command_stored_data[2]      );
		      HW_DMSG_M25(M25PNAME ":    read ready at address 0x%06x (page 0x%04x)\n",
				  M25P_DATA->command_pointer,M25P_DATA->command_pointer >> 8);
		    }
		}
	      else
		{
		  // address is ok
		  HW_DMSG_M25(M25PNAME ":    dummy write for read\n");
		  M25P_DATA->command_needed_data --;
		  M25P_DATA->dummy_write_for_read = 1;
		}
	      break;
	      
	    case M25P_OP_FAST_READ:
	      /* =========== */
	      if (M25P_DATA->command_needed_data > 0) 
		{
		  M25P_DATA->command_stored_data[4 - M25P_DATA->command_needed_data] = M25P_DATA->data_buffer;
		  M25P_DATA->command_needed_data --;
		  if (M25P_DATA->command_needed_data == 0)
		    {
		      M25P_DATA->command_pointer = 
			(M25P_DATA->command_stored_data[0] << 16) | 
			(M25P_DATA->command_stored_data[1] <<  8) |
			(M25P_DATA->command_stored_data[2]      );
		      HW_DMSG_M25(M25PNAME ":    FAST Read ready at address 0x%06x (page 0x%04x)\n",
				  M25P_DATA->command_pointer,M25P_DATA->command_pointer >> 8);
		    }
		} 
	      else 
		{
		  // address is ok
		  HW_DMSG_M25(M25PNAME ":    FAST READ dummy write for read\n");
		  M25P_DATA->command_needed_data --;
		  M25P_DATA->dummy_write_for_read = 1;
		}
	      break;
	      
	    case M25P_OP_SE:
	      /* ========= */
	      if (M25P_DATA->command_needed_data > 0) 
		{
		  M25P_DATA->command_stored_data[3 - M25P_DATA->command_needed_data] = M25P_DATA->data_buffer;
		  M25P_DATA->command_needed_data --;
		} 
	      else 
		{
		  M25P_DATA->command_pointer = 
		    (M25P_DATA->command_stored_data[0] << 16) | 
		    (M25P_DATA->command_stored_data[1] <<  8) |
		    (M25P_DATA->command_stored_data[2]      );
		  HW_DMSG_M25(M25PNAME ":    sector erase (linear=%x, sector %x)\n",
			      M25P_DATA->command_pointer, (M25P_DATA->command_pointer >> 16) & 0xff);
		  M25P_DATA->command_need_to_complete = 1;
		}
	      break;
	      
	    case M25P_OP_PP:
	      /* ========= */
	      if (M25P_DATA->command_needed_data > 0) 
		{
		  M25P_DATA->command_stored_data[3 - M25P_DATA->command_needed_data] = M25P_DATA->data_buffer;
		  M25P_DATA->command_needed_data --;
		  if (M25P_DATA->command_needed_data == 0)
		    {
		      M25P_DATA->command_pointer = 
			(M25P_DATA->command_stored_data[0] << 16) | 
			(M25P_DATA->command_stored_data[1] <<  8) |
			(M25P_DATA->command_stored_data[2]      );
		    }
		} 
	      else 
		{
		  // address is ok, we write at the counter address
		  uint16_t page_index;
		  uint16_t page_offset;
		  page_index  = M25P_DATA->command_pointer / M25P_PAGE_SIZE;
		  page_offset = M25P_DATA->command_pointer % M25P_PAGE_SIZE;
		  HW_DMSG_M25(M25PNAME ":    page program (linear=%x, page %x, offset %x)\n",
			      M25P_DATA->command_pointer, page_index, page_offset);
		  M25P_MEMPAGE[page_index][page_offset] &= M25P_DATA->data_buffer;
		  page_offset ++;
		  if (page_offset == M25P_PAGE_SIZE)
		    {
		      M25P_DATA->command_pointer = page_index;
		    }
		  else 
		    {
		      M25P_DATA->command_pointer ++;
		    }
		}
	      break;
	      
	    case M25P_OP_RES:
	      /* ========== */
	      if (M25P_DATA->command_needed_data > 0) 
		{
		  HW_DMSG_M25(M25PNAME ":    RES dummy write %d\n",3 - M25P_DATA->command_needed_data);
		  M25P_DATA->command_stored_data[3 - M25P_DATA->command_needed_data] = M25P_DATA->data_buffer;
		  M25P_DATA->command_needed_data --;
		}
	      else
		{
		  HW_DMSG_M25(M25PNAME ":    RES dummy write for read\n");
		  M25P_DATA->command_needed_data --;
		  M25P_DATA->dummy_write_for_read = 1;
		}
	      break;
	      
	    default: /* switch command */
	      ERROR(M25PNAME ":    internal error command 0x%02x unknown\n",M25P_DATA->data_buffer);
	      /* HW_DMSG_M25(M25PNAME ":    internal error command 0x%02x unknown\n",M25P_DATA->data_buffer);*/
	      m25p_error_dump_internal_state(dev);
	      break;
	    }
	  
	  break;
	  
	  /*********************************************************
	   * End active mode.
	   *
	   * next lines are for other power modes 
	   *********************************************************/
	  
	case M25P_POWER_STANDBY:
	  WARNING(M25PNAME ":    error in power standby, flash should not receive data\n");
	  /* HW_DMSG_M25(M25PNAME ":    error in power standby, flash should not receive data\n"); */
	  break;
	  
	case M25P_POWER_DEEP_DOWN:
	  M25P_DATA->command = M25P_DATA->data_buffer;
	  if (M25P_DATA->command == M25P_OP_RES)
	    {
	      M25P_DATA->data_buffer_ok = 0;
	      M25P_DATA->power_mode = M25P_POWER_ACTIVE;
	      tracer_event_record(TRACER_M25P_STATE, M25P_POWER_ACTIVE);
	      etracer_slot_event(ETRACER_PER_ID_M25P,
				 ETRACER_PER_EVT_MODE_CHANGED,
				 ETRACER_M25P_POWER_ACTIVE, 0);
	      HW_DMSG_M25(M25PNAME ":    Release from Deep Power Down mode\n");
	    }
	  break;
	} /* switch power_mode */
      
    }
  else /* ! need_to_complete */
    {
      if (M25P_DATA->command_need_to_complete == 1)
	{
	  /* ERROR(M25PNAME ":    write data [val=0x%02x,mask=0x%04x] while needed to complete\n",value & M25P_D,mask); */
	  HW_DMSG_M25(M25PNAME ":    write data [val=0x%02x,mask=0x%04x] while needed to complete [PC 0x%4x]\n",
		      value & M25P_D,mask, mcu_get_pc());
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
 * _HOLD
 *
 * _CS
 *   if (1->0)
 *     if (hold == 0)
 *        switch (command)
 *          case XXX: end command
 *        end
 *   else
 *     state = Active
 *   end
 *
 * _WRITE
 * _CLOCK
 */

void m25p_write(int dev, uint32_t mask, uint32_t value)
{
  HW_DMSG_M25(M25PNAME ": mcu --> m25 write 0x%04x mask 0x%04x\n",value, mask);
  
  /********/
  /* Data */
  /********/
  if ((mask & M25P_D))
    {
      if (M25P_DATA->select_bit == 1)
	{
	  etracer_slot_event(ETRACER_PER_ID_M25P,	 ETRACER_PER_EVT_WRITE_COMMAND, 
			     ETRACER_PER_ARG_WR_DST | ETRACER_ACCESS_LVL_SPI1, 0);
	  m25p_write_spidata(dev, mask, value);
	}
      else
	{
#if defined(DEBUG_ME_HARDER)
	  /* debug removed for shared Usart SPI */
	  if (M25P_DATA->hold_bit == 1)
	    {
	      ERROR(M25PNAME ":    write data [val=0x%02x,mask=0x%04x] during hold\n",value & M25P_D,mask);
	      HW_DMSG_M25(M25PNAME ":    write data [val=0x%02x,mask=0x%04x] during hold\n",value & M25P_D,mask);
	    }
	  else
	    { 
	      ERROR(M25PNAME ":    write data [val=0x%02x,mask=0x%04x] without CS (CS=0 too early ?)\n",value & M25P_D,mask);
	      HW_DMSG_M25(M25PNAME ":    write data [val=0x%02x,mask=0x%04x] without CS (CS=0 too early ?)\n",value & M25P_D,mask);
	    }
#endif
	}
    } /* data (mask & M25P_D) */

  /***************************
   * Control pins. HOLD.
   ***************************/

  if (mask & M25P_H) /* hold negated, taken into account when CS */
    {
      int select =  ! (value & M25P_S);
      M25P_DATA->hold_bit   = ! (value & M25P_H);
      if ((M25P_DATA->hold_bit == 1) && (select == 0))
	{
	  HW_DMSG_M25(M25PNAME ":    setting Hold without CS, will hold on next CS\n");
	}
      HW_DMSG_M25(M25PNAME ":    flash write Hold = %d\n",M25P_DATA->hold_bit);
    }

  /***************************
   * Control pins.
   * Chip Select
   ***************************/

#define END_COMMAND()							\
  do {									\
    HW_DMSG_M25(M25PNAME ":    End command 0x%02x (%s)\n",		\
		M25P_DATA->command, str_cmd(M25P_DATA->command));	\
    M25P_DATA->command_need_to_complete = 0;				\
    M25P_DATA->command = M25P_OP_NOP;					\
    HW_DMSG_M25(M25PNAME ":    chip select : Active -> Standby (end of command)\n"); \
  } while (0) 

#define END_COMMAND_GO_STANDBY()					\
  do {									\
    END_COMMAND();							\
    M25P_DATA->power_mode = M25P_POWER_STANDBY;				\
    tracer_event_record(TRACER_M25P_STATE, M25P_POWER_STANDBY);		\
    etracer_slot_event(ETRACER_PER_ID_M25P,				\
		       ETRACER_PER_EVT_MODE_CHANGED,			\
		       ETRACER_M25P_POWER_STANDBY, 0);			\
  } while (0)

  if (mask & M25P_S) /* chip select negated */
    {
      uint8_t bit_cs = ! (value & M25P_S);
      HW_DMSG_M25(M25PNAME ":    flash write CS = %d\n",bit_cs);

      if ((M25P_DATA->select_bit == 1) && (bit_cs == 0) && (M25P_DATA->hold_bit == 0)) 
	{ /* CS: 1 -> 0    CS goes low */
	  M25P_DATA->select_bit = bit_cs;

	  if (M25P_DATA->hold_bit == 0)
	    {
	      switch (M25P_DATA->command)
		{
		case M25P_OP_NOP:
		  break;
		case M25P_OP_WREN: 
		  // M25P_DATA->command_need_to_complete is always set
		  M25P_DATA->status_register.b.wel    = 1; 
		  END_COMMAND_GO_STANDBY();
		  break;
		case M25P_OP_WRDI:
		  // M25P_DATA->command_need_to_complete is always set
		  M25P_DATA->status_register.b.wel    = 0;
		  END_COMMAND_GO_STANDBY();
		  break;
		case M25P_OP_RDSR:
		  // M25P_DATA->command_need_to_complete is NOT set
		  END_COMMAND_GO_STANDBY();
		  break;
		case M25P_OP_WRSR:
		  // M25P_DATA->command_need_to_complete MUST be set
		  CHECK_COMMAND_ARGS;
		  if (M25P_DATA->command_need_to_complete == 1) {
		    m25p_write_status_register(dev,M25P_DATA->command_stored_data[0]);
		  }
		  END_COMMAND_GO_STANDBY();
		  break;
		case M25P_OP_READ:
		  // M25P_DATA->command_need_to_complete is NOT set
		  CHECK_COMMAND_ARGS;
		  END_COMMAND_GO_STANDBY();
		  break;
		case M25P_OP_FAST_READ:
		  // M25P_DATA->command_need_to_complete is NOT set
		  CHECK_COMMAND_ARGS;
		  END_COMMAND_GO_STANDBY();
		  break;
		case M25P_OP_PP: 
		  // M25P_DATA->command_need_to_complete MIGHT be set
		  // M25P_DATA->command_needed_data == 0 ?
		  CHECK_COMMAND_ARGS;
		  M25P_DATA->status_register.b.wel    = 0;
		  END_COMMAND();
		  /* wait busy flag, mode change in update */
		  break;
		case M25P_OP_SE: 
		  // M25P_DATA->command_need_to_complete MUST be set
		  CHECK_COMMAND_ARGS;
		  m25p_erase_sector(dev,M25P_DATA->command_pointer); 
		  M25P_DATA->status_register.b.wel    = 0;
		  END_COMMAND();
		  /* wait busy flag, mode change in update */
		  break;
		case M25P_OP_BE: 
		  // M25P_DATA->command_need_to_complete is always set
		  m25p_erase_bulk(dev);  
		  M25P_DATA->status_register.b.wel    = 0;
		  END_COMMAND();
		  /* wait busy flag, mode change in update */
		  break;
		case M25P_OP_DP:
		  // M25P_DATA->command_need_to_complete is always set
		  END_COMMAND();
		  HW_DMSG_M25(M25PNAME ":    Power mode changed to DEEP POWER DOWN\n");	
		  M25P_DATA->power_mode = M25P_POWER_DEEP_DOWN;
		  tracer_event_record(TRACER_M25P_STATE, M25P_POWER_DEEP_DOWN);
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_DEEP_DOWN, 0);
		  break;
		case M25P_OP_RES:
		  // M25P_DATA->command_need_to_complete MIGHT be set
		  // M25P_DATA->command_needed_data is variable		 
		  END_COMMAND();
		  HW_DMSG_M25(M25PNAME ":    Power mode changed to ACTIVE\n");
		  M25P_DATA->power_mode = M25P_POWER_ACTIVE;
		  tracer_event_record(TRACER_M25P_STATE, M25P_POWER_ACTIVE);
		  etracer_slot_event(ETRACER_PER_ID_M25P,
				     ETRACER_PER_EVT_MODE_CHANGED,
				     ETRACER_M25P_POWER_ACTIVE, 0);
		  break;
		default:
		  ERROR(M25PNAME ":    Unterminated switch/case for end command CS=0\n");
		  HW_DMSG_M25(M25PNAME ":    Unterminated switch/case for end command CS=0\n");
		  END_COMMAND();
		  break;
		} /* switch (M25P_DATA->command) */
	    } /* hold_bit == 0 */
	  else /* if hold_bit == 1 */
	    {
	      HW_DMSG_M25(M25PNAME ":    flash write CS = 0, Active -> On Hold\n");
	    }
	} /* CS goes low */
      else if ((M25P_DATA->select_bit == 0) && (bit_cs == 1)) 
	{ /* CS: 0 -> 1,  CS goes high */
	  M25P_DATA->select_bit = bit_cs;
	  if (M25P_DATA->hold_bit == 0)
	    {
	      HW_DMSG_M25(M25PNAME ":    flash write CS = 1 : Standby -> Active\n");
	      M25P_DATA->power_mode = M25P_POWER_ACTIVE;
	      tracer_event_record(TRACER_M25P_STATE, M25P_POWER_ACTIVE);
	      etracer_slot_event(ETRACER_PER_ID_M25P,
				 ETRACER_PER_EVT_MODE_CHANGED,
				 ETRACER_M25P_POWER_ACTIVE, 0);
	    }
	  else
	    {
	      HW_DMSG_M25(M25PNAME ":    flash write CS = 1 : On Hold -> Active\n");
	      if (M25P_DATA->power_mode != M25P_POWER_ACTIVE)
		HW_DMSG_M25(M25PNAME ":    ** power state on Hold was not active **\n");
	    }
	}
    } /* mask & M25P_S */ 

  /***************************
   * Control pins. WRITE PROTECT
   ***************************/

  if (mask & M25P_W) /* write protect netgated */
    {
      M25P_DATA->write_protect_bit  = ! (value & M25P_W);
      HW_DMSG_M25(M25PNAME ":    flash write protect W = %d\n",M25P_DATA->write_protect_bit);
    }

  /***************************
   * Control pins. CLOCK
   ***************************/

  if (mask & M25P_C) /* clock */
    {
      ERROR(M25PNAME ":    clock pin should not be used during simulation\n");
      HW_DMSG_M25(M25PNAME ":    clock pin should not be used during simulation\n");
      M25P_DATA->clock = (value >> M25P_C_SHIFT) & 0x1;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static int m25p_update_write_flag(int dev)
{
  if ((M25P_DATA->status_register.b.wip == 1) && (MACHINE_TIME_GET_NANO() >= M25P_DATA->end_of_busy_time))
    {
      M25P_DATA->status_register.b.wip = 0;
      /* M25P_DATA->status_register.b.wel = 0; ok pour WRSR */
      M25P_DATA->command = M25P_OP_NOP;
      HW_DMSG_M25(M25PNAME ":    ====================================\n");
      HW_DMSG_M25(M25PNAME ":    M25P display busyflag returns to 0\n");
      HW_DMSG_M25(M25PNAME ":    ====================================\n");
      return 1;
    }
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_update(int dev)
{
  switch (M25P_DATA->power_mode)
    {
      /**********************/
      /* Active poser mode  */
      /**********************/
    case M25P_POWER_ACTIVE:
      /* end current command ? */
      if (m25p_update_write_flag(dev))
	{
	  /* busyflag returned to 0 : end of current command */
	  if ((M25P_DATA->select_bit == 0) && (M25P_DATA->hold_bit == 0))
	    {
	      /* going from active to standby */
	      HW_DMSG_M25(M25PNAME ":    UPDATE end of command wip=0: Active -> Standby\n");
	      tracer_event_record(TRACER_M25P_STATE, M25P_POWER_STANDBY);
	      etracer_slot_event(ETRACER_PER_ID_M25P,
				 ETRACER_PER_EVT_MODE_CHANGED,
				 ETRACER_M25P_POWER_STANDBY, 0);

	    }
	}
      break;

      /**********************/
      /* Standby power mode */
      /**********************/
    case M25P_POWER_STANDBY:
      /* nothing to do                             */
      /* state is changed in the write switch      */
      break;

      /**********************/
      /* Deep power down    */
      /**********************/
    case M25P_POWER_DEEP_DOWN:
      /* nothing to do                             */
      /* going out of DP is done in the write loop */
      break;
    }
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int m25p_ui_draw      (int UNUSED dev)
{
  return 0;
}

void m25p_ui_get_size (int UNUSED dev, int *w, int *h)
{
  w = 0;
  h = 0;
}

void m25p_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y)
{
  
}

void m25p_ui_get_pos  (int UNUSED dev, int *x, int *y)
{
  *x = 0;
  *y = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/
