
/**
 *  \file   gdm_dev.c
 *  \brief  gdm1602a LCD display 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "devices/gdm1602a/gdm_dev.h"

#undef DEBUG

#ifdef DEBUG
#define HW_DMSG_GDM(x...) fprintf(stderr,x);
#else
#define HW_DMSG_GDM(x...) do {} while(0)
#endif

#define GDM_N_CHARS 16
#define GDM_N_LINES 2

#define GDM_RAM_CHARS 40

#define GDM_PIXEL_SIZE 4

#define GDM_MATRIX_X 5
#define GDM_MATRIX_Y 8

#define GDM_MATRIX_WIDTH  ((2 + (1 + GDM_MATRIX_X + 1) * GDM_N_CHARS + 2)*GDM_PIXEL_SIZE)
#define GDM_MATRIX_HEIGHT ((2 + (1 + GDM_MATRIX_Y + 1) * GDM_N_LINES + 2)*GDM_PIXEL_SIZE)

struct gdm_t {
  // drawing ui 
  int     x;
  int     y;
  // internal state
  uint8_t    display_on;
  uint8_t    cursor_on;
  uint8_t    cursor_blink;

  uint8_t    increment;
  uint8_t    shift_display;

  uint8_t    dd_ram[GDM_N_LINES][GDM_RAM_CHARS];
  uint8_t    dd_ram_window_start;
  uint8_t    dd_addr;
  uint8_t    dd_cursor;

  uint16_t   cg_addr;
  uint8_t    cg_ram[2][16];

  uint8_t    update_gfx;
  uint8_t    busy_flag;

  // command 
  int        rs;
  int        rw;
  int        enable;
  uint8_t    data;
  // timing 
  uint64_t   end_of_busy_time;
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

#define GDM_DATA        ((struct gdm_t*)(machine.device[dev].data))
#define GDM_GFX_UPDATE  GDM_DATA->update_gfx
#define DDRAM_CELL      GDM_DATA->dd_ram[(GDM_DATA->dd_addr & 0x40) ? 1:0][MIN((GDM_DATA->dd_addr & ~0x40),0x27)] 

int  gdm_reset       (int dev);
int  gdm_delete      (int dev);
void gdm_read        (int dev, uint32_t *mask, uint32_t *val);
void gdm_write       (int dev, uint32_t  mask, uint32_t  val);
int  gdm_update      (int dev);
int  gdm_ui_draw     (int dev);
void gdm_ui_get_size (int dev, int *w, int *h);
void gdm_ui_set_pos  (int dev, int  x, int  y);
void gdm_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

int gdm_device_size()
{
  return sizeof(struct gdm_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

int gdm_device_create(int dev_num)
{
  struct gdm_t *dev = (struct gdm_t*) machine.device[dev_num].data;

  memset(dev,0,sizeof(struct gdm_t));

  machine.device[dev_num].reset         = gdm_reset;
  machine.device[dev_num].delete        = gdm_delete;

  machine.device[dev_num].read          = gdm_read;
  machine.device[dev_num].write         = gdm_write;
  machine.device[dev_num].update        = gdm_update;

  machine.device[dev_num].ui_draw       = gdm_ui_draw;
  machine.device[dev_num].ui_get_size   = gdm_ui_get_size;
  machine.device[dev_num].ui_set_pos    = gdm_ui_set_pos;
  machine.device[dev_num].ui_get_pos    = gdm_ui_get_pos;

  machine.device[dev_num].state_size    = gdm_device_size();
  machine.device[dev_num].name          = "KS0066u LCD display";

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int gdm_reset(int dev)
{
  int x,y;
  // Warning: do not modify GDM_DATA->x and GDM_DATA->y
  
  x = GDM_DATA->x;
  y = GDM_DATA->y;

  memset(GDM_DATA,0,sizeof(struct gdm_t));

  GDM_DATA->x      = x;
  GDM_DATA->y      = y;
  GDM_GFX_UPDATE   = 1;
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int gdm_delete(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

static void gdm_set_idle_time(int dev, int nano)
{
  GDM_DATA->busy_flag        = 1;
  GDM_DATA->end_of_busy_time = MACHINE_TIME_GET_NANO() + nano;
  HW_DMSG_GDM("    LCD display busyflag delays %d nano\n",nano);
}

/***************************************************/
/***************************************************/
/***************************************************/

static void gdm_update_busyflag(int dev)
{
  if ((GDM_DATA->busy_flag == 1) && (MACHINE_TIME_GET_NANO() >= GDM_DATA->end_of_busy_time))
    {
      GDM_DATA->busy_flag = 0;
      HW_DMSG_GDM("** LCD display busyflag returns to 0\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void gdm_shift_display(int dev, int increment)
{
  int i;
  if (increment)
    {
      for(i=0; i < (GDM_RAM_CHARS - 1); i++)
	{
	  GDM_DATA->dd_ram[0][i] = GDM_DATA->dd_ram[0][i+1];
	  GDM_DATA->dd_ram[1][i] = GDM_DATA->dd_ram[1][i+1];
	}
      GDM_DATA->dd_ram[0][GDM_RAM_CHARS - 1] = 0x20;
      GDM_DATA->dd_ram[1][GDM_RAM_CHARS - 1] = 0x20;
      // HW_DMSG_GDM("GDM: shift display increment\n");
    }
  else
    {
      for(i=GDM_RAM_CHARS - 1; i > 0; i--)
	{
	  GDM_DATA->dd_ram[0][i] = GDM_DATA->dd_ram[0][i-1];
	  GDM_DATA->dd_ram[1][i] = GDM_DATA->dd_ram[1][i-1];
	}
      GDM_DATA->dd_ram[0][0] = 0x20;
      GDM_DATA->dd_ram[0][0] = 0x20;
      // HW_DMSG_GDM("GDM: shift display decrement\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void gdm_shift_cursor(int dev, int increment)
{
  if (increment)
    {
      switch (GDM_DATA->dd_cursor)
	{
	case 0x27:  GDM_DATA->dd_cursor  = 0x40; break;
	case 067:   GDM_DATA->dd_cursor  = 0x00; break;
	default:    GDM_DATA->dd_cursor += 1; break;
	}
      HW_DMSG_GDM("GDM: shift cursor increment\n");
    }
  else
    {
      switch (GDM_DATA->dd_cursor)
	{
	case 0x00: GDM_DATA->dd_cursor = 0x67; break;
	case 0x40: GDM_DATA->dd_cursor = 0x27; break;
	default: GDM_DATA->dd_cursor  -= 1; break;
	}
      HW_DMSG_GDM("GDM: shift cursor decrement\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

static void gdm_shift_addr(int dev, int increment)
{
  if (increment)
    {
      switch (GDM_DATA->dd_addr)
	{
	case 0x27:  GDM_DATA->dd_addr  = 0x40; break;
	case 067:   GDM_DATA->dd_addr  = 0x00; break;
	default:    GDM_DATA->dd_addr += 1; break;
	}
      HW_DMSG_GDM("GDM: shift address increment\n");
    }
  else
    {
      switch (GDM_DATA->dd_addr)
	{
	case 0x00: GDM_DATA->dd_addr = 0x67; break;
	case 0x40: GDM_DATA->dd_addr = 0x27; break;
	default: GDM_DATA->dd_addr  -= 1; break;
	}
      HW_DMSG_GDM("GDM: shift address decrement\n");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

#define GDM_BUSY_WAIT_39micro 39000       /* 39 µs */
#define GDM_BUSY_WAIT_153     1530000     /* 1.53 ms */

static void gdm_do_command(int dev)
{
  //
  //
  //

  if (GDM_DATA->data & 0x80)
    { // 8. set DD RAM address
      GDM_DATA->dd_addr = GDM_DATA->data & 0x7f;
      HW_DMSG_GDM("  =======================================\n");
      HW_DMSG_GDM("  LCD set ddram address 0x%x\n",GDM_DATA->dd_addr);
      HW_DMSG_GDM("  =======================================\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_39micro); // 39 µs
    }
  else if (GDM_DATA->data & 0x40)
    { // 7. set CG RAM address
      GDM_DATA->cg_addr = GDM_DATA->data & 0x3f;
      HW_DMSG_GDM("  =======================================\n");
      HW_DMSG_GDM("  LCD set cgram address %x\n",GDM_DATA->cg_addr);
      HW_DMSG_GDM("  =======================================\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_39micro); // 39 µs
    }
  else if (GDM_DATA->data & 0x20)
    { // 6. function set
      if ((GDM_DATA->data & 0x10) == 0)
	{
	  ERROR("  =======================================\n");
	  ERROR("  LCD display 4 bit control mode not supported\n");
	  ERROR("  =======================================\n");
	}
      if ((GDM_DATA->data & 0x8) == 0)
	{
	  ERROR("  =======================================\n");
	  ERROR("  LCD display one line display mode not supported\n");
	  ERROR("  =======================================\n");
	}
      if ((GDM_DATA->data & 0x4) == 0x4)
	{
	  ERROR("  =======================================\n");
	  ERROR("  LCD display 5x11 dots not supported\n");
	  ERROR("  =======================================\n");
	}
      HW_DMSG_GDM("  LCD function set\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_39micro); // 39 µs
    }
  else if (GDM_DATA->data & 0x10) 
    { // 5. cursor or display shift  
      int right = (GDM_DATA->data >> 2) & 1;
      if ((GDM_DATA->data >> 3) & 1)
	{ // display shift
	  gdm_shift_display(dev,right);
	  gdm_shift_cursor(dev,right);
	}
      else
	{ // cursor shift
	  gdm_shift_cursor(dev,right);
	  gdm_shift_addr(dev,right);
	}
      HW_DMSG_GDM("  ===========================\n");
      HW_DMSG_GDM("  LCD cursor or display shift\n");
      HW_DMSG_GDM("  ===========================\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_39micro); // 39 µs
    }
  else if (GDM_DATA->data & 0x08)
    { // 4. display on/off control
      GDM_DATA->display_on    = (GDM_DATA->data >> 2) & 1;
      GDM_DATA->cursor_on     = (GDM_DATA->data >> 1) & 1;
      GDM_DATA->cursor_blink  = (GDM_DATA->data >> 0) & 1;
      GDM_GFX_UPDATE = 1;
      HW_DMSG_GDM("  ================================================\n");
      HW_DMSG_GDM("  LCD display on/off display:%d cursor:%d blink:%d\n",
		  GDM_DATA->display_on,GDM_DATA->cursor_on,GDM_DATA->cursor_blink);
      HW_DMSG_GDM("  ================================================\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_39micro); // 39 µs
    }
  else if (GDM_DATA->data & 0x04)
    { // 3. entry mode set
      GDM_DATA->increment     = (GDM_DATA->data >> 1) & 1;
      GDM_DATA->shift_display = (GDM_DATA->data >> 0) & 1;
      HW_DMSG_GDM("  ================================================\n");
      HW_DMSG_GDM("  LCD entry mode set : increment = %d, shift = %d\n",GDM_DATA->increment,GDM_DATA->shift_display);
      HW_DMSG_GDM("  ================================================\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_39micro); // 39 µs
    }
  else if (GDM_DATA->data & 0x02)
    { // 2. Returns home : display being shifted to original position
      GDM_DATA->dd_addr             = 0;
      GDM_DATA->dd_cursor           = 0;
      GDM_DATA->dd_ram_window_start = 0;
      GDM_GFX_UPDATE = 1;
      HW_DMSG_GDM("  =======================================\n");
      HW_DMSG_GDM("  LCD return display to original position\n");
      HW_DMSG_GDM("  =======================================\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_153); // 1.53 ms
    }
  else if (GDM_DATA->data == 1)
    { // 1. clear display
      int c,l;
      for(l=0; l<GDM_N_LINES; l++)
	for(c=0; c<GDM_N_CHARS; c++)
	  {
	    GDM_DATA->dd_ram[l][c] = 0x20;
	  }
      GDM_DATA->dd_addr    = 0;
      GDM_DATA->dd_cursor  = 0;
      GDM_DATA->increment  = 1;
      GDM_GFX_UPDATE       = 1;
      HW_DMSG_GDM("  =================\n");
      HW_DMSG_GDM("  LCD clear command\n");
      HW_DMSG_GDM("  =================\n");
      gdm_set_idle_time(dev,GDM_BUSY_WAIT_153); // 1.53 ms
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int gdm_update(int dev)
{
  char ret = 0;
  gdm_update_busyflag(dev);
  if (GDM_DATA->busy_flag == 1)
    {
      return 0;
    }
  return ret;
}

/***************************************************/
/***************************************************/
/***************************************************/

void gdm_write(int dev, uint32_t addr, uint32_t val)
{
  if (addr & 0x0f00)
    {
      int rw,rs,en,enf;
      rw  = (val >> 2) & 1;
      rs  = (val >> 1) & 1;
      en  = (val & 1);
      enf = ((GDM_DATA->enable == 1) && (en == 0)) ? 1 : 0;


      if (enf == 1)
	{

	  // register select 
	  // 0 CG RAM
	  // 1 DD RAM
	  switch (GDM_DATA->rs)
	    {
	      // ********************************
	      // CG RAM
	      // ********************************
	    case 0: 
	      switch (GDM_DATA->rw)
		{
		case 0: // write 
		  gdm_do_command(dev);
		  break;
		case 1: // read busy flag and address
		  break;
		}
	      break;
	      
	      // ********************************
	      // DD RAM
	      // ********************************
	    case 1: 
	      switch (GDM_DATA->rw)
		{
		case 0: // write if GDM_DAT_UPDATE is true
		  DDRAM_CELL = GDM_DATA->data;
		  gdm_shift_addr(dev,GDM_DATA->increment);
		  
		  HW_DMSG_GDM("  =======================================\n");
		  HW_DMSG_GDM("  LCD write char 0x%x (%c) at cursor %x\n",GDM_DATA->data,isprint(GDM_DATA->data) ? GDM_DATA->data : '.',GDM_DATA->dd_addr);
		  HW_DMSG_GDM("  =======================================\n");
		  GDM_GFX_UPDATE = 1;
		  gdm_set_idle_time(dev,43000); // 43 µs
		  break;
		case 1: // read
		  break;
		}
	      break;
	      
	      // ********************************
	      // Default : error
	      // ********************************
	    default:
	      ERROR("GDM LCD : Should not come here (rs switch)\n");
	      break;
	    }
	  
	} /* enable falling edge */

      HW_DMSG_GDM("LCD display write CMD 0x%x (",val & 0x0f);
      if (GDM_DATA->rw != rw) {
	HW_DMSG_GDM("rw=%d ",rw);
      }
      if (GDM_DATA->rs != rs) {
	HW_DMSG_GDM("rs=%d ",rs);
      }
      if (GDM_DATA->enable != en) {
	HW_DMSG_GDM("en=%d falling=%d",en,enf);
      }
      HW_DMSG_GDM(")\n");
      
      GDM_DATA->rw             = rw;
      GDM_DATA->rs             = rs;
      GDM_DATA->enable         = en;
    }



  if (addr & 0x00ff)
    {
      GDM_DATA->data     = ((unsigned)val) & addr;
      HW_DMSG_GDM("LCD display write DAT 0x%04x (%c)\n",val & 0xff, isprint(val & 0xff) ? val & 0xff : '.');
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void gdm_read(int dev, uint32_t *addr, uint32_t *val)
{
  *addr = 0;
  switch (GDM_DATA->rs)
    {
    case 0:
      if (GDM_DATA->rw == 1)
	{
	  *addr = 0xff;
	  *val = ((GDM_DATA->busy_flag) ? 0x80 : 0x0) | GDM_DATA->dd_addr;
	  // HW_DMSG_GDM("**LCD display read busy flag = %d\n",GDM_DATA->busy_flag);
	  // ERROR("**LCD display read busy flag = %d\n",GDM_DATA->busy_flag);
	}
      break;
    case 1:
      if (GDM_DATA->rw == 1)
	{
	  *addr = 0xff;
	  *val = DDRAM_CELL;
	  // HW_DMSG_GDM("**LCD display read ram cell\n");
	  // ERROR("**LCD display read ram cell\n");
	}
      break;
    }
}

/***************************************************/
/* DRAWING *****************************************/
/***************************************************/

static void gdm_drawchar(int dev, int l, int c, uint8_t map[7])
{
  int x0,y0,x,y;
  int pixel,p,pp;

  x0 = GDM_DATA->x + (2 + 1 + c * (1+ GDM_MATRIX_X +1)) * GDM_PIXEL_SIZE;
  y0 = GDM_DATA->y + (2 + 1 + l * (1+ GDM_MATRIX_Y +1)) * GDM_PIXEL_SIZE;

  //HW_DMSG_GDM("  lcd drawing char at %dx%d\n",x0,y0);

  pixel = (x0 + y0*machine.ui.width)*machine.ui.bpp;

  for(y=0; y<GDM_MATRIX_Y; y++) // lines
    {
      for(p=0; p<GDM_PIXEL_SIZE; p++) 
	{
	  int pii = pixel; 
	  for(x=0; x<GDM_MATRIX_X; x++) // columns
	    {
	      int b = (map[y] >> (4 - x)) & 1;
	      for(pp=0; pp<GDM_PIXEL_SIZE; pp++)
		{
		  if (b)
		    {
		      setpixel(pii,0x55,0x55,0x55); /* on */
		    }
		  else
		    {
		      setpixel(pii,0x10,0x10,0x10); /* off */
		    }
		  pii += 3;
		}
	    }
	  pixel += machine.ui.width * machine.ui.bpp;
	}
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int gdm_ui_draw (int dev)
{
  int l,c;
#include "gdm_font.h"

  if ((GDM_GFX_UPDATE == 0) || (GDM_DATA->display_on == 0))
    return 0;

  for(l=0; l < GDM_N_LINES; l++)
    {
      for(c=0; c<GDM_N_CHARS; c++)
	{
	  gdm_drawchar(dev,l,c,gdm_font[GDM_DATA->dd_ram[l][c] >> 4][GDM_DATA->dd_ram[l][c] & 0xf]);
	}
    }


  GDM_GFX_UPDATE = 0;
  return 1;
}

/***************************************************/
/***************************************************/
/***************************************************/

void gdm_ui_get_size (int UNUSED dev, int *w, int *h)
{
  *w = GDM_MATRIX_WIDTH;
  *h = GDM_MATRIX_HEIGHT;
  HW_DMSG_GDM("  lcd getting ui size (%d,%d)\n",*w,*h);
}

/***************************************************/
/***************************************************/
/***************************************************/

void gdm_ui_set_pos (int dev, int x, int y)
{
  HW_DMSG_GDM("  lcd setting ui pos (%d,%d)\n",x,y);
  GDM_DATA->x = x;
  GDM_DATA->y = y;
}

void gdm_ui_get_pos (int dev, int *x, int *y)
{
  *x = GDM_DATA->x;
  *y = GDM_DATA->y;
}

/***************************************************/
/***************************************************/
;/***************************************************/
