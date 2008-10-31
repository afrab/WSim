
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

typedef int8_t  (*addr_map_read8_t  ) (uint16_t addr);
typedef void    (*addr_map_write8_t ) (uint16_t addr, int8_t val);

typedef int16_t (*addr_map_read16_t ) (uint16_t addr);
typedef void    (*addr_map_write16_t) (uint16_t addr, int16_t val);

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

static void SIGBUS_EXIT(void)
{
  ERROR("msp430: SIGBUS\n");
  msp430_print_registers(4);
  msp430_print_stack(STACK_DUMP_LINES);
  mcu_signal_add(SIG_MCU | SIG_MCU_BUS);
}


static void msp430_write8_sigbus(uint16_t addr, int8_t val)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: write byte [0x%04x] = 0x%02x at pc 0x%04x\n",addr,(uint8_t)val,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  if (((addr & 1) == 0) && (pread16[addr] != msp430_read16_sigbus))
    {
      ERROR("msp430:io:     -- registered 16 bit io handler\n");
      ERROR("msp430:io:     -- if this works on real HW, please report\n");
      ERROR("msp430:io:     -- for WSim correction\n");
    }
  SIGBUS_EXIT();
  ERROR("msp430: =======================\n");
}

static void msp430_write16_sigbus(uint16_t addr, int16_t val)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: write short [0x%04x] = 0x%04x at pc 0x%04x\n",addr,(uint16_t)val,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  SIGBUS_EXIT();
  ERROR("msp430: =======================\n");
}

static int8_t msp430_read8_sigbus(uint16_t addr)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: read byte at address 0x%04x at pc 0x%04x\n",addr,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  SIGBUS_EXIT();
  ERROR("msp430: =======================\n");
  return 0;
}

static int16_t msp430_read16_sigbus(uint16_t addr)
{
  ERROR("msp430: =======================\n");
  ERROR("msp430:io: read short at address 0x%04x at pc 0x%04x\n",addr,mcu_get_pc());
  ERROR("msp430:io:     -- target unknown or block not implemented\n");
  SIGBUS_EXIT();
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

#define msp430_read8_flash   msp430_read8_ram
#define msp430_read16_flash  msp430_read16_ram

/* ************************************************** */
/* ** RAM ******************************************* */
/* ************************************************** */

/* these functions are called using funtion pointer arrays */
/* the inline keyword is almost usless, need to check asm  */
/* when compiler with -O3                                  */
#define MEM_INLINE inline

static MEM_INLINE void msp430_write8_ram(uint16_t addr, int8_t val)
{
  addr &= 0xffffu;  /* broken 16<>32 code with gcc, need to test later */
  mcu_ramctl_write(addr);
  MCU_RAM[addr] = val;
}

static MEM_INLINE void msp430_write16_ram(uint16_t addr, int16_t val)
{
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
      ERROR("msp430:io: MCU create errror, IO read8 0x%04x function not unique\n",addr);
      machine_exit(0);
    }
}

static void msp430_set_readptr16(addr_map_read16_t f, uint16_t addr)
{
  if ((pread16[addr] ==  0) || (pread16[addr] == msp430_read16_sigbus))
    {
      pread16[addr] = f;
    }
  else
    {
      ERROR("msp430:io: MCU create errror, IO read16 0x%04x function not unique\n",addr);
      machine_exit(0);
    }
}

static void msp430_set_writeptr8(addr_map_write8_t f, uint16_t addr)
{
  if ((pwrite8[addr] == 0) || (pwrite8[addr] == msp430_write8_sigbus))
    {
      pwrite8[addr] = f;
    }
  else
    {
      ERROR("msp430:io: MCU create errror, IO write8 0x%04x function not unique\n",addr);
      machine_exit(0);
    }

}

static void msp430_set_writeptr16(addr_map_write16_t f, uint16_t addr)
{
  if ((pwrite16[addr] == 0) || (pwrite16[addr] == msp430_write16_sigbus))
    {
      pwrite16[addr] = f;
    }
  else
    {
      ERROR("msp430:io: MCU create errror, IO write16 0x%04x function not unique\n",addr);
      machine_exit(0);
    }
}

/* ************************************************** */
/* ** I/O Function pointers assignment ************** */
/* ************************************************** */

static void P8_ADDR(uint16_t i, uint16_t ADDR,
		    addr_map_read8_t read8, addr_map_write8_t write8)
{
  if (i == ADDR)
    {
     msp430_set_readptr8(read8,i);
     msp430_set_writeptr8(write8,i);
  }
}

static void P8(uint16_t i, uint16_t start, uint16_t stop, 
	       addr_map_read8_t read8, addr_map_write8_t write8)
{
  if ((i>=start) && (i<=stop))
    {
      msp430_set_readptr8(read8,i);
      msp430_set_writeptr8(write8,i);
    }
}


static void P16_ADDR(uint16_t i, uint16_t ADDR,
		     addr_map_read16_t read16, addr_map_write16_t write16)
{
  if (i == ADDR)
    {
      msp430_set_readptr16(read16,i);
      msp430_set_writeptr16(write16,i);
    }
}

static void P16(uint16_t i, uint16_t start, uint16_t stop, 
		addr_map_read16_t read16, addr_map_write16_t write16)
{
  if ((i>=start) && (i<=stop))
    {
      msp430_set_readptr16(read16,i);
      msp430_set_writeptr16(write16,i);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_io_init(void)
{
  int32_t i;
  memset(pread8  ,0,sizeof(pread8));
  memset(pwrite8 ,0,sizeof(pwrite8));
  memset(pread16 ,0,sizeof(pread16));
  memset(pwrite16,0,sizeof(pwrite16));

  for(i=0; i < ADDR64K; i++) 
    {
      P8_ADDR  (i,i,msp430_read8_sigbus,  msp430_write8_sigbus);
      P16_ADDR (i,i,msp430_read16_sigbus, msp430_write16_sigbus);
    }

  for(i=0; i < ADDR64K; i++)
    {
      // ///////
      // 8 bits
      // ///////
      P8(i,SFR_START,SFR_END,msp430_sfr_read,msp430_sfr_write);

#if defined(__msp430_have_svs_at_55)
      P8_ADDR(i,SVSCTL,msp430_svs_read,msp430_svs_write);
#endif
#if defined(__msp430_have_basic_clock)
      P8(i,BC_START,BC_END,msp430_basic_clock_read,msp430_basic_clock_write);
#endif
#if defined(__msp430_have_fll_and_xt2)
      P8(i,FLL_START,FLL_END,msp430_fll_clock_read,msp430_fll_clock_write);
#endif
#if defined(__msp430_have_basic_timer)
      P8(i,BASIC_TIMER_START,BASIC_TIMER_END,msp430_basic_timer_read,msp430_basic_timer_write);
#endif

      P8(i,DIGIIO_START,DIGIIO_END,msp430_digiIO_mcu_read,msp430_digiIO_mcu_write);
      P8(i,USART0_START,USART0_END,msp430_usart0_read,msp430_usart0_write);

#if defined(__msp430_have_usart1)
      P8(i,USART1_START,USART1_END,msp430_usart1_read,msp430_usart1_write);
#endif
#if defined(__msp430_have_lcd)
      P8(i,LCD_IOMEM_BEGIN,LCD_IOMEM_END,msp430_lcd_read,msp430_lcd_write);
#endif
#if defined(__msp430_have_cmpa)
      P8(i,CMPA_START,CMPA_END,msp430_cmpa_read,msp430_cmpa_write);
#endif
	
      P8(i,ADDR_FLASH_START,  ADDR_FLASH_STOP,  msp430_read8_flash,        msp430_write8_flash);
      P8(i,ADDR_RAM_START,    ADDR_RAM_STOP,    msp430_read8_ram,          msp430_write8_ram);
      P8(i,ADDR_NVM_START,    ADDR_NVM_STOP,    msp430_read8_flash,        msp430_write8_flash);
#if defined(ADDR_BOOT_START)
      P8(i,ADDR_BOOT_START,   ADDR_BOOT_STOP,   msp430_read8_flash,        msp430_write8_flash);
#endif
#if defined(ADDR_MIRROR_START)
      P8(i,ADDR_MIRROR_START, ADDR_MIRROR_STOP, msp430_read8_ram_mirrored, msp430_write8_ram_mirrored);
#endif

      P16(i,ADDR_FLASH_START,  ADDR_FLASH_STOP,  msp430_read16_flash,        msp430_write16_flash);
      P16(i,ADDR_RAM_START,    ADDR_RAM_STOP,    msp430_read16_ram,          msp430_write16_ram);
      P16(i,ADDR_NVM_START,    ADDR_NVM_STOP,    msp430_read16_flash,        msp430_write16_flash);
#if defined(ADDR_BOOT_START)
      P16(i,ADDR_BOOT_START,   ADDR_BOOT_STOP,   msp430_read16_flash,        msp430_write16_flash);
#endif
#if defined(ADDR_MIRROR_START)
      P16(i,ADDR_MIRROR_START, ADDR_MIRROR_STOP, msp430_read16_ram_mirrored, msp430_write16_ram_mirrored);
#endif


      // ////////
      // 16 bits
      // ////////
      if ((i & 1) == 0)
	{
#if defined(__msp430_have_watchdog)
	  P16(i,WATCHDOG_START,WATCHDOG_END,msp430_watchdog_read,msp430_watchdog_write);
#endif
#if defined(__msp430_have_timera3)
	  P16_ADDR(i,TAIV,msp430_timerA3_read,msp430_timerA3_write);
	  P16(i,TIMER_A3_START,TIMER_A3_END,msp430_timerA3_read,msp430_timerA3_write);

	  P8_ADDR(i,TAIV,msp430_timerA3_read8,msp430_timerA3_write8);
	  P8     (i,TIMER_A3_START,TIMER_A3_END+1,msp430_timerA3_read8,msp430_timerA3_write8);
#endif
#if defined(__msp430_have_timera5) 
	  P16_ADDR(i,TA1IV,msp430_timerA5_read,msp430_timerA5_write);
	  P16(i,TIMER_A5_START,TIMER_A5_END,msp430_timerA5_read,msp430_timerA5_write);
#endif
#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)
	  P16_ADDR(i,TBIV,msp430_timerB_read,msp430_timerB_write);
	  P16(i,TIMER_B_START,TIMER_B_END,msp430_timerB_read,msp430_timerB_write);
#endif
#if defined(__msp430_have_hwmul)
	  P16(i,HWMUL_START,HWMUL_END,msp430_hwmul_read,msp430_hwmul_write);
#endif
#if defined(__msp430_have_dma)
	  P16_ADDR(i,DMACTL0,msp430_dma_read,msp430_dma_write);
	  P16_ADDR(i,DMACTL1,msp430_dma_read,msp430_dma_write);
	  P16(i,DMA_START,DMA_END,msp430_dma_read,msp430_dma_write);
#endif
#if defined(__msp430_have_flash)
	  P16(i,FLASH_START,FLASH_END,msp430_flash_read,msp430_flash_write);
#endif
#if defined(__msp430_have_adc12)
	  P16(i,ADC12CTL0 ,ADC12IV    ,msp430_adc12_read16,msp430_adc12_write16);
	  P16(i,ADC12MEM0 ,ADC12MEM15 ,msp430_adc12_read16,msp430_adc12_write16);
	  P8 (i,ADC12MCTL0,ADC12MCTL15,msp430_adc12_read8 ,msp430_adc12_write8);
#endif
#if defined(__msp430_have_adc10)
	  P16_ADDR(i,ADC10CTL0,msp430_adc10_read16,msp430_adc10_write16);
	  P16_ADDR(i,ADC10CTL1,msp430_adc10_read16,msp430_adc10_write16);
	  P16_ADDR(i,ADC10MEM ,msp430_adc10_read16,msp430_adc10_write16);
	  P16_ADDR(i,ADC10SA  ,msp430_adc10_read16,msp430_adc10_write16);
	  P8_ADDR (i,ADC10AE  ,msp430_adc10_read8 ,msp430_adc10_write8);
	  P8_ADDR (i,ADC10DTC0,msp430_adc10_read8 ,msp430_adc10_write8);
	  P8_ADDR (i,ADC10DTC1,msp430_adc10_read8 ,msp430_adc10_write8);
#endif
#if defined(__msp430_have_dac12)
	  P16_ADDR(i,DAC12_0CTL,msp430_dac12_read,msp430_dac12_write);
	  P16_ADDR(i,DAC12_1CTL,msp430_dac12_read,msp430_dac12_write);
	  P16_ADDR(i,DAC12_0DAT,msp430_dac12_read,msp430_dac12_write);
	  P16_ADDR(i,DAC12_1DAT,msp430_dac12_read,msp430_dac12_write);
#endif
	}
    }

  /* Notes:
   * should fill pread/pwrite with memory access functions and test performance
   */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline void CHECK_ALIGNMENT(uint16_t loc,char *str)
{
  if ((loc & 1) == 1)
    {
      ERROR("msp430:io: unaligned %s at 0x%04x [PC=0x%04x]\n",str,loc,mcu_get_pc());
    }
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
  CHECK_ALIGNMENT(loc,"read");

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
  CHECK_ALIGNMENT(loc,"write");

  HW_DMSG_IO("msp430:op: write_short [0x%04x] = 0x%04x\n",loc,val);
  etracer_slot_access(loc, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_HWORD, ETRACER_ACCESS_LVL_BUS, 0);
  pwrite16[loc](loc,val);
  mcu_ramctl_tst_write(loc);
  mcu_ramctl_tst_write(loc + 1);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
