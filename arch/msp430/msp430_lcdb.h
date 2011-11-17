/**
 *  \file   msp430_lcdb.h
 *  \brief  MSP430 LCD_B definition (based upon msp430_lcd.h)
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef MSP430_LCDB_H
#define MSP430_LCDB_H

#if defined(__msp430_have_lcdb)
#define LCDB_IOMEM_BEGIN        LCDB_BASE
#define LCDB_IOMEM_END          (LCDB_BASE + 0x5f)

#define LCDB_LCDBCTL0           (LCDB_BASE + 0x00)
#define LCDB_LCDBCTL1           (LCDB_BASE + 0x02)
#define LCDB_LCDBBLKCTL         (LCDB_BASE + 0x04)
#define LCDB_LCDM_START         (LCDB_BASE + 0x20)
#define LCDB_LCDM_STOP          (LCDB_BASE + 0x2D)
#define LCDB_LCDM_SIZE          (LCDB_LCDM_STOP - LCDB_LCDM_START + 1)
#define LCDB_LCDBM_START        (LCDB_BASE + 0x40)
#define LCDB_LCDBM_STOP         (LCDB_BASE + 0x4D)
#define LCDB_LCDBM_SIZE         (LCDB_LCDBM_STOP - LCDB_LCDBM_START + 1)


#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) lcdbctl0_t
{
  uint16_t
  lcddivx : 5,
    lcdprex : 3,
    lcdssel : 1,
    reserved2 : 1,
    lcdmxx : 2,
    lcdson : 1,
    reserved : 1,
    lcdon : 1;
};
#else

struct __attribute__((packed)) lcdbctl0_t
{
  uint16_t
  lcdon : 1,
    reserved : 1,
    lcdson : 1,
    lcdmxx : 2,
    reserved2 : 1,
    lcdssel : 1,
    lcdprex : 3,
    lcddivx : 5;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) lcdbctl1_t
{
  uint16_t
  lcdnocapie : 1,
    lcdblkonie : 1,
    lcdblkoffie : 1,
    lcdfrmie : 1,
    reserved : 4,
    lcdnocapifg : 1,
    lcdblkonifg : 1,
    lcdblkoffifg : 1,
    lcdfrmifg : 1;
};
#else

struct __attribute__((packed)) lcdbctl1_t
{
  uint16_t
  lcdfrmifg : 1,
    lcdblkoffifg : 1,
    lcdblkonifg : 1,
    lcdnocapifg : 1,
    reserved : 4,
    lcdfrmie : 1,
    lcdblkoffie : 1,
    lcdblkonie : 1,
    lcdnocapie : 1;
};
#endif

#if defined(WORDS_BIGENDIAN)

struct __attribute__((packed)) lcdbblkctl_t
{
  uint16_t
  reserved : 8,
    lcdblkdivx : 3,
    lcdblkprex : 3,
    lcdblkmodx : 2;
};
#else

struct __attribute__((packed)) lcdbblkctl_t
{
  uint16_t
  lcdblkmodx : 2,
    lcdblkprex : 3,
    lcdblkdivx : 3,
    reserved : 8;
};
#endif

/**
 * LCD Data Structure
 **/
struct msp430_lcdb_t {

  union {
    struct lcdbctl0_t b;
    uint16_t s;
  } lcdbctl0;

  union {
    struct lcdbctl1_t b;
    uint16_t s;
  } lcdbctl1;

  union {
    struct lcdbblkctl_t b;
    uint16_t s;
  } lcdbblkctl;
  uint8_t mem[LCDB_LCDM_SIZE];
  uint8_t bmem[LCDB_LCDBM_SIZE];
};

void msp430_lcdb_create();
int msp430_lcdb_reset();
void msp430_lcdb_update();
int8_t msp430_lcdb_read8(uint16_t addr);
int16_t msp430_lcdb_read(uint16_t addr);
void msp430_lcdb_write8(uint16_t addr, int8_t val);
void msp430_lcdb_write(uint16_t addr, int16_t val);

#else
#define msp430_lcdb_create() do { } while (0)
#define msp430_lcdb_reset() do { } while (0)
#define msp430_lcdb_update() do { } while (0)
#endif
#endif
