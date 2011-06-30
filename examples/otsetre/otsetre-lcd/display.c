/******************************************************************
 *****                                                        *****
 *****  Name: display.c                                       *****
 *****  modified version of Andreas Dannenberg                *****
 *****  Includes only display specific code.		      *****
 ******************************************************************/


#include <msp430x14x.h>
#include "display.h"

#define bitset(var,bitno) ((var) |= 1 << (bitno))
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno)))

#define          B1                 BIT4&P4IN         //B1 - P4.4
#define          B2                 BIT5&P4IN         //B2 - P4.5
#define          B3                 BIT6&P4IN         //B3 - P4.6
#define          B4                 BIT7&P4IN         //B4 - P4.7
#define          FREQ               BIT0&P1IN         //FREQuency input - P1.0
#define          DI1                BIT1&P1IN         //Digital Input 1 - P1.1
#define          DI2                BIT2&P1IN         //Digital Input 2 - P1.2
#define          DI3                BIT3&P1IN         //Digital Input 3 - P1.3
#define          DI4                BIT4&P1IN         //Digital Input 4 - P1.4
#define          DALLAS             BIT7&P1IN         //DALLAS input - P1.7
#define          P20                BIT0&P2IN         //P20 input
#define          SDA                BIT0&P4IN         //SDA
#define          SCL                BIT1&P4IN         //SCL
#define          STATUS_LED_ON      P2OUT &= ~BIT1    //STATUS_LED - P2.1
#define          STATUS_LED_OFF     P2OUT |= BIT1     //STATUS_LED - P2.1
#define          RELAY1_ON          P1OUT |= BIT5     //RELAY1 - P1.5
#define          RELAY1_OFF         P1OUT &= ~BIT5    //RELAY1 - P1.5
#define          RELAY2_ON          P1OUT |= BIT6     //RELAY1 - P1.6
#define          RELAY2_OFF         P1OUT &= ~BIT6    //RELAY1 - P1.6
#define          BUZ1_ON            P4OUT |= BIT2     //P4.2
#define          BUZ1_OFF           P4OUT &= ~BIT2    //P4.2
#define          BUZ2_ON            P4OUT |= BIT3     //P4.3
#define          BUZ2_OFF           P4OUT &= ~BIT3    //P4.3
#define          LCD_Data           P2OUT
#define          _100us             100                //
#define          _10us              10                 //
#define          E                  3                 //P2.3
#define          RS                 2                 //P2.2
#define          CR                 0x0d
#define          LF                 0x0a
#define          BUTTON_TIME        100

#define		DISP_ON			0x0c	        //LCD control constants
#define		DISP_OFF		0x08	        //
#define		CLR_DISP		0x01    	//
#define		CUR_HOME		0x02	        //
#define		ENTRY_INC		0x06            //
#define		DD_RAM_ADDR		0x80	        //
#define		DD_RAM_ADDR2		0xc0	        //
#define		DD_RAM_ADDR3		0x28	        //
#define		CG_RAM_ADDR		0x40	        //



void Delay (unsigned int a);
void Delayx100us(unsigned char b);
void SEND_CHAR (unsigned char c);
void SEND_CMD (unsigned char e);
void _E(void);
void InitLCD(void);
void InitOsc(void);
void InitPorts(void);

int variableGlobalePourBSS;

void initDisplay() {
  InitOsc();
  InitPorts();
  InitLCD();
  clearDisplay();
}
void putc(char c) {
    SEND_CHAR(c);
}
void clearDisplay() {
    SEND_CMD(CLR_DISP);
    Delayx100us(10);
}
void gotoSecondLine() {
//    SEND_CMD(CLR_DISP);
    SEND_CMD(DD_RAM_ADDR2);
}
void printString(char *String) {
  while(*String)
    putc(*String++);
}
char HexDigit(int digitvalue) {
  if (digitvalue < 10)
    return(digitvalue + '0');
  else
    return(digitvalue + 'A' - 10);
}
void printByte(unsigned int theByte) {
  char HexBuffer[3];
  HexBuffer[2] = 0;
  HexBuffer[1] = HexDigit(theByte & 0x000f);
  theByte = theByte >> 4;
  HexBuffer[0] = HexDigit(theByte & 0x000f);
  printString(HexBuffer);
}
void printHex(unsigned int Number) {
  char HexBuffer[5];
  HexBuffer[4] = 0;
  HexBuffer[3] = HexDigit(Number & 0x000f);
  Number = Number >> 4;
  HexBuffer[2] = HexDigit(Number & 0x000f);
  Number = Number >> 4;
  HexBuffer[1] = HexDigit(Number & 0x000f);
  Number = Number >> 4;
  HexBuffer[0] = HexDigit(Number & 0x000f);
  printString(HexBuffer);
}
void printDecimal(int Number) {
  // need to move to long int to account for
  // negative 32768
  char DecimalBuffer[7];
  long lNumber = Number;
  DecimalBuffer[6] = 0;
  if (lNumber < 0) {
    DecimalBuffer[0] = '-';
    lNumber = -lNumber;
  } else
    DecimalBuffer[0] = '+';
  DecimalBuffer[5] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[4] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[3] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[2] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[1] = (lNumber % 10)+'0';
  printString(DecimalBuffer);
}


void InitOsc(void)
{
  WDTCTL = WDTPW | WDTHOLD;                      // stop watchdog timer

  BCSCTL1 |= XTS;                                // XT1 as high-frequency
  _BIC_SR(OSCOFF);                               // turn on XT1 oscillator

  do                                             // wait in loop until crystal is stable
    IFG1 &= ~OFIFG;
  while (IFG1 & OFIFG);

  BCSCTL1 |= DIVA0;                              // ACLK = XT1 / 2
  BCSCTL1 &= ~DIVA1;

  IE1 &= ~WDTIE;                                 // disable WDT int.
  IFG1 &= ~WDTIFG;                               // clear WDT int. flag

  WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | WDTSSEL | WDTIS1; // use WDT as timer, flag each
                                                           // 512 pulses from ACLK

  while (!(IFG1 & WDTIFG));                      // count 1024 pulses from XT1 (until XT1's
                                                 // amplitude is OK)

  IFG1 &= ~OFIFG;                                // clear osc. fault int. flag
  BCSCTL2 |= SELM0 | SELM1;                      // set XT1 as MCLK
}

void InitPorts(void)
{
  P1SEL = 0;                                     //
  P1OUT = 0;                                     //
  P1DIR = BIT5 | BIT6;                           //enable only Relay outputs

  P2SEL = 0;
  P2OUT = 0;
  P2DIR = ~BIT0;                                //only P2.0 is input

  P3SEL |= BIT4 | BIT5;                         //enable UART0
  P3DIR |= BIT4;                                //enable TXD0 as output
  P3DIR &= ~BIT5;                               //enable RXD0 as input

  P4SEL = 0;
  P4OUT = 0;
  P4DIR = BIT2 | BIT3;                          //only buzzer pins are outputs

  //P6SEL = 0x80;
  //P6OUT = 0;
 // P6DIR = 0x00;                                  // all output
}

void Delay (unsigned int a)
{
  int k;
  for (k=0 ; k != a; ++k) {
    _NOP();
    _NOP();
    _NOP();
    _NOP();
  }
}

void Delayx100us(unsigned char b)
{
  int j;
  for (j=0; j!=b; ++j) Delay (_100us);
}


void _E(void)
{
        bitset(P2OUT,E);		//toggle E for LCD
	Delay(_10us);
	bitclr(P2OUT,E);
}


void SEND_CHAR (unsigned char d)
{
        int temp;
	Delayx100us(5);                 //.5ms	
	temp = d & 0xf0;		//get upper nibble	
	LCD_Data &= 0x0f;
	LCD_Data |= temp;
	bitset(P2OUT,RS);     	        //set LCD to data mode
	_E();                           //toggle E for LCD
	temp = d & 0x0f;
	temp = temp << 4;               //get down nibble
	LCD_Data &= 0x0f;
	LCD_Data |= temp;
	bitset(P2OUT,RS);   	        //set LCD to data mode
	_E();                           //toggle E for LCD
}

void SEND_CMD (unsigned char e)
{
        int temp;
	Delayx100us(10);                //10ms
	temp = e & 0xf0;		//get upper nibble	
	LCD_Data &= 0x0f;
	LCD_Data |= temp;               //send CMD to LCD
	bitclr(P2OUT,RS);     	        //set LCD to CMD mode
	_E();                           //toggle E for LCD
	temp = e & 0x0f;
	temp = temp << 4;               //get down nibble
	LCD_Data &= 0x0f;
	LCD_Data |= temp;
	bitclr(P2OUT,RS);   	        //set LCD to CMD mode
	_E();                           //toggle E for LCD
}

void InitLCD(void)
{
    bitclr(P2OUT,RS);
    Delayx100us(250);                   //Delay 100ms
    Delayx100us(250);
    Delayx100us(250);
    Delayx100us(250);
    LCD_Data |= BIT4 | BIT5;            //D7-D4 = 0011
    LCD_Data &= ~BIT6 & ~BIT7;
    _E();                               //toggle E for LCD
    Delayx100us(100);                   //10ms
    _E();                               //toggle E for LCD
    Delayx100us(100);                   //10ms
    _E();                               //toggle E for LCD
    Delayx100us(100);                   //10ms
    LCD_Data &= ~BIT4;
    _E();                               //toggle E for LCD

    SEND_CMD(DISP_ON);
    SEND_CMD(CLR_DISP);
    Delayx100us(250);
    Delayx100us(250);
    Delayx100us(250);
    Delayx100us(250);
}


int main( void )
{
  unsigned int i;

  InitPorts();
  InitLCD(); 
  clearDisplay();
  printString("Test LCD");
  Delay(10000);
  return 0;
}

