
/**
 *  \file   ot2007.c
 *  \brief  Platform devices handling functions, OT Setre 2007 edition 
 *  \author Bruno Allard, Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"
#include "devices/devices.h"
#include "devices/7seg/7seg_dev.h"
#include "devices/led/led_dev.h"
#include "devices/hd44780/hd44780_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/bargraph/bargraph_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

/**
 * OT Setre platform 
 *
 *  port 1 : DALLAS (7)
 *  port 2 : 
 *           0 NC
 *           1 LED-status (1)
 *           2 LCD RS
 *           3 LCD E
 *          4-7 LCD D4-D7
 *  port 3 : ethernet controller (control 0 to 3), serial port (TX=4, RX=5)
 *  port 4 : Buttons (4 to 7), Buzzer (2, 3)
 *  port 5 : ethernet controller data
 *  port 6 : extension
 *    0: D    (bargraph)	
 *    1: S0   (bargraph)
 *    2: S1   (bargraph)
 *    3: S2   (bargraph)
 *    4: DIN  (DAC, volume)
 *    5: SCLK (DAC, volume)
 *    6: /CS  (DAC, volume)
 *    7: Select for bargraph(LEb, 14), enable_DAC, /enable_volume, /LED_toggle
 **/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM        0 
#define LEDstatus     1
#define BAGR          2
#define BLED9         3
#define BLED10        4
#define LCD           5
#define SERIAL        6

#define LOGO1         7
#define LOGO2         8

#define END_DEV       LOGO1
#define BOARD_DEVICE_MAX (END_DEV+1)

//#define DAC           4
//#define VOL           5

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add()
{
  ptty_add_options(SERIAL,0,"serial0");
  VERBOSE(2,"ot2007: boutton 1 = 'a'\n");
  VERBOSE(2,"ot2007: boutton 2 = 'z'\n");
  VERBOSE(2,"ot2007: boutton 3 = 'e'\n");
  VERBOSE(2,"ot2007: boutton 4 = 'r'\n");
  VERBOSE(2,"ot2007:\n");
  VERBOSE(2,"ot2007: 'q' pour quitter\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct ot2007_struct_t {
  uint8_t bar_latch[8];
  int port6_lastvalue;
  int buttons_lastvalue;
  int ui_loop_count;
};

#define SYSTEM_DATA   ((struct ot2007_struct_t*)(machine.device[SYSTEM].data))
#define BAR_LATCH     SYSTEM_DATA->bar_latch
#define PORT6_LAST    SYSTEM_DATA->port6_lastvalue
#define BUTTONS_LAST  SYSTEM_DATA->buttons_lastvalue
#define UI_LOOP_COUNT SYSTEM_DATA->ui_loop_count

int system_reset (int UNUSED dev) 
{ 
  BUTTONS_LAST  = 0xf0;
  UI_LOOP_COUNT = 0;
  return 0; 
}

int system_delete(int UNUSED dev) 
{ 
  return 0; 
}

int system_create(int dev_num)
{
  machine.device[dev_num].reset         = system_reset;
  machine.device[dev_num].delete        = system_delete;
  machine.device[dev_num].state_size    = sizeof(struct ot2007_struct_t);
  machine.device[dev_num].name          = "System Platform";
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create()
{
  /* int i; */
  int res = 0;

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/

  res += msp430_mcu_create(8000000 /* xin_freq*/, 0 /* xt2in_freq */);

  /*********************************/
  /* begin platform specific part  */
  /*********************************/

  /* easyweb2 */
  machine.device_max             = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM]    = sizeof(struct ot2007_struct_t);
  machine.device_size[LEDstatus] = led_device_size();
  machine.device_size[LCD]       = hd44_device_size();
  machine.device_size[SERIAL]    = ptty_device_size();
  /* ot extension */
  machine.device_size[BAGR]      = bargraph_device_size();
  machine.device_size[BLED9]     = led_device_size();
  machine.device_size[BLED10]    = led_device_size();
  /* uigfx */
  machine.device_size[LOGO1]     = uigfx_device_size();

  /*********************************/
  /* end of platform specific part */
  /*********************************/

  res += devices_memory_allocate();

  /*********************************/
  /* begin platform specific part  */
  /*********************************/

#if 0
#define XPM_FILE insa_ot08.xpm
#define XPM_NAME SgpdZq
#define BKG 0xefefef
#define OFF 0x202020
#else
#define XPM_FILE wsim.xpm
#define XPM_NAME wsim
#define BKG 0xffffff
#define OFF 0x202020
#include "wsim.xpm"
#endif

  machine.ui.framebuffer_background = BKG;

  res += system_create          (SYSTEM);
  res += led_device_create      (LEDstatus, 0xee0000, OFF, BKG, "status");
  res += hd44_device_create     (LCD,       0xaaaaaa, OFF, BKG);
  res += ptty_device_create     (SERIAL,    0);
  res += bargraph_device_create (BAGR,      0xee1010, OFF, BKG);
  res += led_device_create      (BLED9,     0x00ee00, OFF, BKG, "led9");
  /* res += led_device_create      (BLED10,    0x001e00, OFF, BKG); */
  res += led_device_create      (BLED10,    0x00ee00, OFF, BKG, "led10");

  res += uigfx_device_create    (LOGO1,     wsim);

  /*********************************/
  /* end of platform specific part */
  /*********************************/

  {
    int bar_w,bar_h;
    int led_w,led_h;
    int lcd_w,lcd_h;
    int log_w,log_h;

    machine.device[LEDstatus].ui_get_size(LEDstatus, &led_w, &led_h   );
    machine.device[BAGR].ui_get_size     (BAGR,      &bar_w, &bar_h   );
    machine.device[LCD].ui_get_size      (LCD,       &lcd_w, &lcd_h   );
    machine.device[LOGO1].ui_get_size    (LOGO1,     &log_w, &log_h   );

    machine.device[LEDstatus].ui_set_pos (LEDstatus,  5          , 10 );
    machine.device[BLED9].ui_set_pos     (BLED9,      5 + 1*led_w, 10 );
    machine.device[BLED10].ui_set_pos    (BLED10,     5 + 2*led_w, 10 );
    
    machine.device[BAGR].ui_set_pos      (BAGR,       0, log_h - bar_h);
    machine.device[LCD].ui_set_pos       (LCD,        0, log_h        );

    machine.device[LOGO1].ui_set_pos     (LOGO1,  lcd_w - log_w - 10, 0);
  }
  
  /*********************************/
  /* end of platform specific part */
  /*********************************/

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* this function is called after devices reset    */
/* devices init conditions should be written here */
int devices_reset_post()
{
  int refresh = 0;

  /* p1.7 dallas, niveau haut */
  msp430_digiIO_dev_write(PORT1, 0x80, 0x80);
  BUTTONS_LAST = 0xf0;

  /* p4 boutons */
  msp430_digiIO_dev_write(PORT4, 0xf0, 0xf0);
  
  REFRESH(BAGR);
  REFRESH(LEDstatus);
  REFRESH(BLED9);
  REFRESH(BLED10);
  REFRESH(LCD);
  REFRESH(LOGO1);

  if (refresh)
    {
      ui_refresh();
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update()
{
  int res       = 0;
  uint8_t  val8 = 0;
  int refresh   = 0;
  
  /* ************************* */
  /* MCU -> devices            */
  /* ************************* */

  /* LEDstatus sur P2.1 
   * LCD       P2.2-P2.7
   */
  if (msp430_digiIO_dev_read(PORT2,&val8))
    {
      machine.device[LEDstatus].write(LEDstatus,1,1-BIT(val8,1)); /* inversion */
      UPDATE(LEDstatus);
      REFRESH(LEDstatus);

      /* lcd RW is tied to 0 */
      machine.device[LCD].write(LCD, 
				HD44_RS | HD44_RW | HD44_E | HD44_D4D7,
				((val8 >> 2) & 0x1) << HD44_RS_S |
				(              0x0) << HD44_RW_S |
				((val8 >> 3) & 0x1) << HD44_E_S  |
				((val8 >> 4) & 0xf) << HD44_D4D7_S); 
      UPDATE(LCD);
      REFRESH(LCD);
    }

  /* carte extension */
  if (msp430_digiIO_dev_read(PORT6,&val8))
    {
      /* leds are inverted */
      machine.device[BLED9].write (BLED9 ,1, 1-BIT(val8,1));
      UPDATE(BLED9);
      REFRESH(BLED9);

      machine.device[BLED10].write(BLED10,1,   BIT(val8,1));
      UPDATE(BLED10);
      REFRESH(BLED10);
      
      /* 74HC259N + bargraph */
      if (((val8 & 0x80) == 0) && ((PORT6_LAST & 0x80) != 0)) /* ?? */
	{
	  int in;
	  int wval8=0;
	  BAR_LATCH[((val8 >> 1) & 0x07)]  = (val8 & 0x01);
	  for (in=0; in<8; in++)
	    {
	      wval8 |= (BAR_LATCH[in] << in);
	    }
	  machine.device[BAGR].write(BAGR,1,wval8);
	  UPDATE(BAGR);
	  REFRESH(BAGR);  
	}
      
      PORT6_LAST = val8;
    }
  


  switch (MCU.usart0.mode)
    {
    case USART_MODE_SPI:
      break;
    case USART_MODE_UART:
      if (msp430_usart0_dev_read_uart(&val8))
	{
	  machine.device[SERIAL].write(SERIAL, PTTY_D, val8);
	}
      break;
    default:
      break;
    }


  /* ************************* */
  /* devices -> MCU            */
  /* ************************* */

  /* input on usart0 line */
  if (msp430_usart0_dev_write_uart_ok())
    {
      uint32_t mask,value;
      machine.device[SERIAL].read(SERIAL,&mask,&value);
      if ((mask & PTTY_D) != 0)
	{
	  msp430_usart0_dev_write_uart(value & PTTY_D);
	}
    }

  /* input on LCD */
  {
    uint32_t mask, value;
    machine.device[LCD].read(LCD,&mask,&value);
    if ((mask & HD44_D4D7) != 0)
      {
	/* port 2 : D4D7 - E,RS,0,0 */
	/* LCD-D7 can be used as a busy flag */
	uint8_t val = (value & HD44_D4D7) >> HD44_D4D7_S;
	msp430_digiIO_dev_write(PORT2,(val & 0x0f) << 4,0xf0);
      }
  }


  /* input on buttons */
  {
#define UI_EVENT_SKIP 1
    if ((UI_LOOP_COUNT --) == 0)
      {
	int ev;
	UI_LOOP_COUNT = UI_EVENT_SKIP;
	switch ((ev = ui_getevent()))
	  {
	  case UI_EVENT_USER:
	    {
	      uint8_t b = 0xf0;
	      // the reset button is negated
	      //  if (machine.ui.val & UI_BUTTON_1)
	      //  msp430_reset_pin((machine.ui.b_down & UI_BUTTON_1) ? 0 : 1);
	      
	      // P0.012 buttons 1 2 and 3 -> p6 3 4 5
	      if ((machine.ui.b_down & UI_BUTTON_1) != 0)
		{
		  b &= ~0x10;
		  VERBOSE(3,"ot2007: button 1 pressed\n");
		}
	      if ((machine.ui.b_down & UI_BUTTON_2) != 0)
		{
		  b &= ~0x20;
		  VERBOSE(3,"ot2007: button 2 pressed\n");
		}
	      if ((machine.ui.b_down & UI_BUTTON_3) != 0)
		{
		  b &= ~0x40;
		  VERBOSE(3,"ot2007: button 3 pressed\n");
		}
	      if ((machine.ui.b_down & UI_BUTTON_4) != 0)
		{
		  b &= ~0x80;
		  VERBOSE(3,"ot2007: button 4 pressed\n");
		}

	      if (b != BUTTONS_LAST)
		{
		  int i;
		  /* VERBOSE(3,"ot2007-2: b 0x%02x last 0x%02x\n",b,BUTTONS_LAST); */
		  for(i=0; i<4; i++)
		    {
		      if (((b            & (0x10<<i)) == 0) && 
			  ((BUTTONS_LAST & (0x10<<i)) != 0))
			{
			  VERBOSE(3,"ot2007-2: button %d pressed\n",i+1);
			}

		      if (((b            & (0x10<<i)) != 0) && 
			  ((BUTTONS_LAST & (0x10<<i)) == 0))
			{
			  VERBOSE(3,"ot2007-2: button %d released\n",i+1);
			}
		    }
		}

	      /* P4.4-7                                */
	      /* 4 buttons mask                        */
	      VERBOSE(4,"ot2007: port4 write 0x%02x\n",b);
	      msp430_digiIO_dev_write(PORT4, b, 0xf0);
	      /* p1.7 // Dallas, high to low interrupt */
	      /* Logical or binded to p1.7 for IRQ     */
	      /* p1.7 high to low on button pressed    */
	      msp430_digiIO_dev_write(PORT1, b ? 0x80 : 0x00, 0x80);

	      BUTTONS_LAST = b;
	    }
	    break;
	  case UI_EVENT_QUIT:
	    HW_DMSG_UI("ot2007: UI event QUIT\n");
	    MCU_SIGNAL = SIG_UI;
	    break;
	  case UI_EVENT_NONE:
	    break;
	  default:
	    ERROR("ot2007: unknown ui event\n");
	    break;
	  }
      }
  }

  /* ************************* */
  /* update                    */
  /* ************************* */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  UPDATE(SERIAL);

  if (refresh)
    {
      ui_refresh();
    }
  
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
