#ifndef HARDWARE_SETRE_H
#define HARDWARE_SETRE_H

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

#define     OUTPUT      0XFF        //
#define     INPUT       0X00        //

const char      bitnumbers[] = { L0,L1,L2,L3,L4,L5,L6,L7,L8,L9,PP };
int             LCD_DATA_V=0;
int             LCD_CTRL_V=0;

#define LED_OUT  P4OUT
#define SEG_OUT  P3OUT

#define         E_HIGH          P6OUT &= ~BIT5 // due to inverter / buffer
#define         E_LOW           P6OUT |= BIT5  // level of bits needs to be
#define         RS_HIGH         P6OUT &= ~BIT6 // complemented with respect to LCD
#define         RS_LOW          P6OUT |= BIT6  //
#define         RW_HIGH         P6OUT &= ~BIT7 // Bits 0-2 are reserved for ADC
#define         RW_LOW          P6OUT |= BIT7  // and Bits 3-4 are 7SEG display Enable
#define         LCD_Data        P5OUT
#define     LCD_READ    RW_HIGH
#define     LCD_WRITE   RW_LOW
// #define     LCD_CLEAR

#define     LCD_PORT_DATA P5OUT //      Port 5 data LCD
#define     LCD_PORT_CTRL P6OUT //      Port 6 ctrl LCD
#define     LCD_E  0x20                 //      E : Enable                              bit 5 on port 6
#define     LCD_RS 0x40                 //      RS : Register select signal     bit 6 on port 6
#define     LCD_RW 0x80                 //      Read/Write signal               bit 7 on port 6
#define   B0                      BIT0&P1IN         //B1 - P1.0
#define   B1              BIT1&P1IN         //B2 - P1.1
#define   B2              BIT2&P1IN         //B3 - P1.2
#define   B3              BIT3&P1IN         //B4 - P1.4

/*
 */
#endif //HARDWARE_SETRE_H
