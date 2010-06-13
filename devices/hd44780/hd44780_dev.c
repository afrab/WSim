
/**
 *  \file   hd44780_dev.c
 *  \brief  HD44780U (LCD-II) Hitachi dot-matrix LCD
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "arch/common/hardware.h"
#include "devices/hd44780/hd44780_dev.h"

#if defined(DEBUG)
#define HW_DMSG_HD44(x...) VERBOSE(6,x)
#else
#define HW_DMSG_HD44(x...) do {} while(0)
#endif

#define HD44_N_LINES     2 
#define HD44_N_COLS     16 /* 16 char drawn on matrix */
#define HD44_RAM_CHARS  40 /* buffer memory           */

#define HD44_MATRIX_X    5 /* */
#define HD44_MATRIX_Y    8 /* */
#define HD44_MATRIX_S    4 /* pixel per matrix dot */

/* 2 || 1 || 1 || 2 */
#define HD44_MATRIX_WIDTH  ((2 + (HD44_MATRIX_X + 1) * HD44_N_COLS  + 1) * HD44_MATRIX_S)
#define HD44_MATRIX_HEIGHT ((2 + (HD44_MATRIX_Y + 1) * HD44_N_LINES + 1) * HD44_MATRIX_S)



struct hd44_t {

  // internal state
  uint8_t    IR;            /* instruction register                   */
  uint8_t    DR;            /* data register                          */

  uint8_t    datalength;    /* 0: 4bit data         1: 8bit data      */
  uint8_t    linedisplay;   /* 0: 1 line display    1:                */
  uint8_t    fontsize;      /* 0: 5x8 char font     1:                */
  uint8_t    display_on;    /* 0: display off       1:                */
  uint8_t    cursor_on;     /* 0: cursor off        1:                */
  uint8_t    blinking_on;   /* 0: blinking off      1:                */
  uint8_t    increment;     /* 0: none              1: increment by 1 */
  uint8_t    display_shift; /* 0: no shift          1:                */

  uint8_t    dd_ram[HD44_N_LINES][HD44_RAM_CHARS];
  uint8_t    dd_ram_window_start;
  uint8_t    dd_addr;
  uint8_t    dd_cursor;

  uint16_t   cg_addr;
  uint8_t    cg_ram[2][16];


  // timing 
  uint8_t    busy_flag;
  uint64_t   end_of_busy_time;

  // pins state
  int        rs;            /* register selection                     */
  int        rw;            /* read write                             */
  int        enable;        /* enable                                 */
  uint8_t    d0d3;          /* data on port                           */
  uint8_t    d4d7;          /* data on port                           */
  uint8_t    second_half;   /* data is second half of byte            */

  // drawing ui 
  uint32_t   gfx_x;
  uint32_t   gfx_y;
  uint32_t   gfx_on;
  uint32_t   gfx_off;
  uint32_t   gfx_bg;
  uint8_t    gfx_update;
};

/** **************************
 *        5
 *    . . . . . 
 *    . . . . . 
 *    . . . . . 
 * 7  . . . . . 
 *    . . . . . 
 *    . . . . .
 *    . . . . . 
 *    /////////  cursor
 *
 *  X (2 + (1 + 5 + 1)*i + 2)
 *  Y (2 + (1 + 7 + 1)*i + 2)
 * pixel size = 2*2 square
 *
 * ****************************/

#define MIN(a,b) ((a<b)?a:b)

#define HD44_DATA        ((struct hd44_t*)(machine.device[dev].data))
#define HD44_GFX_UPDATE  HD44_DATA->gfx_update
#define DDRAM_CELL       HD44_DATA->dd_ram[(HD44_DATA->dd_addr & 0x40) ? 1:0][MIN((HD44_DATA->dd_addr & ~0x40),0x27)] 

int  hd44_reset       (int dev);
int  hd44_delete      (int dev);
void hd44_read        (int dev, uint32_t *mask, uint32_t *val);
void hd44_write       (int dev, uint32_t  mask, uint32_t  val);
int  hd44_update      (int dev);
int  hd44_ui_draw     (int dev);
void hd44_ui_get_size (int dev, int *w, int *h);
void hd44_ui_set_pos  (int dev, int  x, int  y);
void hd44_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

int hd44_device_size()
{
  return sizeof(struct hd44_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

int hd44_device_create(int dev, uint32_t on, uint32_t off, uint32_t bg)
{
  struct hd44_t *hddev = (struct hd44_t*) machine.device[dev].data;

  memset(hddev,0,sizeof(struct hd44_t));

  machine.device[dev].reset         = hd44_reset;
  machine.device[dev].delete        = hd44_delete;

  machine.device[dev].read          = hd44_read;
  machine.device[dev].write         = hd44_write;
  machine.device[dev].update        = hd44_update;

  machine.device[dev].ui_draw       = hd44_ui_draw;
  machine.device[dev].ui_get_size   = hd44_ui_get_size;
  machine.device[dev].ui_set_pos    = hd44_ui_set_pos;
  machine.device[dev].ui_get_pos    = hd44_ui_get_pos;

  machine.device[dev].state_size    = hd44_device_size();
  machine.device[dev].name          = "hd44780u LCD display";

  HD44_DATA->gfx_on                 = on;
  HD44_DATA->gfx_off                = off;
  HD44_DATA->gfx_bg                 = bg;

  HW_DMSG_HD44("HD44: on  set to 0x%06x\n",HD44_DATA->gfx_on);
  HW_DMSG_HD44("HD44: off set to 0x%06x\n",HD44_DATA->gfx_off);
  HW_DMSG_HD44("HD44: bg  set to 0x%06x\n",HD44_DATA->gfx_bg);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int hd44_reset(int dev)
{
  // Warning: do not modify HD44_DATA->x and HD44_DATA->y
  
  HD44_DATA->datalength    = 1;
  HD44_DATA->linedisplay   = 0;
  HD44_DATA->fontsize      = 0;
  HD44_DATA->display_on    = 0;
  HD44_DATA->cursor_on     = 0;
  HD44_DATA->blinking_on   = 0;
  HD44_DATA->increment     = 1;
  HD44_DATA->display_shift = 0;

  HD44_DATA->second_half   = 0;
  HD44_GFX_UPDATE          = 1;
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int hd44_delete(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

static void hd44_set_idle_time(int dev, int nano)
{
  HD44_DATA->busy_flag        = 1;
  HD44_DATA->end_of_busy_time = MACHINE_TIME_GET_NANO() + nano;
  /* HW_DMSG_HD44("    LCD display busyflag delays %d nano\n",nano); */
}

/***************************************************/
/***************************************************/
/***************************************************/

static void hd44_update_busyflag(int dev)
{
  if ((HD44_DATA->busy_flag == 1) && (MACHINE_TIME_GET_NANO() >= HD44_DATA->end_of_busy_time))
    {
      HD44_DATA->busy_flag = 0;
      /* HW_DMSG_HD44(" ** LCD display busyflag returns to 0\n"); */
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void hd44_shift_display(int dev, int increment)
{
  int i;
  if (increment)
    {
      for(i=0; i < (HD44_RAM_CHARS - 1); i++)
	{
	  HD44_DATA->dd_ram[0][i] = HD44_DATA->dd_ram[0][i+1];
	  HD44_DATA->dd_ram[1][i] = HD44_DATA->dd_ram[1][i+1];
	}
      HD44_DATA->dd_ram[0][HD44_RAM_CHARS - 1] = 0x20;
      HD44_DATA->dd_ram[1][HD44_RAM_CHARS - 1] = 0x20;
      // HW_DMSG_HD44("HD44: shift display increment\n");
    }
  else
    {
      for(i=HD44_RAM_CHARS - 1; i > 0; i--)
	{
	  HD44_DATA->dd_ram[0][i] = HD44_DATA->dd_ram[0][i-1];
	  HD44_DATA->dd_ram[1][i] = HD44_DATA->dd_ram[1][i-1];
	}
      HD44_DATA->dd_ram[0][0] = 0x20;
      HD44_DATA->dd_ram[0][0] = 0x20;
      // HW_DMSG_HD44("HD44: shift display decrement\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void hd44_shift_cursor(int dev, int increment)
{
  if (increment)
    {
      switch (HD44_DATA->dd_cursor)
	{
	case 0x27:  HD44_DATA->dd_cursor  = 0x40; break;
	case 067:   HD44_DATA->dd_cursor  = 0x00; break;
	default:    HD44_DATA->dd_cursor += 1; break;
	}
      HW_DMSG_HD44("HD44: shift cursor increment\n");
    }
  else
    {
      switch (HD44_DATA->dd_cursor)
	{
	case 0x00: HD44_DATA->dd_cursor = 0x67; break;
	case 0x40: HD44_DATA->dd_cursor = 0x27; break;
	default: HD44_DATA->dd_cursor  -= 1; break;
	}
      HW_DMSG_HD44("HD44: shift cursor decrement\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void hd44_shift_addr(int dev, int increment)
{
  if (increment)
    {
      switch (HD44_DATA->dd_addr)
	{
	case 0x27:  HD44_DATA->dd_addr  = 0x40; break;
	case 067:   HD44_DATA->dd_addr  = 0x00; break;
	default:    HD44_DATA->dd_addr += 1; break;
	}
      HW_DMSG_HD44("HD44: shift address increment\n");
    }
  else
    {
      switch (HD44_DATA->dd_addr)
	{
	case 0x00: HD44_DATA->dd_addr = 0x67; break;
	case 0x40: HD44_DATA->dd_addr = 0x27; break;
	default: HD44_DATA->dd_addr  -= 1; break;
	}
      HW_DMSG_HD44("HD44: shift address decrement\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

#define HD44_BUSY_WAIT_39micro 39000       /* 39 µs */
#define HD44_BUSY_WAIT_153     1530000     /* 1.53 ms */

static void hd44_do_command(int dev)
{
  //
  // commands are defined from left to right 
  //

  if (HD44_DATA->IR & 0x80)
    { // 8. set DD RAM address
      HD44_DATA->dd_addr = HD44_DATA->IR & 0x7f;
      HW_DMSG_HD44("  =======================================\n");
      HW_DMSG_HD44("  LCD set ddram address 0x%x\n",HD44_DATA->dd_addr);
      HW_DMSG_HD44("  =======================================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_39micro); // 39 µs
    }
  else if (HD44_DATA->IR & 0x40)
    { // 7. set CG RAM address
      HD44_DATA->cg_addr = HD44_DATA->IR & 0x3f;
      HW_DMSG_HD44("  =======================================\n");
      HW_DMSG_HD44("  LCD set cgram address %x\n",HD44_DATA->cg_addr);
      HW_DMSG_HD44("  =======================================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_39micro); // 39 µs
    }
  else if ((HD44_DATA->IR & 0x20) != 0)
    { // 6. function set
      HD44_DATA->datalength  = (HD44_DATA->IR & 0x10) >> 4;
      HD44_DATA->linedisplay = (HD44_DATA->IR & 0x08) >> 3;
      HD44_DATA->fontsize    = (HD44_DATA->IR & 0x04) >> 2;
      HW_DMSG_HD44("  =======================================\n");
      HW_DMSG_HD44("  LCD function set: DL=%d N=%d F=%d\n",
		   HD44_DATA->datalength, HD44_DATA->linedisplay, HD44_DATA->fontsize);
      HW_DMSG_HD44("  =======================================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_39micro); // 39 µs
    }
  else if (HD44_DATA->IR & 0x10) 
    { // 5. cursor or display shift  
      int right = (HD44_DATA->IR >> 2) & 1;
      if ((HD44_DATA->IR >> 3) & 1)
	{ // display shift
	  hd44_shift_display(dev,right);
	  hd44_shift_cursor(dev,right);
	}
      else
	{ // cursor shift
	  hd44_shift_cursor(dev,right);
	  hd44_shift_addr(dev,right);
	}
      HW_DMSG_HD44("  ===========================\n");
      HW_DMSG_HD44("  LCD cursor or display shift\n");
      HW_DMSG_HD44("  ===========================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_39micro); // 39 µs
    }
  else if (HD44_DATA->IR & 0x08)
    { // 4. display on/off control
      HD44_DATA->display_on    = (HD44_DATA->IR >> 2) & 1;
      HD44_DATA->cursor_on     = (HD44_DATA->IR >> 1) & 1;
      HD44_DATA->blinking_on   = (HD44_DATA->IR >> 0) & 1;
      HD44_GFX_UPDATE = 1;
      HW_DMSG_HD44("  ================================================\n");
      HW_DMSG_HD44("  LCD display on/off display:%d cursor:%d blink:%d\n",
		  HD44_DATA->display_on,HD44_DATA->cursor_on,HD44_DATA->blinking_on);
      HW_DMSG_HD44("  ================================================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_39micro); // 39 µs
    }
  else if (HD44_DATA->IR & 0x04)
    { // 3. entry mode set
      HD44_DATA->increment     = (HD44_DATA->IR >> 1) & 1;
      HD44_DATA->display_shift = (HD44_DATA->IR >> 0) & 1;
      HW_DMSG_HD44("  ================================================\n");
      HW_DMSG_HD44("  LCD entry mode set : increment = %d, shift = %d\n",
		   HD44_DATA->increment,HD44_DATA->display_shift);
      HW_DMSG_HD44("  ================================================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_39micro); // 39 µs
    }
  else if (HD44_DATA->IR & 0x02)
    { // 2. Returns home : display being shifted to original position
      HD44_DATA->dd_addr             = 0;
      HD44_DATA->dd_cursor           = 0;
      HD44_DATA->dd_ram_window_start = 0;
      HD44_GFX_UPDATE = 1;
      HW_DMSG_HD44("  =======================================\n");
      HW_DMSG_HD44("  LCD return display to original position\n");
      HW_DMSG_HD44("  =======================================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_153); // 1.53 ms
    }
  else if (HD44_DATA->IR == 1)
    { // 1. clear display
      int c,l;
      for(l=0; l<HD44_N_LINES; l++)
	for(c=0; c<HD44_N_COLS; c++)
	  {
	    HD44_DATA->dd_ram[l][c] = 0x20;
	  }
      HD44_DATA->dd_addr    = 0;
      HD44_DATA->dd_cursor  = 0;
      HD44_DATA->increment  = 1;
      HD44_GFX_UPDATE       = 1;
      HW_DMSG_HD44("  =================\n");
      HW_DMSG_HD44("  LCD clear command\n");
      HW_DMSG_HD44("  =================\n");
      hd44_set_idle_time(dev,HD44_BUSY_WAIT_153); // 1.53 ms
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void hd44_do_data(int dev)
{
  HW_DMSG_HD44("  =======================================\n");
  HW_DMSG_HD44("  LCD write char 0x%x (%c) at cursor %x\n",
	       HD44_DATA->DR,
	       isprint(HD44_DATA->DR) ? HD44_DATA->DR : '.', HD44_DATA->dd_addr);
  HW_DMSG_HD44("  =======================================\n");
  HD44_GFX_UPDATE = 1;
  hd44_set_idle_time(dev,43000); // 43 µs
  DDRAM_CELL = HD44_DATA->DR;
  hd44_shift_addr(dev,HD44_DATA->increment);
}

/***************************************************/
/***************************************************/
/***************************************************/

int hd44_update(int dev)
{
  char ret = 0;
  hd44_update_busyflag(dev);
  if (HD44_DATA->busy_flag == 1)
    {
      return 0;
    }
  return ret;
}

/***************************************************/
/***************************************************/
/***************************************************/

static int hd44_write_xR(int dev, uint8_t *xR, char UNUSED *regname)
{
  int ret = 0;
  if (HD44_DATA->datalength == 0)
    {
      if (HD44_DATA->second_half == 1) 
	{
	  *xR                    = (*xR & 0xf0) | ((HD44_DATA->d4d7 & 0x0f) << 0);
	  HD44_DATA->second_half = 0;
	  HW_DMSG_HD44("LCD display write(4) reg[0xff] %s 0x%04x (%c)\n", 
		       regname, *xR, isprint(*xR) ? *xR : '.');
	  ret                    = 1;
	}
      else
	{
	  *xR                    = (*xR & 0x0f) | ((HD44_DATA->d4d7 & 0x0f) << 4);
	  HD44_DATA->second_half = 1;
	  HW_DMSG_HD44("LCD display write(4) reg[0xf0] %s 0x%04x (%c)\n", 
		       regname, *xR, isprint(*xR) ? *xR : '.');
	  ret                    = 0;
	}
    }
  else /* 8 bit */
    {
      *xR = ((HD44_DATA->d4d7 & 0xf) << 4) | (HD44_DATA->d0d3 & 0xf);
      HW_DMSG_HD44("LCD display write reg %s 0x%04x (%c)\n", 
		   regname, *xR, isprint(*xR) ? *xR : '.');
      ret = 1;
    }
  return ret;
}

/***************************************************/
/***************************************************/
/***************************************************/

void hd44_write(int dev, uint32_t mask, uint32_t value)
{
  if ((mask & HD44_D0D3) != 0)
    {
      HD44_DATA->d0d3 = ((unsigned)value & HD44_D0D3) >> HD44_D0D3_S;
    }

  if ((mask & HD44_D4D7) != 0)
    {
      HD44_DATA->d4d7 = ((unsigned)value & HD44_D4D7) >> HD44_D4D7_S;
    }

  if (mask & (HD44_RS | HD44_RW | HD44_E))
    {
      int rw,rs,en;
      int falling_en;
      rw         = (value >> HD44_RW_S) & 1;
      rs         = (value >> HD44_RS_S) & 1;
      en         = (value >> HD44_E_S ) & 1;
      falling_en = ((HD44_DATA->enable == 1) && (en == 0)) ? 1 : 0;

      if (falling_en == 1) 
	{
	  switch (HD44_DATA->rs)
	    {
	    case 0:     /* instruction/address        */
	      switch (HD44_DATA->rw)
		{
		case 0: /* instruction register       */
		  if (hd44_write_xR(dev, & HD44_DATA->IR, "IR") == 1)
		    {
		      hd44_do_command(dev);
		    }
		  break;
		case 1: /* read busy flag and address */
		  break;
		}
	      break;
	      
	    case 1:     /* data                      */
	      switch (HD44_DATA->rw)
		{
		case 0: /* write */
		  if (hd44_write_xR(dev, & HD44_DATA->DR, "DR") == 1)
		    {
		      hd44_do_data(dev);
		    }
		  break;
		case 1: /* read */
		  break;
		}
	      break;
	    }
	}
      
      if (HD44_DATA->rw != rw)
	{
	  HD44_DATA->rw             = rw;
	  HD44_DATA->second_half    = 0;
	}
      if (HD44_DATA->rs != rs)
	{
	  HD44_DATA->rs             = rs;
	  HD44_DATA->second_half    = 0;
	}
      if (HD44_DATA->enable != en)
	{
	  HD44_DATA->enable         = en;
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void hd44_read(int dev, uint32_t *mask, uint32_t *value)
{
  *mask = 0;
  switch (HD44_DATA->rs)
    {
    case 0:
      if (HD44_DATA->rw == 1)
	{
	  *mask   = 0xff;
	  *value  = ((HD44_DATA->busy_flag) ? 0x80 : 0x0) | HD44_DATA->dd_addr;
	  // HW_DMSG_HD44("**LCD display read busy flag = %d\n",HD44_DATA->busy_flag);
	  // ERROR("**LCD display read busy flag = %d\n",HD44_DATA->busy_flag);
	}
      break;
    case 1:
      if (HD44_DATA->rw == 1)
	{
	  *mask   = 0xff;
	  *value  = DDRAM_CELL;
	  // HW_DMSG_HD44("**LCD display read ram cell\n");
	  // ERROR("**LCD display read ram cell\n");
	}
      break;
    }
}

/***************************************************/
/* DRAWING *****************************************/
/***************************************************/

static void hd44_drawchar(int dev, int l, int c, uint8_t map[7])
{
  int  x0,y0,x,y;
  int  pixel,p,pp;
  unsigned char or,og,ob;
  unsigned char Or,Og,Ob;
  unsigned char Br,Bg,Bb;

  /* #define HD44_MATRIX_WIDTH  ((2 + (HD44_MATRIX_X + 1) * HD44_N_COLS  + 1) * HD44_MATRIX_S) */
  /* #define HD44_MATRIX_HEIGHT ((2 + (HD44_MATRIX_Y + 1) * HD44_N_LINES + 1) * HD44_MATRIX_S) */

  x0 = HD44_DATA->gfx_x + (2 + (HD44_MATRIX_X + 1) * c) * HD44_MATRIX_S;
  y0 = HD44_DATA->gfx_y + (2 + (HD44_MATRIX_Y + 1) * l) * HD44_MATRIX_S;

  or = (HD44_DATA->gfx_on  >> 16) & 0xff;
  og = (HD44_DATA->gfx_on  >>  8) & 0xff;
  ob = (HD44_DATA->gfx_on  >>  0) & 0xff;

  Or = (HD44_DATA->gfx_off >> 16) & 0xff;
  Og = (HD44_DATA->gfx_off >>  8) & 0xff;
  Ob = (HD44_DATA->gfx_off >>  0) & 0xff;

  Br = (HD44_DATA->gfx_bg  >> 16) & 0xff;
  Bg = (HD44_DATA->gfx_bg  >>  8) & 0xff;
  Bb = (HD44_DATA->gfx_bg  >>  0) & 0xff;

  /* ERROR("HD44: on/off/bg %02x%02x%02x %02x%02x%02x %02x%02x%02x\n", or,og,ob,Or,Og,Ob,Br,Bg,Bb); */
  /* HW_DMSG_HD44("  lcd drawing char at %dx%d\n",x0,y0); */

  pixel = (x0 + y0*machine.ui.width)*machine.ui.bpp;

  // draw letters
  for(y=0; y < HD44_MATRIX_Y; y++)                  /* char height  */
    {
      for(p=0; p < HD44_MATRIX_S; p++)              /* matrix point */
	{
	  int pii = pixel; 
	  for(x=0; x<HD44_MATRIX_X; x++)            /* char width   */
	    {
	      int b = (map[y] >> (HD44_MATRIX_S - x)) & 1;
	      for(pp=0; pp < HD44_MATRIX_S; pp++)   /* matrix point */
		{
		  if (b)
		    {
		      setpixel(pii,or,og,ob); /* on */
		    }
		  else
		    {
		      setpixel(pii,Or,Og,Ob); /* off */
		    }
		  pii += machine.ui.bpp;
		}
	    }
	  pixel += machine.ui.width * machine.ui.bpp;
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/
#include "hd44780_font.h"

int hd44_ui_draw (int dev)
{
  int l,c;
  /*   uint8_t null_map[7] = { 0, 0, 0, 0, 0, 0, 0 }; */

  if ((HD44_GFX_UPDATE == 0))
    {
      return 0;
    }

  HW_DMSG_HD44("HD44: draw update\n");


  if (HD44_DATA->display_on == 0)
    {
      int x0    = HD44_DATA->gfx_x + 3;
      int y0    = HD44_DATA->gfx_y + 3;
      int pixel = (y0 * machine.ui.width + x0) * machine.ui.bpp;
      
      for(l=0; l < (HD44_MATRIX_HEIGHT - 6); l++)
	{
	  pixel = ((y0 + l) * machine.ui.width + x0) * machine.ui.bpp;
	  for(c=0; c < (HD44_MATRIX_WIDTH - 6); c++)
	    {
	      setpixel(pixel, 0x10, 0x10, 0x10);
	      pixel += machine.ui.bpp;
	    }
	}
    }
  

  if (HD44_DATA->display_on == 1)
    {
      for(l=0; l < HD44_N_LINES; l++)
	{
	  for(c=0; c < HD44_N_COLS; c++)
	    {
	      hd44_drawchar(dev,l,c,hd44_font[HD44_DATA->dd_ram[l][c] >> 4][HD44_DATA->dd_ram[l][c] & 0xf]);
	    }
	}
    }

  HD44_GFX_UPDATE = 0;
  return 1;
}

/***************************************************/
/***************************************************/
/***************************************************/

void hd44_ui_get_size (int UNUSED dev, int *w, int *h)
{
  *w = HD44_MATRIX_WIDTH;
  *h = HD44_MATRIX_HEIGHT;
  HW_DMSG_HD44("HD44:  lcd getting ui size (%d,%d)\n",*w,*h);
}

/***************************************************/
/***************************************************/
/***************************************************/

void hd44_ui_set_pos (int dev, int x, int y)
{
  HW_DMSG_HD44("HD44:  lcd setting ui pos  (%d,%d)\n",x,y);
  HD44_DATA->gfx_x = x;
  HD44_DATA->gfx_y = y;
}

void hd44_ui_get_pos (int dev, int *x, int *y)
{
  *x = HD44_DATA->gfx_x;
  *y = HD44_DATA->gfx_y;
}

/***************************************************/
/***************************************************/
;/***************************************************/
