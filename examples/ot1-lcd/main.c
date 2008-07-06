
#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>
#include "uart0.h"

#define LED_OUT  P4OUT
#define SEG_OUT  P3OUT


int led_state;
int msg_state;

#if 0
#define WAIT_CRISTAL()                                                 \
do {                                                                   \
  int i;                                                               \
  do {                                                                 \
    IFG1 &= ~OFIFG;                  /* Clear OSCFault flag  */        \
    for (i = 0xff; i > 0; i--)       /* Time for flag to set */        \
       nop();                        /*                      */        \
  }  while ((IFG1 & OFIFG) != 0);    /* OSCFault flag still set? */    \
} while (0)


void set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz()
{
  DCOCTL  = 0;
  BCSCTL1 = 0;
  BCSCTL2 = SELM_2 | (SELS | DIVS_3) ;

  WAIT_CRISTAL();
}
#endif

/* ************************************************** */
/* * Uart0 print ************************************ */
/* ************************************************** */

int putchar(int c)
{
  return uart0_putchar(c);
}

/* ************************************************** */
/* * Delay ****************************************** */
/* ************************************************** */

void delay(unsigned int d) 
{
  int i,j;
  for(j=0; j < 0xff; j++)
    {
      for (i = 0; i<d; i++) 
	   {
	     nop();
	     nop();
	   }
    }
}

/* ************************************************** */
/* * 7 segments ************************************* */
/* ************************************************** */

#define TOP  0x01
#define TR   0x02
#define BR   0x04
#define BOT  0x08
#define BL   0x10
#define TL   0x20
#define MID  0x40
#define DP   0x80

#define L0 ( TOP | TR | BR | BOT | BL | TL       )
#define L1 (       TR | BR                       )
#define L2 ( TOP | TR | MID | BL | BOT           )
#define L3 ( TOP | TR | MID | BR | BOT           )
#define L4 ( TL  | TR | MID | BR                 )
#define L5 ( TOP | TL | MID | BR | BOT           )
#define L6 ( TOP | TL | MID | BR | BL | BOT      )
#define L7 ( TOP | TR | BR                       )
#define L8 ( TOP | TL | TR | MID | BL | BR | BOT )
#define L9 ( TOP | TL | TR | MID | BR | BOT      )
#define PP ( DP                                  )

#define La ( TOP | TL | TR | MID | BL | BR )
#define Lb (       TL |      MID | BL | BR | BOT )
#define Lc (                 MID | BL |      BOT )
#define Ld ( TR | MID | BL | BR | BOT )
#define Le ( TOP | TL | MID | BL | BOT )
#define Lf ( TOP | TL | BL | MID )

const char bitnumbers[] = { L0,L1,L2,L3,L4,L5,L6,L7,L8,L9,PP };
const char hexnumbers[] = { L0,L1,L2,L3,L4,L5,L6,L7,L8,L9,La,Lb,Lc,Ld,Le,Lf,PP };

void seg_output_char(int i, char s)
{
  SEG_OUT = bitnumbers[i];
  switch (s) {
    case 'l': // left
      P6OUT |=  0x10; // 6.4 = 1
      P6OUT &= ~0x10; // 6.4 = 0
      break;
    case 'r':
      P6OUT |=  0x08; // 6.3 = 1
      P6OUT &= ~0x08; // 6.3 = 0
      break;
    case 'b':
      P6OUT |=  0x18; 
      P6OUT &= ~0x18; 
      break;
    }

}

void seg_output(int i)
{
  seg_output_char(i/10,'l');
  seg_output_char(i%10,'r');
}

/* ************************************************** */
/* * LCD ******************************************** */
/* ************************************************** */

#define LCD_DATA_DIR  P5DIR
#define LCD_DATA_OUT  P5OUT
#define LCD_DATA_IN   P5IN

#define LCD_PORT_CTRL P6OUT

#define LCD_RW 0x80u
#define LCD_RS 0x40u
#define LCD_E  0x20u

void lcd_wait_busyflag()
{
  volatile int busyflag = 1;
  LCD_PORT_CTRL &= ~(LCD_RW | LCD_RS | LCD_E);
  LCD_PORT_CTRL |=   LCD_RW;
  LCD_DATA_DIR = 0x0;
  while (busyflag != 0)
    {
      LCD_PORT_CTRL |= LCD_E;  // E = 1
      nop();
      busyflag = LCD_DATA_IN & 0x80;
      LCD_PORT_CTRL &= ~LCD_E; // E = 0
      nop();
    }
  LCD_DATA_DIR = 0xff;
}

    
#define WRITE_START                  \
  do {                               \
  LCD_PORT_CTRL &= ~LCD_RW ;         \
  LCD_PORT_CTRL |= (LCD_RS | LCD_E); \
  } while(0)
  
#define WRITE_END                    \
  do {                               \
  LCD_PORT_CTRL &=  ~LCD_E;          \
} while(0)


/****************/
/* LCD commands */
/****************/
void lcd_write_command(unsigned char c)
{
  LCD_PORT_CTRL &= ~(LCD_RW | LCD_RS | LCD_E) ;
  LCD_PORT_CTRL |= LCD_E;
  LCD_DATA_OUT   = c;
  LCD_PORT_CTRL &= ~LCD_E;
}

void lcd_clear()
{
  lcd_wait_busyflag();
  lcd_write_command(0x01);
}

void lcd_reset()
{
  lcd_wait_busyflag();
  lcd_clear();
  lcd_wait_busyflag();
  lcd_write_command(0x08 | 0x04);
}

void lcd_set_cursor_address(int l, int c)
{
  lcd_wait_busyflag();
  lcd_write_command(0x80 | ((l == 0) ? 0x0: 0x40) | (c & 0x3f));
}

void lcd_init()
{
  // wait for 30ms
  delay(60);

  // Function set   0x30
  // LCD_2LINES     0x08
  lcd_write_command(0x30 | 0x08);
  delay(4);

  // Display on/off 0x08
  // display on     0x04
  // cursor on      0x02
  // blink on       0x01
  lcd_write_command(0x08 | 0x04 | 0x02);
  delay(4);

  // Clear          0x1
  lcd_write_command(0x01);
  delay(10); 

  // Entry mode set 0x04
  // increment mode 0x02
  // entire shift   0x01
  lcd_write_command(0x04 | 0x02 | 0x01);
  delay(10); 

  // initialization end

  /*
  lcd_write_command(0x04); // display off
  lcd_wait_busyflag();
  lcd_write_command(0x01); // display clear
  lcd_wait_busyflag();
  lcd_write_command(0x07); // Entry mode set
  lcd_wait_busyflag();
  lcd_write_command(0x0c); // lcd on, cursor on, blink off
  lcd_wait_busyflag();
  */
}


/************/
/* LCD data */
/************/

void lcd_write_data(unsigned char d)
{
  lcd_wait_busyflag();
  LCD_PORT_CTRL &= ~(LCD_RW | LCD_RS | LCD_E) ;
  LCD_PORT_CTRL |= LCD_RS | LCD_E;
  LCD_DATA_OUT   = d;
  LCD_PORT_CTRL &= ~(LCD_RS | LCD_E);
}

void lcd_putchar(char s)
{
  lcd_wait_busyflag();
  lcd_write_data(s);
}

void lcd_print(int line, char* string)
{
  char* s;
  int i;
  lcd_wait_busyflag();
  for(s=string, i=0; *s; s++, i++)
    {
      lcd_set_cursor_address(line,i);
      lcd_putchar(*s);
    }
}

/* ************************************************** */
/* * Timer ****************************************** */
/* ************************************************** */

#define TA_ACLK_FREQ_HZ  32768
#define TA_TICK_RATE_HZ    100
#define TA_DIV            ID_3

void set_timer()
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = TA_ACLK_FREQ_HZ / TA_TICK_RATE_HZ;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;
  /* Freq divider. */
  TACTL |= TA_DIV;
  /* Start up clean. */
  //  TACTL |= TACLR;
  /* Up mode. */
  TACTL |= MC_1;
}

interrupt (TIMERA0_VECTOR) prvTickISR( void )
{
  led_state <<= 1;
  if (led_state > 0x80)
    led_state = 1;

  LED_OUT = led_state;
}

/* ************************************************** */
/* * User buttons *********************************** */
/* ************************************************** */

interrupt (PORT1_VECTOR) prvbutton( void )
{
  int val = P1IN;
  LED_OUT = val;
  P1IFG = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define DELAY 0x100

int main(void) {
    int i,c;
    //    char buff[50];
    //WDTCTL = WDTCTL_INIT;

    P1OUT  = 0;           // Init output data of port1 0:
    P2OUT  = 0;
    P3OUT  = 0;
    P4OUT  = 0;
    P5OUT  = 0;
    P6OUT  = 0;

    P1SEL  = 0;           // Select port or module -function on port1
    P2SEL  = 0xff;        // 0 : I/O function 
    P3SEL  = 0;           // 1 : peripheral module
    P4SEL  = 0;
    P5SEL  = 0;
    P6SEL  = 0;

    P1DIR  = 0xf0;        // Init port direction register of port1
    P2DIR  = 0x10;        // 0 : input direction
    P3DIR  = 0xff;        // 1 : output direction
    P4DIR  = 0xff;
    P5DIR  = 0xff;
    P6DIR  = 0xff;
    
    P1IES  = 0xff;        // Interrupt edge select
    P2IES  = 0x00;        // 0: low to high, 1 : high to low

    P1IE   = 0xff;        // Interrupt enable
    P2IE   = 0x00;        // 0:disable 1:enable

    lcd_init();
    set_timer();
    uart0_init();
    eint();

    led_state = 1;
    msg_state = 0;

    // 6.5 LCD Enable
    // 6.4 7seg1
    // 6.3 7seg2
    P6OUT &= ~(0x10 | 0x08 | LCD_E);

    lcd_putchar('O' & 0x0ffu);
    lcd_putchar('k' & 0x0ffu);

    printf("Ot1 platform test ready\n");

    while (1) 
      {                         
        for (i=0; i<99; i++)
	  {
	    seg_output(i);
	    if ((c = uart0_getchar()))
	      {
		printf("%c",c);
		lcd_putchar(c & 0x0ffu);
	      }
	  }
      }
}

