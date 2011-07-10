
/**
 *  \file   msp430_io.c
 *  \brief  MSP430 Memory mapped IO selector
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "msp430.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define ADDR64K 0x10000

static addr_map_read8_t   pread8  [ADDR64K];
static addr_map_write8_t  pwrite8 [ADDR64K];   
 
static addr_map_read16_t  pread16 [ADDR64K];
static addr_map_write16_t pwrite16[ADDR64K];

static int8_t  msp430_read8_sigbus   (uint16_t addr);
static int16_t msp430_read16_sigbus  (uint16_t addr);
static void    msp430_write8_sigbus  (uint16_t addr, int8_t val);
static void    msp430_write16_sigbus (uint16_t addr, int16_t val);

/* ************************************************** */
/* ** SIGBUS **************************************** */
/* ************************************************** */

#define STACK_DUMP_LINES 32

static void SIGBUS_EXIT(uint32_t sig)
{
  ERROR("msp430: SIGBUS\n");
  msp430_print_registers(4);
  msp430_print_stack(STACK_DUMP_LINES);
  mcu_signal_add(SIG_MCU | SIG_MCU_BUS | sig);
}


static void msp430_write8_sigbus(uint16_t addr, int8_t val)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: write byte [0x%04x] = 0x%02x at pc 0x%04x\n",addr,(uint8_t)val,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  SIGBUS_EXIT(0);
  ERROR("msp430: =======================\n");
}

static void msp430_write16_sigbus(uint16_t addr, int16_t val)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: write short [0x%04x] = 0x%04x at pc 0x%04x\n",addr,(uint16_t)val,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  SIGBUS_EXIT(0);
  ERROR("msp430: =======================\n");
}

static int8_t msp430_read8_sigbus(uint16_t addr)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: read byte at address 0x%04x at pc 0x%04x\n",addr,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  if (mcu_get_pc() == addr)
    {
      SIGBUS_EXIT(SIG_MCU_ILL);
    }
  ERROR("msp430: =======================\n");
  return 0;
}

static int16_t msp430_read16_sigbus(uint16_t addr)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: read short at address 0x%04x at pc 0x%04x\n",addr,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  if (mcu_get_pc() == addr)
    {
      SIGBUS_EXIT(SIG_MCU_ILL);
    }
  ERROR("msp430: =======================\n");
  return 0;
}

/* ************************************************** */
/* ** FLASH ***************************************** */
/* ************************************************** */

static void msp430_write8_flash(uint16_t addr, int8_t UNUSED val)
{
  ERROR("msp430:io: writing byte to Flash [0x%04x] PC = 0x%04x\n",addr,mcu_get_pc());
}

static void msp430_write16_flash(uint16_t addr, int16_t UNUSED val)
{
  ERROR("msp430:io: writing short to Flash [0x%04x] PC = 0x%04x\n",addr,mcu_get_pc());
}

static void msp430_write8_start_flash_erase(uint16_t addr, int8_t val)
{
  // HW_DMSG("msp430:io: writing byte to Flash [0x%04x] PC = 0x%04x\n",addr,mcu_get_pc());
  msp430_flash_start_erase(addr, 1, val);
}

static void msp430_write16_start_flash_erase(uint16_t addr, int16_t val)
{
  // HW_DMSG("msp430:io: writing short to Flash [0x%04x] PC = 0x%04x\n",addr,mcu_get_pc());
  msp430_flash_start_erase(addr, 2, val);
}

static int16_t msp430_read16_flash_jump_pc(uint16_t UNUSED addr)
{
  return 0x03FFF;  /* JUMP PC */
}

#define msp430_read8_flash   msp430_read8_ram
#define msp430_read16_flash  msp430_read16_ram

/* ************************************************** */
/* ** RAM ******************************************* */
/* ************************************************** */

static inline void CHECK_ALIGNMENT(uint16_t loc,char *str)
{
  if ((loc & 1) == 1)
    {
      ERROR("msp430:io: unaligned %s at 0x%04x [PC=0x%04x]\n",str,loc,mcu_get_pc());
    }
}

/* these functions are called using funtion pointer arrays */
/* the inline keyword is almost useless, need to check asm */
/* when compiled with -O3                                  */
#define MEM_INLINE inline

static MEM_INLINE void msp430_write8_ram(uint16_t addr, int8_t val)
{
  addr &= 0xffffu;  /* broken 16<>32 code with gcc, need to test later */
  mcu_ramctl_write(addr);
  MCU_RAM[addr] = val;
}

static MEM_INLINE void msp430_write16_ram(uint16_t addr, int16_t val)
{
  CHECK_ALIGNMENT(addr,"read ");

  addr &= 0xffffu;  /* broken 16<>32 code with gcc, need to test later */
  mcu_ramctl_write(addr);
  MCU_RAM[addr++] =  val       & 0xff;
  mcu_ramctl_write(addr);
  MCU_RAM[addr  ] = (val >> 8) & 0xff;
}

static MEM_INLINE int8_t msp430_read8_ram(uint16_t addr)
{
  addr &= 0xffffu; /* broken 16<>32 code with gcc, need to test later */
  mcu_ramctl_read(addr);
  return MCU_RAM[addr];
}

static MEM_INLINE int16_t msp430_read16_ram(uint16_t addr)
{
  CHECK_ALIGNMENT(addr,"write");

  addr &= 0xffffu;  /* broken 16<>32 code with gcc, need to test later */
  mcu_ramctl_read(addr);
  mcu_ramctl_read(addr+1);
  return MCU_RAM[addr+1] << 8 | MCU_RAM[addr];
}

/* ************************************************** */
/* ** RAM MIRRORED ** defined in msp430_models.h **** */
/* ************************************************** */

#if defined(ADDR_MIRROR_START)
#define MIRROR_OFFSET (ADDR_RAM_START - ADDR_MIRROR_START)

static MEM_INLINE void msp430_write8_ram_mirrored (uint16_t addr, int8_t val)
{
  msp430_write8_ram (addr + MIRROR_OFFSET,val);
}

static MEM_INLINE void msp430_write16_ram_mirrored(uint16_t addr, int16_t val)
{
  msp430_write16_ram(addr + MIRROR_OFFSET, val);
}

static MEM_INLINE int8_t msp430_read8_ram_mirrored (uint16_t addr)
{
  return msp430_read8_ram (addr + MIRROR_OFFSET);
}

static MEM_INLINE int16_t msp430_read16_ram_mirrored(uint16_t addr)
{
  return msp430_read16_ram(addr + MIRROR_OFFSET);
}
#endif // ADDR_MIRROR_START

/* ************************************************** */
/* ** INITIAL SETUP * I/O Function pointers ********* */
/* ************************************************** */

static void msp430_set_readptr8(addr_map_read8_t f, uint16_t addr)
{
  if ((pread8[addr] == 0) || (pread8[addr] == msp430_read8_sigbus))
    {
      pread8[addr] = f;
    }
  else
    {
      ERROR("msp430:io: MCU create error, IO read8 0x%04x function not unique\n",addr);
      machine_exit(0);
    }
}

static void msp430_set_readptr16(addr_map_read16_t f, uint16_t addr)
{
  if ((pread16[addr] ==  0) || 
      (pread16[addr] == msp430_read16_sigbus) ||
      (pread16[addr] == msp430_read16_flash_jump_pc) ||
      (f             == msp430_read16_flash_jump_pc) ||
      (f             == msp430_read16_ram))
    {
      pread16[addr] = f;
    }
  else
    {
      ERROR("msp430:io: MCU create error, IO read16 0x%04x function not unique\n",addr);
      ERROR("msp430:io: function to set      %p\n",f);
      ERROR("msp430:io: function registered  %p\n",pread16[addr]);
      ERROR("msp430:io:       %p : sigbus\n",     msp430_read16_sigbus);
      ERROR("msp430:io:       %p : flash\n",      msp430_read16_flash);
      ERROR("msp430:io:       %p : jump pc\n",    msp430_read16_flash_jump_pc);
      ERROR("msp430:io:       %p : ram\n",        msp430_read16_ram);
#if defined(ADDR_MIRROR_START)
      ERROR("msp430:io:       %p : ram mirror\n", msp430_read16_ram_mirrored);
#endif
      machine_exit(0);
    }
}

static void msp430_set_writeptr8(addr_map_write8_t f, uint16_t addr)
{
  if ((pwrite8[addr] == 0) || 
      (pwrite8[addr] == msp430_write8_sigbus) || 
      (pwrite8[addr] == msp430_write8_start_flash_erase) ||
      (f             == msp430_write8_start_flash_erase))
    {
      pwrite8[addr] = f;
    }
  else
    {
      ERROR("msp430:io: MCU create error, IO write8 0x%04x function not unique\n",addr);
      machine_exit(0);
    }
}

static void msp430_set_writeptr16(addr_map_write16_t f, uint16_t addr)
{
  if ((pwrite16[addr] == 0) || 
      (pwrite16[addr] == msp430_write16_sigbus) ||
      (pwrite16[addr] == msp430_write16_start_flash_erase) ||
      (f              == msp430_write16_start_flash_erase))
    {
      pwrite16[addr] = f;
    }
  else
    {
      ERROR("msp430:io: MCU create error, IO write16 0x%04x function not unique\n",addr);
      machine_exit(0);
    }
}

/* ****************************************************** */
/* ** I/O Function to start flash erase on dummy write ** */
/* ****************************************************** */

void msp430_io_set_flash_write_start_erase(uint16_t start, uint16_t stop)
{
  uint32_t addr;
  for(addr = start; addr <= stop; addr++)
    {
      msp430_set_writeptr8  (msp430_write8_start_flash_erase,  addr);
      msp430_set_writeptr16 (msp430_write16_start_flash_erase, addr);
    }
}

void msp430_io_set_flash_write_normal(uint16_t start, uint16_t stop)
{
  uint32_t addr;
  for(addr = start; addr <= stop; addr++)
    {
      msp430_set_writeptr8  (msp430_write8_flash,  addr);
      msp430_set_writeptr16 (msp430_write16_flash, addr);
    }
}

void msp430_io_set_flash_read_jump_pc(uint16_t start, uint16_t stop)
{
  uint32_t addr;
  for(addr = start; addr <= stop; addr++)
    {
      msp430_set_readptr16 (msp430_read16_flash_jump_pc, addr);
    }
}

void msp430_io_set_flash_read_normal (uint16_t start, uint16_t stop)
{
  uint32_t addr;
  for(addr = start; addr <= stop; addr++)
    {
      msp430_set_readptr16 (msp430_read16_flash, addr);
    }
}

/* ************************************************** */
/* ** I/O Function pointers assignment ************** */
/* ************************************************** */

void msp430_io_register_addr8(uint16_t addr, 
                              addr_map_read8_t read8, addr_map_write8_t write8)
{
  msp430_set_readptr8  (read8,  addr);
  msp430_set_writeptr8 (write8, addr);
}

void msp430_io_register_range8(uint16_t start, uint16_t stop, 
                               addr_map_read8_t read8, addr_map_write8_t write8)
{
  uint32_t addr;
  for(addr = start; addr <= stop; addr++)
    {
      msp430_set_readptr8  (read8,  addr);
      msp430_set_writeptr8 (write8, addr);
    }
}


void msp430_io_register_addr16(uint16_t addr, 
                               addr_map_read16_t read16, addr_map_write16_t write16)
{
  msp430_set_readptr16  (read16,  addr);
  msp430_set_writeptr16 (write16, addr);
}

void msp430_io_register_range16(uint16_t start, uint16_t stop, 
                                addr_map_read16_t read16, addr_map_write16_t write16)
{
  uint32_t addr;
  for(addr = start; addr <= stop; addr++)
    {
      msp430_set_readptr16  (read16,  addr);
      msp430_set_writeptr16 (write16, addr);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_io_create(void)
{
  uint32_t i;

  memset(pread8  ,0,sizeof(pread8));
  memset(pwrite8 ,0,sizeof(pwrite8));
  memset(pread16 ,0,sizeof(pread16));
  memset(pwrite16,0,sizeof(pwrite16));

  for(i=0; i < ADDR64K; i++) 
    {
      msp430_io_register_addr8 (i, msp430_read8_sigbus,  msp430_write8_sigbus);
      msp430_io_register_addr16(i, msp430_read16_sigbus, msp430_write16_sigbus);
    }

  // memory 
  msp430_io_register_range8(ADDR_FLASH_START,  ADDR_FLASH_STOP,  msp430_read8_flash,  msp430_write8_flash);
  msp430_io_register_range8(ADDR_RAM_START,    ADDR_RAM_STOP,    msp430_read8_ram,    msp430_write8_ram);
  msp430_io_register_range8(ADDR_NVM_START,    ADDR_NVM_STOP,    msp430_read8_flash,  msp430_write8_flash);

  msp430_io_register_range16(ADDR_FLASH_START, ADDR_FLASH_STOP, msp430_read16_flash,  msp430_write16_flash);
  msp430_io_register_range16(ADDR_RAM_START,   ADDR_RAM_STOP,   msp430_read16_ram,    msp430_write16_ram);
  msp430_io_register_range16(ADDR_NVM_START,   ADDR_NVM_STOP,   msp430_read16_flash,  msp430_write16_flash);

#if defined(ADDR_BOOT_START)
  msp430_io_register_range16(ADDR_BOOT_START,ADDR_BOOT_STOP,msp430_read16_flash,msp430_write16_flash);
  msp430_io_register_range8 (ADDR_BOOT_START,ADDR_BOOT_STOP,msp430_read8_flash, msp430_write8_flash);
#endif

#if defined(ADDR_MIRROR_START)
  msp430_io_register_range16(ADDR_MIRROR_START,ADDR_MIRROR_STOP, 
                             msp430_read16_ram_mirrored, msp430_write16_ram_mirrored);
  msp430_io_register_range8 (ADDR_MIRROR_START,ADDR_MIRROR_STOP, 
                             msp430_read8_ram_mirrored, msp430_write8_ram_mirrored);
#endif

}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_read_byte(uint16_t loc)
{
  int8_t res = 0;
  etracer_slot_access(loc, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_BUS, 0);
  res = pread8[loc](loc);
  mcu_ramctl_tst_fetch(loc);
  HW_DMSG_IO("msp430:io: read_byte [0x%04x] = 0x%02x\n",loc,res & 0xffu);
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_read_short(uint16_t loc)
{
  int16_t res = 0;
  etracer_slot_access(loc, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
  res = pread16[loc](loc);
  mcu_ramctl_tst_read(loc);
  mcu_ramctl_tst_read(loc + 1);
  HW_DMSG_IO("msp430:io: read_short [0x%04x] = 0x%04x\n",loc,res & 0xffffu);
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint16_t msp430_fetch_short(uint16_t loc)
{
  int16_t res = 0;
  etracer_slot_access(loc, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
  res = pread16[loc](loc);
  mcu_ramctl_tst_fetch(loc);
  /* mcu_ramctl_tst_fetch(loc + 1); *//* ? */
  HW_DMSG_IO("msp430:io: read_short [0x%04x] = 0x%04x\n",loc,res & 0xffffu);
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_write_byte(uint16_t loc, int8_t val)
{
  HW_DMSG_IO("msp430:io: write_byte [0x%04x] = 0x%02x\n",loc,val);
  etracer_slot_access(loc, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_BUS, 0);
  pwrite8[loc](loc,val);
  mcu_ramctl_tst_write(loc);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_write_short(uint16_t loc, int16_t val)
{
  HW_DMSG_IO("msp430:op: write_short [0x%04x] = 0x%04x\n",loc,val);
  etracer_slot_access(loc, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
  pwrite16[loc](loc,val);
  mcu_ramctl_tst_write(loc);
  mcu_ramctl_tst_write(loc + 1);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
