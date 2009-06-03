
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

#ifdef DEBUG
#define HW_DMSG_AT45(x...) HW_DMSG(x)
#else
#define HW_DMSG_AT45(x...) do {} while(0)
#endif


/***************************************************/
/** Flash internal data ****************************/
/***************************************************/

#define NANO   (1)
#define MICRO  (1000)
#define MILLI  (1000 * 1000)
#define SECOND (1000 * 1000 * 1000)

#define AT45_MAX_COMMAND_DATA 8

#if defined(AT45DB041B)
#define AT45_PAGE_MAX             2048
#define AT45_PAGE_SIZE            264
#define AT45_FLASH_SIZE           (AT45_PAGE_MAX * AT45_PAGE_SIZE) 
#define AT45_BLOCK_MAX            256
#define AT45_PAGE_PER_BLOCK       8
#define AT45_HIGH_ADDR_MASK       0x0f     /* 4 bits unused */
#define AT45_DENSITY_BIT_0        1
#define AT45_DENSITY_BIT_1        1
#define AT45_DENSITY_BIT_2        1
#define AT45_DENSITY_BIT_3        0

#define AT45_TIME_RESET        (20  * MILLI)
#define AT45_TIME_TXFR         (250 * MICRO) /* transfer        */
#define AT45_TIME_TEP          (20  * MILLI) /* erase + program */
#define AT45_TIME_TP           (14  * MILLI) /* program         */
#define AT45_TIME_TPE          (8   * MILLI) /* page erase      */
#define AT45_TIME_TBE          (12  * MILLI) /* block erase     */
#else
#error "you must define a specific Atmel flash model"
#endif

/*
 * BFA8 - BFA0            == buffer address
 * PA10 - PA0 + BA8 - BA0 == page address + byte address
 * 
 * this model considers that all 4 access modes are equal
 *     Inactive Clock Polarity Low or Inactive Clock Polarity High and SPI Mode 0 or SPI Mode 3.
 * 
 * 
 * read commands:
 *      continuous   == 68|E8 + 24bits + 32bits_dont_care
 *                             24 bits == xxx+11bits+9bits
 *      page read    == 52|D2 + 24b(4+11+9)           + 32b
 *      buffer1 read == 54|D4 + 15b don'tcare + 9bits + 8b
 *      buffer2 read == 56|D6 + 15b don'tcare + 9bits + 8b
 *      SR           == 57|D7 + repeat
 *
 * erase commands:
 *    SI --> bufferx
 *      write buffer1 == 84 + 15b don'tcare + 9bits
 *      write buffer2 == 87 + 15b don'tcare + 9bits
 *    Bufferx --> Page + builtin erase
 *      buffer1       == 83 + 4+11bits + 9b don'tcare (time TEP)
 *      buffer2       == 86 + 4+11bits + 9b don'tcare (time TEP)
 *    Bufferx --> Page without erase
 *      buffer1       == 88 + 4+11bits + 9b don'tcare (time TP)
 *      buffer2       == 89 + 4+11bits + 9b don'tcare (time TP)
 *    
 *    Page erase  == 81 + 4 + 11bits + 9b don'tcare   (time TPE)
 *    Block erase == 50 + 4 + 8bits + 12bdon'tcare    (time TBE)
 * 
 *  Buffer + page program with erase
 *    buffer1 == 82 + 4+20, writing at end of command       (buffer write + time TEP)
 *    buffer2 == 85
 *
 * Copy:
 *    page to buffer1 == 53 + 4+11+9don'tcare (time TXFR)
 *    page to buffer2 == 55 + 4+11+9don'tcare (time TXFR)
 * 
 * Compare:
 *    page to buffer1 == 60 + 4+11+9don'tcare (time TXFR)
 *    page to buffer2 == 61 + 4+11+9don'tcare (time TXFR)
 *
 * Multiple rewrite
 *    combo :: page to buffer + buffer to page with built in erase
 *      buffer1 == 58 + 4+11+9don'tcare  (time TEP)
 *      buffer2 == 59 + 4+11+9don'tcare  (time TEP)

  write()
    updates internal data

  update()
    switch (op)
    updates read value;

  read()
    sends data to mcu

  update_cycle();
 */


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed))  at45_status_t {
  uint8_t
    ready:1,    /* MSB */
    comp:1,
    density3:1,
    density2:1,
    density1:1,
    density0:1,
    unused1:1,
    unused0:1;
};
#else
struct __attribute__ ((packed)) at45_status_t {
  uint8_t
    unused0:1,
    unused1:1,
    density0:1,
    density1:1,
    density2:1,
    density3:1,
    comp:1,
    ready:1;
};
#endif

enum at45_opcode_t {
  AT45_OP_NOP              = 0x00,  /* nop                   */

  /* read */
  AT45_OP_CONTREAD_1       = 0x68,
  AT45_OP_CONTREAD_2       = 0xE8,
  AT45_OP_PAGE_READ_1      = 0x52,
  AT45_OP_PAGE_READ_2      = 0xD2,
  AT45_OP_BUFFER1_READ_1   = 0x54,
  AT45_OP_BUFFER1_READ_2   = 0xD4,
  AT45_OP_BUFFER2_READ_1   = 0x56,
  AT45_OP_BUFFER2_READ_2   = 0xD6,
  AT45_OP_STATUS_READ_1    = 0x57,
  AT45_OP_STATUS_READ_2    = 0xD7,

  /* erase */
  AT45_OP_SI_TO_BUFFER1    = 0x84,
  AT45_OP_SI_TO_BUFFER2    = 0x87,

  AT45_OP_BUFFER1_WI_ERASE = 0x83,
  AT45_OP_BUFFER2_WI_ERASE = 0x86,
  AT45_OP_BUFFER1_WO_ERASE = 0x88,
  AT45_OP_BUFFER2_WO_ERASE = 0x89,

  AT45_OP_PAGE_ERASE       = 0x81,
  AT45_OP_BLOCK_ERASE      = 0x50,

  /* erase buffer + page program with erase */
  AT45_OP_SI_BUFFER1_PPE   = 0x82,
  AT45_OP_SI_BUFFER2_PPE   = 0x85,

  /* copy / compare */
  AT45_OP_COPY_BUFFER1     = 0x53,
  AT45_OP_COPY_BUFFER2     = 0x55,
  AT45_OP_COMP_BUFFER1     = 0x60,
  AT45_OP_COMP_BUFFER2     = 0x61,

  /* multiple rewrite */
  AT45_OP_MULTI_BUFFER1    = 0x58,
  AT45_OP_MULTI_BUFFER2    = 0x59
};

const char* str_cmd(int n)
{
  switch (n)
    {
    case AT45_OP_NOP             : return "AT45_OP_NOP"              ;

      /* read */
    case AT45_OP_CONTREAD_1      : return "AT45_OP_CONTREAD"         ;
    case AT45_OP_CONTREAD_2      : return "AT45_OP_CONTREAD"         ;
    case AT45_OP_PAGE_READ_1     : return "AT45_OP_PAGE_READ"        ;
    case AT45_OP_PAGE_READ_2     : return "AT45_OP_PAGE_READ"        ;
    case AT45_OP_BUFFER1_READ_1  : return "AT45_OP_BUFFER1_READ"     ;
    case AT45_OP_BUFFER1_READ_2  : return "AT45_OP_BUFFER1_READ"     ;
    case AT45_OP_BUFFER2_READ_1  : return "AT45_OP_BUFFER2_READ"     ;
    case AT45_OP_BUFFER2_READ_2  : return "AT45_OP_BUFFER2_READ"     ;
    case AT45_OP_STATUS_READ_1   : return "AT45_OP_STATUS_READ"      ;
    case AT45_OP_STATUS_READ_2   : return "AT45_OP_STATUS_READ"      ;

      /* erase */
    case AT45_OP_SI_TO_BUFFER1   : return "AT45_OP_SI_TO_BUFFER1"    ;
    case AT45_OP_SI_TO_BUFFER2   : return "AT45_OP_SI_TO_BUFFER2"    ;

    case AT45_OP_BUFFER1_WI_ERASE: return "AT45_OP_BUFFER1_WI_ERASE" ;
    case AT45_OP_BUFFER2_WI_ERASE: return "AT45_OP_BUFFER2_WI_ERASE" ;
    case AT45_OP_BUFFER1_WO_ERASE: return "AT45_OP_BUFFER1_WO_ERASE" ;
    case AT45_OP_BUFFER2_WO_ERASE: return "AT45_OP_BUFFER1_WO_ERASE" ;

    case AT45_OP_PAGE_ERASE      : return "AT45_OP_PAGE_ERASE"       ;
    case AT45_OP_BLOCK_ERASE     : return "AT45_OP_BLOCK_ERASE"      ;

      /* erase buffer + page program with erase */
    case AT45_OP_SI_BUFFER1_PPE  : return "AT45_OP_SI_BUFFER1_PPE"   ;
    case AT45_OP_SI_BUFFER2_PPE  : return "AT45_OP_SI_BUFFER2_PPE"   ;

      /* copy / compare */
    case AT45_OP_COPY_BUFFER1    : return "AT45_OP_COPY_BUFFER1"     ;
    case AT45_OP_COPY_BUFFER2    : return "AT45_OP_COPY_BUFFER2"     ;
    case AT45_OP_COMP_BUFFER1    : return "AT45_OP_COMP_BUFFER1"     ;
    case AT45_OP_COMP_BUFFER2    : return "AT45_OP_COMP_BUFFER2"     ;

      /* multiple rewrite */
    case AT45_OP_MULTI_BUFFER1   : return "AT45_OP_MULTI_BUFFER1"    ;
    case AT45_OP_MULTI_BUFFER2   : return "AT45_OP_MULTI_BUFFER2"    ;

    default:               return "UNKNOWN";
    }
  return "UNKNOWN";
}

int at45db_needed_data(int cmd)
{
  switch (cmd) {
  case AT45_OP_NOP               : return 0;
  case AT45_OP_CONTREAD_1        :
  case AT45_OP_CONTREAD_2        : return 3+4;
  case AT45_OP_PAGE_READ_1       :
  case AT45_OP_PAGE_READ_2       : return 3+4;
  case AT45_OP_BUFFER1_READ_1    :
  case AT45_OP_BUFFER1_READ_2    : return 3;
  case AT45_OP_BUFFER2_READ_1    :
  case AT45_OP_BUFFER2_READ_2    : return 3;
  case AT45_OP_STATUS_READ_1     :
  case AT45_OP_STATUS_READ_2     : return 0;
  case AT45_OP_SI_TO_BUFFER1     : 
  case AT45_OP_SI_TO_BUFFER2     : return 3;
  case AT45_OP_BUFFER1_WI_ERASE  : 
  case AT45_OP_BUFFER2_WI_ERASE  : return 3;
  case AT45_OP_BUFFER1_WO_ERASE  : 
  case AT45_OP_BUFFER2_WO_ERASE  : return 3;
  case AT45_OP_PAGE_ERASE        : return 3;
  case AT45_OP_BLOCK_ERASE       : return 3;
  case AT45_OP_SI_BUFFER1_PPE    : 
  case AT45_OP_SI_BUFFER2_PPE    : return 3;
  case AT45_OP_COPY_BUFFER1      : 
  case AT45_OP_COPY_BUFFER2      : return 3;
  case AT45_OP_COMP_BUFFER1      : 
  case AT45_OP_COMP_BUFFER2      : return 3;
  case AT45_OP_MULTI_BUFFER1     : 
  case AT45_OP_MULTI_BUFFER2     : return 3;
  default:               return 0;
  }
  return 0;
}

int at45db_need_to_complete(int cmd)
{
  switch (cmd) {
  case AT45_OP_NOP               : return 0;
  case AT45_OP_CONTREAD_1        :
  case AT45_OP_CONTREAD_2        : return 0;
  case AT45_OP_PAGE_READ_1       :
  case AT45_OP_PAGE_READ_2       : return 0;
  case AT45_OP_BUFFER1_READ_1    :
  case AT45_OP_BUFFER1_READ_2    : return 0;
  case AT45_OP_BUFFER2_READ_1    :
  case AT45_OP_BUFFER2_READ_2    : return 0;
  case AT45_OP_STATUS_READ_1     :
  case AT45_OP_STATUS_READ_2     : return 0;
  case AT45_OP_SI_TO_BUFFER1     : 
  case AT45_OP_SI_TO_BUFFER2     : return 0;
  case AT45_OP_BUFFER1_WI_ERASE  : 
  case AT45_OP_BUFFER2_WI_ERASE  : return 1;
  case AT45_OP_BUFFER1_WO_ERASE  : 
  case AT45_OP_BUFFER2_WO_ERASE  : return 1;
  case AT45_OP_PAGE_ERASE        : return 1;
  case AT45_OP_BLOCK_ERASE       : return 1;
  case AT45_OP_SI_BUFFER1_PPE    : 
  case AT45_OP_SI_BUFFER2_PPE    : return 1;
  case AT45_OP_COPY_BUFFER1      : 
  case AT45_OP_COPY_BUFFER2      : return 1;
  case AT45_OP_COMP_BUFFER1      : 
  case AT45_OP_COMP_BUFFER2      : return 1;
  case AT45_OP_MULTI_BUFFER1     : 
  case AT45_OP_MULTI_BUFFER2     : return 0;
  default:               return 0;
  }
  return 0;
}

int at45db_busy_time(int cmd)
{
  switch (cmd) {
  case AT45_OP_NOP               : return 0;
  case AT45_OP_CONTREAD_1        :
  case AT45_OP_CONTREAD_2        : return 0;
  case AT45_OP_PAGE_READ_1       :
  case AT45_OP_PAGE_READ_2       : return 0;
  case AT45_OP_BUFFER1_READ_1    :
  case AT45_OP_BUFFER1_READ_2    : return 0;
  case AT45_OP_BUFFER2_READ_1    :
  case AT45_OP_BUFFER2_READ_2    : return 0;
  case AT45_OP_STATUS_READ_1     :
  case AT45_OP_STATUS_READ_2     : return 0;
  case AT45_OP_SI_TO_BUFFER1     : 
  case AT45_OP_SI_TO_BUFFER2     : return 0;
  case AT45_OP_BUFFER1_WI_ERASE  : 
  case AT45_OP_BUFFER2_WI_ERASE  : return AT45_TIME_TEP;
  case AT45_OP_BUFFER1_WO_ERASE  : 
  case AT45_OP_BUFFER2_WO_ERASE  : return AT45_TIME_TP;
  case AT45_OP_PAGE_ERASE        : return AT45_TIME_TPE;
  case AT45_OP_BLOCK_ERASE       : return AT45_TIME_TBE;
  case AT45_OP_SI_BUFFER1_PPE    : 
  case AT45_OP_SI_BUFFER2_PPE    : return AT45_TIME_TEP;
  case AT45_OP_COPY_BUFFER1      : 
  case AT45_OP_COPY_BUFFER2      : return AT45_TIME_TXFR;
  case AT45_OP_COMP_BUFFER1      : 
  case AT45_OP_COMP_BUFFER2      : return AT45_TIME_TXFR;
  case AT45_OP_MULTI_BUFFER1     : 
  case AT45_OP_MULTI_BUFFER2     : return AT45_TIME_TEP;
  default:               return 0;
  }
  return 0;
}

struct at45db_t 
{
  union {
    struct at45_status_t b;
    uint8_t              s;
  } status_register;

  uint8_t pin_select;           /* IN: chip select   */
  uint8_t pin_write_protect;    /* IN: write protect */
  uint8_t pin_reset;            /* IN:  */
  //  uint8_t pin_ready;            /* OUT: */

  uint8_t buffer1[AT45_PAGE_SIZE];
  uint8_t buffer2[AT45_PAGE_SIZE];

  union {
    uint8_t raw   [AT45_FLASH_SIZE];
    uint8_t page  [AT45_PAGE_MAX][AT45_PAGE_SIZE];
  } mem;

  enum at45_opcode_t   command;  /* current command */
  int                  command_next;
  int                  command_needed_data;
  uint32_t             command_need_to_complete;
  uint32_t             command_stored_data[AT45_MAX_COMMAND_DATA];
  uint32_t             command_page_addr;
  uint32_t             command_addr;
  int                  command_start;

  /* data just written */
  uint32_t              write_mask;
  uint32_t              write_data;
  uint8_t               write_byte_value;
  uint8_t               write_byte_valid;

  /* data to be read */
  uint32_t              read_mask;
  uint32_t              read_data;

  /* busy timing */
  uint64_t              busy_time;
  uint64_t              end_of_busy_time;

  int                   compare_value;
  int                   compare_update;

  /* file names */
  char                * file_init;
  char                * file_dump;
};

#define AT45_DATA        ((struct at45db_t*)(machine.device[dev].data))
#define AT45_SR          (AT45_DATA->status_register)
#define AT45_MEMRAW      (AT45_DATA->mem.raw   )
#define AT45_MEMSECTOR   (AT45_DATA->mem.sector)
#define AT45_MEMPAGE     (AT45_DATA->mem.page  )
#define AT45_INIT        (AT45_DATA->file_init )
#define AT45_DUMP        (AT45_DATA->file_dump )

#define AT45_READ_MASK   (AT45_DATA->read_mask)
#define AT45_READ_DATA   (AT45_DATA->read_data)
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
int  at45db_update_cycle(int dev);
int  at45db_ui_draw     (int dev);
void at45db_ui_get_size (int dev, int *w, int *h);
void at45db_ui_set_pos  (int dev, int  x, int  y);
void at45db_ui_get_pos  (int dev, int *x, int *y);

void at45db_write_buffer_with_erase(int dev, uint8_t *buffer);
void at45db_write_buffer_without_erase(int dev, uint8_t *buffer);

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

tracer_id_t TRACER_AT45DB_STATE;

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

  if ((f = fopen(name, "rb")) == NULL)
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

  if ((f = fopen(name, "wb")) == NULL)
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
  machine.device[dev].update        = at45db_update_cycle;

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

  //  TRACER_AT45DB_STATE = tracer_event_add_id(8, "state" , "at45db");

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_reset(int dev)
{
  HW_DMSG_AT45("at45db: flash reset\n");
  AT45_DATA->command                  = AT45_OP_NOP;
  AT45_DATA->busy_time                = 0;
  AT45_DATA->end_of_busy_time         = 0;
  AT45_DATA->command_need_to_complete = 0;
  AT45_DATA->compare_update           = 0;
  AT45_DATA->status_register.s        = 0;
  AT45_DATA->write_mask               = 0;
  AT45_DATA->write_byte_valid         = 0;
  AT45_DATA->read_mask                = 0;

  AT45_SR.b.ready    = 1;
  AT45_SR.b.comp     = 0;
  AT45_SR.b.density3 = AT45_DENSITY_BIT_3;
  AT45_SR.b.density2 = AT45_DENSITY_BIT_2;
  AT45_SR.b.density1 = AT45_DENSITY_BIT_1;
  AT45_SR.b.density0 = AT45_DENSITY_BIT_0;
  AT45_SR.b.unused1  = 0;
  AT45_SR.b.unused0  = 0;

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

  return 0;
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

static void at45db_set_busy_time(int dev, uint64_t nano)
{
  AT45_DATA->busy_time = nano;
}

static void at45db_start_write(int dev)
{
  AT45_DATA->status_register.b.ready = 0;
  AT45_DATA->end_of_busy_time        = MACHINE_TIME_GET_NANO() + AT45_DATA->busy_time;
  HW_DMSG_AT45("at45db:    ============================\n");
  HW_DMSG_AT45("at45db:    AT45DB busyflag start write \n");
  HW_DMSG_AT45("at45db:    ============================\n");
}

static int at45db_update_write_flag(int dev)
{
  if ((AT45_DATA->status_register.b.ready == 0) && (MACHINE_TIME_GET_NANO() >= AT45_DATA->end_of_busy_time))
    {
      AT45_DATA->status_register.b.ready = 1;
      AT45_DATA->busy_time               = 0;
      AT45_DATA->end_of_busy_time        = 0;
      HW_DMSG_AT45("at45db:    ============================\n");
      HW_DMSG_AT45("at45db:    AT45DB busyflag returns to 0\n");
      HW_DMSG_AT45("at45db:    ============================\n");
      if (AT45_DATA->compare_update)
	{
	  /* 0 means page and buffer are equal */
	  AT45_SR.b.comp = (AT45_DATA->compare_value > 0);
	  AT45_DATA->compare_update = 0;
	}
      return 1;
    }
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_read(int UNUSED dev, uint32_t *mask, uint32_t *value)
{
  *mask  = AT45_READ_MASK;
  *value = AT45_READ_DATA;

  AT45_DATA->write_byte_valid = 0;

  /*
  if (*mask != 0)
    {
      HW_DMSG_AT45("at45db:    read data [val=0x%02x,mask=0x%04x] \n", *value, *mask);
    }
  */
  if (AT45_SR.b.ready)
    {
      *mask  |= AT45DB_RDY;
      *value |= AT45DB_RDY;
    }

  AT45_READ_MASK = 0; 
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

static
int check_pin(char UNUSED *msg, uint32_t mask, uint32_t value, int rshift, uint8_t *current_val)
{
  if ((mask >> rshift) & 1)
    {
      uint8_t pin = (value >> rshift) & 1;
      if ((*current_val) != pin)
	{
	  // HW_DMSG_AT45("at45db:      pin %s set to %d\n",msg,pin);
	  *current_val = pin;
	  return 1;
	}
    }
  return 0;
}


void at45db_write(int UNUSED dev, uint32_t mask, uint32_t value)
{
  int CS,W,R; /* modified flags */
  // HW_DMSG_AT45("at45db: write to flash 0x%04x mask 0x%04x\n",value, mask);
  
  CS = check_pin("CS",    mask,value,AT45DB_S_SHIFT, & AT45_DATA->pin_select);
  W  = check_pin("W",     mask,value,AT45DB_W_SHIFT, & AT45_DATA->pin_write_protect);
  R  = check_pin("Reset", mask,value,AT45DB_R_SHIFT, & AT45_DATA->pin_reset);

  /*********/
  /* DATA  */
  /*********/
  AT45_DATA->write_byte_valid = 0;
  if ((mask & AT45DB_D) == AT45DB_D)
    {
      if (AT45_DATA->pin_select == 0) /* CS is negated */
	{
	  AT45_DATA->write_byte_value = value & AT45DB_D;
	  AT45_DATA->write_byte_valid = 1;
	  // HW_DMSG_AT45("at45db:    write byte 0x%02x\n", AT45_DATA->write_byte_value);
	  if (AT45_DATA->command_need_to_complete)
	    {
	      HW_DMSG_AT45("at45db: command should be validated \n");
	    }
	}
    }

  /*********/
  /* WP    */
  /*********/
  if (W)
    {
      HW_DMSG_AT45("at45db: ** WRITE PROTECT IS %s **\n",(AT45_DATA->pin_write_protect == 0)?"ON":"OFF");
    }
  
  /*********/
  /* RESET */
  /*********/
  if (R && AT45_DATA->pin_reset == 0)
    {
      HW_DMSG_AT45("at45db: ** RESET **\n");
      at45db_reset(dev);
    }

  /*********/
  /* CS    */
  /*********/
  if (AT45_DATA->pin_select != 0) /* CS is negated, we're off */
    {
      switch (AT45_DATA->command) 
	{
	case AT45_OP_SI_BUFFER1_PPE:
	  at45db_write_buffer_with_erase(dev,AT45_DATA->buffer1);
	  break;
	case AT45_OP_SI_BUFFER2_PPE:
	  at45db_write_buffer_with_erase(dev,AT45_DATA->buffer2);
	  break;
	default:
	  break;
	}

      if ((AT45_DATA->busy_time != 0) || (AT45_DATA->command_need_to_complete))
	{
	  at45db_start_write(dev);
	  AT45_DATA->command_need_to_complete = 0;
	}

      AT45_DATA->command                  = AT45_OP_NOP;
      AT45_DATA->command_needed_data      = 0;
      AT45_DATA->command_need_to_complete = 0;
      AT45_DATA->write_byte_valid         = 0;
    }

  at45db_update(dev);
}

/***************************************************/
/***************************************************/
/***************************************************/

/* 4 + 11 + 9 :: byte address */   
/* 0000 pppp ppppppb bbbbbbbb */

inline void READ_PAGE_ADDRESS(int dev)
{
  AT45_DATA->command_page_addr  = (AT45_DATA->command_stored_data[1] >> 1) & 0x7F;
  AT45_DATA->command_page_addr |= (AT45_DATA->command_stored_data[0] & AT45_HIGH_ADDR_MASK) << 7;
  AT45_DATA->command_page_addr %= AT45_PAGE_MAX;
}

inline void READ_BYTE_ADDRESS(int dev)
{
  AT45_DATA->command_addr       =  AT45_DATA->command_stored_data[2];
  AT45_DATA->command_addr      |= (AT45_DATA->command_stored_data[1] & 0x01) <<  8;
  AT45_DATA->command_addr      %= AT45_PAGE_SIZE;
}

inline void READ_LINEAR_ADDRESS(int dev)
{
  AT45_DATA->command_addr       = /* byte address */
    ((AT45_DATA->command_stored_data[0] & AT45_HIGH_ADDR_MASK)  << 16) |
    ((AT45_DATA->command_stored_data[1]                      )  <<  8) |
    ((AT45_DATA->command_stored_data[2]                      )  <<  0);
  AT45_DATA->command_addr      %= AT45_FLASH_SIZE; 
}

inline void READ_BLOCK_ADDRESS(int dev)
{
  READ_PAGE_ADDRESS(dev);
  AT45_DATA->command_page_addr = AT45_DATA->command_page_addr >> 3; /* block number */
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_read_from_buffer(int dev, uint8_t *buffer)
{
  if (AT45_DATA->command_start)
    {
      READ_BYTE_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }
  HW_DMSG_AT45("at45db: %s at address 0x%04x\n",str_cmd(AT45_DATA->command),AT45_DATA->command_addr);
  AT45_READ_MASK = AT45DB_D; 
  AT45_READ_DATA = buffer[AT45_DATA->command_addr];
  AT45_DATA->command_addr = (AT45_DATA->command_addr + 1) % AT45_PAGE_SIZE; 
}

/***************************************************/
/***************************************************/
/***************************************************/

      /*           buffer
       *           |  0   1
       *         ----------
       *         0 |  0   0
       * flash     |
       *         1 |  0   1
       *
       */

void at45db_write_buffer_without_erase(int dev, uint8_t *buffer)
{
  if (AT45_DATA->command_start)
    {
      READ_PAGE_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }
  HW_DMSG_AT45("at45db: %s at page 0x%04x\n", str_cmd(AT45_DATA->command), AT45_DATA->command_page_addr);
  for(AT45_DATA->command_addr=0; AT45_DATA->command_addr < AT45_PAGE_SIZE; AT45_DATA->command_addr++)
    { /* we write 0 */
      AT45_DATA->mem.page[AT45_DATA->command_page_addr][AT45_DATA->command_addr] &= 
	buffer[AT45_DATA->command_addr];
    }
  at45db_set_busy_time(dev,at45db_busy_time(AT45_DATA->command));
  AT45_DATA->command_need_to_complete = 1;
  AT45_READ_MASK = AT45DB_D;
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_write_buffer_with_erase(int dev, uint8_t *buffer)
{
  if (AT45_DATA->command_start)
    {
      READ_PAGE_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }
  HW_DMSG_AT45("at45db: %s at page 0x%04x\n", str_cmd(AT45_DATA->command), AT45_DATA->command_page_addr);
  memcpy(AT45_DATA->mem.page[AT45_DATA->command_page_addr], buffer ,AT45_PAGE_SIZE);
  at45db_set_busy_time(dev,at45db_busy_time(AT45_DATA->command));
  AT45_DATA->command_need_to_complete = 1;
  AT45_READ_MASK = AT45DB_D;
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_write_si_to_buffer(int dev, uint8_t *buffer)
{
  if (AT45_DATA->command_start)
    {
      READ_PAGE_ADDRESS(dev);
      READ_BYTE_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }
  HW_DMSG_AT45("at45db: %s at address 0x%04x\n",str_cmd(AT45_DATA->command),AT45_DATA->command_addr);
  buffer[AT45_DATA->command_addr] = AT45_DATA->write_byte_value;
  AT45_DATA->command_addr = (AT45_DATA->command_addr + 1) % AT45_PAGE_SIZE; 
  AT45_READ_MASK = AT45DB_D; 
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_block_erase(int dev)
{
  int i;
  if (AT45_DATA->command_start)
    {
      READ_BLOCK_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }

  for(i=0; i < AT45_PAGE_PER_BLOCK; i++)
    {
      memset(AT45_DATA->mem.page[AT45_DATA->command_page_addr + i], 1, AT45_FLASH_SIZE);
    }

  at45db_set_busy_time(dev,at45db_busy_time(AT45_DATA->command));
  AT45_DATA->command_need_to_complete = 1;
  AT45_READ_MASK = AT45DB_D;
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_copy_page_to_buffer(int dev, uint8_t *buffer)
{
  if (AT45_DATA->command_start)
    {
      READ_PAGE_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }
  HW_DMSG_AT45("at45db: %s for page 0x%04x\n", str_cmd(AT45_DATA->command), AT45_DATA->command_page_addr);
  memcpy(buffer, AT45_DATA->mem.page[AT45_DATA->command_page_addr], AT45_PAGE_SIZE);
  at45db_set_busy_time(dev,at45db_busy_time(AT45_DATA->command));
  AT45_DATA->command_need_to_complete = 1;
  AT45_READ_MASK = AT45DB_D;
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_copy_page_to_buffer_and_back(int dev, uint8_t *buffer)
{
  if (AT45_DATA->command_start)
    {
      READ_PAGE_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }
  HW_DMSG_AT45("at45db: %s for page 0x%04x\n", str_cmd(AT45_DATA->command), AT45_DATA->command_page_addr);
  memcpy(buffer, AT45_DATA->mem.page[AT45_DATA->command_page_addr], AT45_PAGE_SIZE);
  at45db_set_busy_time(dev,at45db_busy_time(AT45_DATA->command));
  /* back is a null operation here */
  AT45_DATA->command_need_to_complete = 1;
  AT45_READ_MASK = AT45DB_D;
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void at45db_compare_page_to_buffer(int dev, uint8_t *buffer)
{
  int cmp = 0;

  if (AT45_DATA->command_start)
    {
      READ_PAGE_ADDRESS(dev);
      AT45_DATA->command_start = 0;
    }
  HW_DMSG_AT45("at45db: %s for page 0x%04x\n", str_cmd(AT45_DATA->command), AT45_DATA->command_page_addr);

  for(AT45_DATA->command_addr=0; AT45_DATA->command_addr < AT45_PAGE_SIZE; AT45_DATA->command_addr++)
    { /* we write 0 */
      if (AT45_DATA->mem.page[AT45_DATA->command_page_addr][AT45_DATA->command_addr] !=
	  buffer[AT45_DATA->command_addr])
	cmp ++;
    }

  AT45_DATA->compare_value  = cmp;
  AT45_DATA->compare_update = 1;
  at45db_set_busy_time(dev,at45db_busy_time(AT45_DATA->command));
  AT45_DATA->command_need_to_complete = 1;
  AT45_READ_MASK = AT45DB_D;
  AT45_READ_DATA = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int at45db_update_cycle(int dev)
{
  if (at45db_update_write_flag(dev))
    {
      /* busyflag returned to 0 : end of current command */
    }
  return 0;
}


int at45db_update(int dev)
{
  switch (AT45_DATA->command)
    {
    case AT45_OP_NOP: 
      if (AT45_DATA->write_byte_valid)
	{
	  if (AT45_DATA->command_needed_data == 0) /* new command */
	    {
	      AT45_DATA->command_next             = AT45_DATA->write_byte_value;
	      AT45_DATA->command_needed_data      = at45db_needed_data(AT45_DATA->command_next);
	      AT45_DATA->command_addr             = 0;
	      AT45_DATA->command_need_to_complete = at45db_need_to_complete(AT45_DATA->command_next);
              /* HW_DMSG_AT45("at45db:     command start %s\n",str_cmd(AT45_DATA->write_byte_value)); */
	    }
	  else
	    {
	      AT45_DATA->command_stored_data[ AT45_DATA->command_addr ] = AT45_DATA->write_byte_value;
	      /*HW_DMSG_AT45("at45db:     command arg %d = 0x%02x\n",AT45_DATA->command_addr,
		AT45_DATA->command_stored_data[ AT45_DATA->command_addr ]);*/
	      AT45_DATA->command_addr        += 1;
	      AT45_DATA->command_needed_data -= 1;
	    }

	  if (AT45_DATA->command_needed_data == 0)
	    {
	      AT45_DATA->command       = AT45_DATA->command_next;
	      AT45_DATA->command_start = 1;
	    }
	  AT45_READ_MASK = AT45DB_D; 
	  AT45_READ_DATA = 0;
	}
      break;
      
      /********/
      /* read */
      /********/
    case AT45_OP_CONTREAD_1      :
    case AT45_OP_CONTREAD_2      :
      if (AT45_DATA->command_start)
	{
	  READ_LINEAR_ADDRESS(dev);
	  AT45_DATA->command_start = 0;
	}
      HW_DMSG_AT45("at45db: %s at address 0x%04x\n",str_cmd(AT45_DATA->command),AT45_DATA->command_addr);
      AT45_READ_MASK = AT45DB_D; 
      AT45_READ_DATA = AT45_DATA->mem.raw[AT45_DATA->command_addr];
      AT45_DATA->command_addr = (AT45_DATA->command_addr + 1) % AT45_FLASH_SIZE; 
      break;

    case AT45_OP_PAGE_READ_1     : 
    case AT45_OP_PAGE_READ_2     : /* wraps at page boundary */
      if (AT45_DATA->command_start)
	{
	  READ_PAGE_ADDRESS(dev);
	  READ_BYTE_ADDRESS(dev);
	  AT45_DATA->command_start = 0;
	}
      AT45_READ_MASK = AT45DB_D; 
      AT45_READ_DATA = AT45_DATA->mem.page[AT45_DATA->command_page_addr][AT45_DATA->command_addr];
      HW_DMSG_AT45("at45db: %s at page 0x%04x addr 0x%04x = 0x%02x\n",str_cmd(AT45_DATA->command),
		   AT45_DATA->command_page_addr,AT45_DATA->command_addr,
		   AT45_DATA->mem.page[AT45_DATA->command_page_addr][AT45_DATA->command_addr]);
      AT45_DATA->command_addr = (AT45_DATA->command_addr + 1) % AT45_PAGE_SIZE; 
      break;

    case AT45_OP_BUFFER1_READ_1  : 
    case AT45_OP_BUFFER1_READ_2  : 
      at45db_read_from_buffer(dev,AT45_DATA->buffer1);
      break;

    case AT45_OP_BUFFER2_READ_1  :
    case AT45_OP_BUFFER2_READ_2  :
      at45db_read_from_buffer(dev,AT45_DATA->buffer2);
      break;

    case AT45_OP_STATUS_READ_1   :
    case AT45_OP_STATUS_READ_2   :
      HW_DMSG_AT45("at45db: %s = 0x%02x\n",str_cmd(AT45_DATA->command),AT45_SR.s & 0xff);
      AT45_READ_MASK = AT45DB_D; 
      AT45_READ_DATA = AT45_SR.s;
      break;

      /*********/
      /* erase */
      /*********/
    case AT45_OP_SI_TO_BUFFER1   :
      at45db_write_si_to_buffer(dev,AT45_DATA->buffer1);
      break;

    case AT45_OP_SI_TO_BUFFER2   : 
      at45db_write_si_to_buffer(dev,AT45_DATA->buffer2);
      break;

    case AT45_OP_BUFFER1_WI_ERASE: 
      at45db_write_buffer_with_erase(dev,AT45_DATA->buffer1);
      break;

    case AT45_OP_BUFFER2_WI_ERASE: 
      at45db_write_buffer_with_erase(dev,AT45_DATA->buffer2);
      break;

    case AT45_OP_BUFFER1_WO_ERASE:
      at45db_write_buffer_without_erase(dev,AT45_DATA->buffer1);
      break;

    case AT45_OP_BUFFER2_WO_ERASE:
      at45db_write_buffer_without_erase(dev,AT45_DATA->buffer2);
      break;

    case AT45_OP_PAGE_ERASE      : 
      if (AT45_DATA->command_start)
	{
	  READ_PAGE_ADDRESS(dev);
	  AT45_DATA->command_start = 0;
	}
      memset(AT45_DATA->mem.page[AT45_DATA->command_page_addr], 0, AT45_PAGE_SIZE);
      at45db_set_busy_time(dev,at45db_busy_time(AT45_DATA->command));
      AT45_DATA->command_need_to_complete = 1;
      break;

    case AT45_OP_BLOCK_ERASE     : 
      at45db_block_erase(dev);
      break;

      /******************************************/
      /* erase buffer + page program with erase */
      /******************************************/
    case AT45_OP_SI_BUFFER1_PPE  :
      if (AT45_DATA->command_start)
	{
	  READ_PAGE_ADDRESS(dev);
	}
      at45db_write_si_to_buffer(dev,AT45_DATA->buffer1);
      /* write on CS */
      break;

    case AT45_OP_SI_BUFFER2_PPE  : 
      if (AT45_DATA->command_start)
	{
	  READ_PAGE_ADDRESS(dev);
	}
      at45db_write_si_to_buffer(dev,AT45_DATA->buffer2);
      /* write on CS */
      break;

      /******************/
      /* copy / compare */
      /******************/
    case AT45_OP_COPY_BUFFER1    : 
      at45db_copy_page_to_buffer(dev,AT45_DATA->buffer1);
      break;

    case AT45_OP_COPY_BUFFER2    : 
      at45db_copy_page_to_buffer(dev,AT45_DATA->buffer2);
      break;

    case AT45_OP_COMP_BUFFER1    : 
      at45db_compare_page_to_buffer(dev,AT45_DATA->buffer1);
      break;

    case AT45_OP_COMP_BUFFER2    : 
      at45db_compare_page_to_buffer(dev,AT45_DATA->buffer2);
      break;

      /********************/
      /* multiple rewrite */
      /********************/
    case AT45_OP_MULTI_BUFFER1   : 
      at45db_copy_page_to_buffer_and_back(dev,AT45_DATA->buffer1);
      break;

    case AT45_OP_MULTI_BUFFER2   : 
      at45db_copy_page_to_buffer_and_back(dev,AT45_DATA->buffer2);
      break;

    default:
      HW_DMSG_AT45("at45db: unknown command 0x%02x\n", AT45_DATA->command & 0xff);
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
