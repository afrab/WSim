
/**
 *  \file   mcu.h
 *  \brief  WSim MCU hardware definitions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef HW_MCU_H
#define HW_MCU_H


/**********************************************************************************************
  MCU PC
  ======

  The PC can be read at any time by mcu_get_pc();

  This PC must be valid during current instruction simulation cycle.
  The next PC is set either at the end of an instruction or at the
  beginning by setting PC to the next PC.

 **********************************************************************************************/



/**********************************************************************************************
  MCU signals
  ===========

  mcu signal is a 32 bits variable that holds signal and 
  control bitfields

 
   .... .... | .... .... | .... .... | XXXX XXXX  : SIG_MCU_xxxx or HOST_SIGNAL
   .... .... | .... .... | .... ...X | .... ....  : SIG_MCU_LPM_CHANGE
  
   .... .... | .... ...X | .... .... | .... ....  : SIG_MCU          (set if SIG_MCU)
   .... .... | .... ..X. | .... .... | .... ....  : SIG_RUN_INSN     (insn single step mode)
   .... .... | .... .X.. | .... .... | .... ....  : SIG_RUN_TIME     (time single step mode)
   .... .... | .... X... | .... .... | .... ....  : SIG_GDB_SINGLE   (gdb  single step mode)
   .... .... | ...X .... | .... .... | .... ....  : SIG_GDB_IO       (IO TCP/GDB)
   .... .... | ..X. .... | .... .... | .... ....  : SIG_CON_IO       (IO Console)
   .... .... | .X.. .... | .... .... | .... ....  : SIG_WORLDSENS_IO (IO UDB/WSNnet)
   .... .... | X... .... | .... .... | .... ....  : SIG_UI           (IO UI)
  
   .... ...X | .... .... | .... .... | .... ....  : SIG_HOST         (Host signal on WSim)
   .... ..X. | .... .... | .... .... | .... ....  : SIG_MAC          (Memory access control)
   XXXX X... | .... .... | .... .... | .... ....  : SIG_MAC_xxxx


   SIG_MCU 8bits can be set either for an internal MCU signal (identified by the SIG_MCU bit)
   or for an external Unix signal on the WSim process (identified by SIG_HOST bit).

**********************************************************************************************/


/* mcu internal signal id */
#define SIG_MCU_HUP        0x00000001
#define SIG_MCU_INT        0x00000002
#define SIG_MCU_QUIT       0x00000004 /* used */
#define SIG_MCU_ILL        0x00000008 /* used */
#define SIG_MCU_TRAP       0x00000010 /* used */
#define SIG_MCU_ABRT       0x00000020
#define SIG_MCU_BUS        0x00000040 /* used */
#define SIG_MCU_TSTP       0x00000080
#define SIG_MCU_LPM_CHANGE 0x00000100
#define SIG_MCU_ALL        0x000001ff

#define SIG_HOST_SIGNAL    0x000000ff

/* signal source identifier */
#define SIG_MCU            0x00010000 /* mcu internal signal     */
#define SIG_RUN_INSN       0x00020000 /* insn mode               */
#define SIG_RUN_TIME       0x00040000 /* time mode               */
#define SIG_GDB_SINGLE     0x00080000 /* simul trap for GDB      */

#define SIG_GDB_IO         0x00100000 /* gdb tcp io request      */
#define SIG_CON_IO         0x00200000 /* console mode            */
#define SIG_WORLDSENS_IO   0x00400000 /* worldsens network io    */
#define SIG_UI             0x00800000 /* ui signal (keyboard)    */
#define SIG_HOST           0x01000000 /* host signal             */
#define SIG_MAC            0x02000000 /* mem breakpoint          */
#define SIG_BREAK_MEM_XX   0xf0000000 /*                         */


/* Memory Access Control */
/* ===================== */

#define MAC_BREAK_UNKNOWN        0x00 /* no breakpoint           */
#define MAC_BREAK_SOFT           0x01 /* software breakpoint     */
#define MAC_BREAK_HARD           0x02 /* hardware breakpoint     */
#define MAC_WATCH_READ           0x04 /* read watchpoint         */
#define MAC_WATCH_WRITE          0x08 /* write watchpoint        */
#define MAC_MUST_WRITE_FIRST     0x10 /* Read before write check */

#define MAC_BREAK_FETCH         ( MAC_BREAK_SOFT | MAC_BREAK_HARD                   )
#define MAC_WATCH_ACCESS        ( MAC_WATCH_READ | MAC_WATCH_WRITE                  )
#define MAC_BREAK_WATCH_FETCH   ( MAC_BREAK_SOFT | MAC_BREAK_HARD  | MAC_WATCH_READ )
#define MAC_ALL                 ( 0x1f )

#define MAC_TO_SIG(b)           ((b & MAC_ALL) << 27)
#define SIG_TO_MAC(b)           ((b >> 27) & MAC_ALL)

/* ************************************** */
/* ************************************** */
/* ************************************** */

int      mcu_arch_id            (void);
int      mcu_mach_id            (void);
char*    mcu_name               (void);
char*    mcu_modelname          (void);

int      mcu_options_add        (void);
void     mcu_print_description  (void);

void     mcu_reset              (void);
void     mcu_run                (void);

int      mcu_registers_number   (void);
uint16_t mcu_register_get       (int i);
void     mcu_register_set       (int i, uint16_t v);

uint16_t mcu_get_pc             (void);        /* current instruction   */
void     mcu_set_pc_next        (uint16_t x);  /* set next instruction  */
uint16_t mcu_get_pc_next        (void);        /* read next instruction */

void     mcu_signal_set         (uint32_t sig);
void     mcu_signal_add         (uint32_t sig);
void     mcu_signal_remove      (uint32_t sig);
void     mcu_signal_clr         (void);
uint32_t mcu_signal_get         (void);
char*    mcu_signal_str         (void);        /* debug.c */

void     mcu_state_save         (void);
void     mcu_state_restore      (void);

void     mcu_dump_stats         (int64_t user_nanotime);

uint64_t mcu_get_cycles         (void);
uint64_t mcu_get_insn           (void);

/* ************************************** */
/* ************************************** */
/* ************************************** */

uint8_t  mcu_jtag_read_byte     (uint16_t addr);
void     mcu_jtag_write_byte    (uint16_t addr, uint8_t val);
uint16_t mcu_jtag_read_word     (uint16_t addr);
int      mcu_jtag_read_section  (uint8_t *mem, uint16_t start, uint16_t size);
void     mcu_jtag_write_section (uint8_t *mem, uint16_t start, uint16_t size);
void     mcu_jtag_write_zero    (uint16_t start, uint16_t size);
int      mcu_hexfile_load       (char *filename);

/* ************************************** */
/* ************************************** */
/* ************************************** */

char*    mcu_regname_str        (unsigned r);
char*    mcu_ramctl_str         (int type);

/* ************************************** */
/* ************************************** */
/* ************************************** */

#if defined(ENABLE_RAM_CONTROL)
/* 
 * This RAM Control is not backtracked since we have to survice a 
 * backtrack when doing GDB hardware breakpoint while in WSNet mode.
 * This will have an influence on read before write error detection.
 *
 */
extern uint8_t MCU_RAMCTL [];
#endif

/* ************************************** */
/* ************************************** */
/* ************************************** */

#if defined(ENABLE_RAM_CONTROL)
int      mcu_ramctl_init        (void);
void     mcu_ramctl_tst_read    (uint16_t addr);
void     mcu_ramctl_tst_write   (uint16_t addr);
void     mcu_ramctl_tst_fetch   (uint16_t addr);
void     mcu_ramctl_set_bp      (uint16_t addr, int type);
void     mcu_ramctl_unset_bp    (uint16_t addr, int type);
void     mcu_ramctl_read        (uint16_t addr);
void     mcu_ramctl_read_block  (uint16_t addr, int size);
void     mcu_ramctl_write       (uint16_t addr);
void     mcu_ramctl_write_block (uint16_t addr, int size);
uint8_t  mcu_ramctl_read_ctl    (uint16_t addr);
#else
#define  mcu_ramctl_init(s)          do { } while (0)
#define  mcu_ramctl_tst_read(x)      do { } while (0)
#define  mcu_ramctl_tst_write(x)     do { } while (0)
#define  mcu_ramctl_tst_fetch(x)     do { } while (0)
#define  mcu_ramctl_set_bp(x,t)      do { } while (0)
#define  mcu_ramctl_unset_bp(x,t)    do { } while (0)
#define  mcu_ramctl_read(x)          do { } while (0)
#define  mcu_ramctl_read_block(x,s)  do { } while (0)
#define  mcu_ramctl_write(x)         do { } while (0)
#define  mcu_ramctl_write_block(x,s) do { } while (0)
#define  mcu_ramctl_read_ctl(x)      do { } while (0)
#endif

/* ************************************** */
/* ************************************** */
/* ************************************** */

#endif /* MCU_H */
