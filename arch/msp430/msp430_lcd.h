
/**
 *  \file   msp430_lcd.h
 *  \brief  MSP430 LCD definition for 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_LCD_H
#define MSP430_LCD_H

#if defined(__msp430_have_lcd)

#define LCD_IOMEM_BEGIN     0x090
#define LCD_IOMEM_END       0x0a4

#define LCDCTL        0x090
#define LCD_MEM_START 0x091 
#define LCD_MEM_STOP  0x0a4
#define LCD_MEM_SIZE  (LCD_MEM_STOP - LCD_MEM_START + 1)

/**
 * [doc copy/paste slau056e.pdf page 350]
 *
 * LCDPx  Bits  LCD Port Select. These bits select the pin function to be port I/O or LCD
 *    7-5   function for groups of segments pins. These bits ONLY affect pins with
 *           multiplexed functions. Dedicated LCD pins are always LCD function.
 *           000 No multiplexed pins are LCD function
 *           001 S0-S15 are LCD function
 *           010 S0-S19 are LCD function
 *           011 S0-S23 are LCD function
 *           100 S0-S27 are LCD function
 *           101 S0-S31 are LCD function
 *           110 S0-S35 are LCD function
 *           111 S0-S39 are LCD function
 * LCDMXx Bits  LCD mux rate. These bits select the LCD mode.
 *    4-3   00 Static
 *           01 2-mux
 *           10 3-mux
 *           11 4-mux
 * LCDSON Bit 2 LCD segments on. This bit supports flashing LCD applications by turning
 *          off all segment lines, while leaving the LCD timing generator and R33
 *           enabled.
 *           0       All LCD segments are off
 *           1       All LCD segments are enabled and on or off according to their
 *                   corresponding memory location.
 * Unused Bit 1 Unused
 * LCDON  Bit 0 LCD On. This bit turns on the LCD timing generator and R33.
 *           0       LCD timing generator and Ron are off
 *           1       LCD timing generator and Ron are on
 **/

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) lcdctl_t {
  uint8_t
    lcdp:3,
    lcdmx:2,
    lcdson:1,
    unused:1,
    lcdon:1;
};
#else
struct __attribute__ ((packed)) lcdctl_t {
  uint8_t
    lcdon:1,
    unused:1,
    lcdson:1,
    lcdmx:2,
    lcdp:3;
};
#endif

/**
 * LCD Data Structure
 **/
struct msp430_lcd_t
{
  union {
    struct lcdctl_t b;
    uint8_t         s;
  } lcdctl;
  uint8_t mem[LCD_MEM_SIZE];
};

void   msp430_lcd_create(void);
void   msp430_lcd_reset(void);
void   msp430_lcd_update(void);
int8_t msp430_lcd_read (uint16_t addr);
void   msp430_lcd_write(uint16_t addr, int8_t val);

#else
#define msp430_lcd_create() do { } while (0)
#endif
#endif
