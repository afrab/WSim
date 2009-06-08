#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "m25p80.h"

/**************************************************/
/** MSP430 <> M25P80 PORT *************************/
/**************************************************/

#define M25P80_PORT_WPR  P1OUT /* 1.2 write enable    */
#define M25P80_PORT_HCS  P4OUT /* 4.7 Hold and 4.4 CS */
#define M25P80_PORT_SPI  P5OUT /* 5.1/2/3 SPI         */

/* on MSP430_PORT 1                 */
#define M25P80_PIN_PROTECT    2

/* on MSP430_PORT 4                 */
#define M25P80_PIN_CS         4
#define M25P80_PIN_HOLD       7

/* on MSP430_PORT 5                 */
#define M25P80_PIN_Clk_uclk   3 /* uclk */
#define M25P80_PIN_Din_somi   2 /* somi */
#define M25P80_PIN_Dout_simo  1 /* simo */

#define BITMASK(n)      (1 << n)

#define M25P80_BIT_CS      BITMASK(M25P80_PIN_CS)
#define M25P80_BIT_HOLD    BITMASK(M25P80_PIN_HOLD)
#define M25P80_BIT_PROTECT BITMASK(M25P80_PIN_PROTECT)
#define M25P80_BIT_Clk     BITMASK(M25P80_PIN_Clk_uclk)
#define M25P80_BIT_Din     BITMASK(M25P80_PIN_Din_somi)
#define M25P80_BIT_Dout    BITMASK(M25P80_PIN_Dout_simo)


/* chip select is negated : active low */
/* Bit : 0 = chip select               */
/*       1 = chip disable              */
#define M25P80_CS_ENABLE()     do { M25P80_PORT_HCS &= ~M25P80_BIT_CS; } while(0)
#define M25P80_CS_DISABLE()    do { M25P80_PORT_HCS |=  M25P80_BIT_CS; } while(0)

/* hold is negated : active low */
/* Bit : 0 = hold               */
/*       1 = open               */
#define M25P80_HOLD_ENABLE()   do { M25P80_PORT_HCS &= ~M25P80_BIT_HOLD; } while(0)
#define M25P80_HOLD_DISABLE()  do { M25P80_PORT_HCS |=  M25P80_BIT_HOLD; } while(0)

/* write protect negated : active low */
/* Bit : 0 = Write protect on         */
/*       1 = Write protect off        */
#define M25P80_PROTECT_ENABLE()  do { M25P80_PORT_WPR &= ~M25P80_BIT_PROTECT; } while(0)
#define M25P80_PROTECT_DISABLE() do { M25P80_PORT_WPR |=  M25P80_BIT_PROTECT; } while(0)

/**************************************************/
/** M25P80 <> SPI port 1 **************************/
/**************************************************/

#define SPI1_TX_READY    (IFG2 & UTXIFG1)
#define SPI1_RX_READ     (IFG2 & URXIFG1)

#define SPI1_TX_BUF      TXBUF1
#define SPI1_RX_BUF      RXBUF1

#define DUMMY 0x00

unsigned char spi_write_then_read(const unsigned char data);
unsigned char spi_write_then_read(const unsigned char data)
{
  while (SPI1_TX_READY == 0);
  SPI1_TX_BUF = data;                
  while (SPI1_RX_READ == 0);  // wait for RX buffer full
  return SPI1_RX_BUF;
}

/**************************************************/
/** M25P80 Instructions ****************************/
/**************************************************/

#define OPCODE_WREN      0x06u /* write enable         */
#define OPCODE_WRDI      0x04u /* */
#define OPCODE_RDSR      0x05u /* read status register */
#define OPCODE_WRSR      0x01u /* */
#define OPCODE_READ      0x03u
#define OPCODE_FAST_READ 0x0Bu
#define OPCODE_PP        0x02u
#define OPCODE_SE        0xd8u
#define OPCODE_BE        0xc7u
#define OPCODE_DP        0xb9u
#define OPCODE_RES       0xabu

enum m25_opcode_t {
  M25_OP_NOP       = 0x00u,  /* nop                   */
  M25_OP_WREN      = 0x06u,  /* write enable          */
  M25_OP_WRDI      = 0x04u,  /* write disable         */
  M25_OP_RDSR      = 0x05u,  /* read status register  */
  M25_OP_WRSR      = 0x01u,  /* write status register */   
  M25_OP_READ      = 0x03u,  /* read                  */
  M25_OP_FAST_READ = 0x0Bu,  /* fast read             */
  M25_OP_PP        = 0x02u,  /* page program          */
  M25_OP_SE        = 0xd8u,  /* sector erase          */
  M25_OP_BE        = 0xc7u,  /* bulk erase            */
  M25_OP_DP        = 0xb9u,  /* deep power down       */
  M25_OP_RES       = 0xabu   /* read electronic signature / release from deep power mode */
};

#define M25P80_ELECTRONIC_SIGNATURE 0x13

/* msp430 is little endian */
struct __attribute__ ((packed)) m25p80_sr {
  uint8_t
    wip:1,     /* b0 */
    wel:1,
    bp0:1,
    bp1:1,
    bp2:1,
    unused1:1,
    unused2:1, 
    srwd:1;    /* b7 */
};
typedef struct m25p80_sr m25p80_sr_t;
 
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void m25p80_active()
{
  /*****************************************/
  /* Select port or module-function        */
  /* 0 : I/O function                      */
  /* 1 : peripheral module                 */
  /* Init port direction register of port  */
  /* 0 : input direction                   */
  /* 1 : output direction                  */
  /*****************************************/

  // Configure as outputs(SIMO,SCK)
  // Port 5 Function           Dir       On/Off
  //         5.0-mmcCD         Out       0 - card inserted
  //         5.1-Dout          Out       0 - off    1 - On -> init in SPI_Init
  //         5.2-Din           Inp       0 - off    1 - On -> init in SPI_Init
  //         5.3-Clk           Out       -                 -> init in SPI_Init
  //         5.4-mmcCS         Out       0 - Active 1 - none Active

  P5SEL |= 0x0E;
  P5SEL &= ~0x11;
  P5OUT |= 0x10;
  P5DIR |= 0x1A;

  // dir is set to 0, although it is not used when SEL=1
  P5DIR  |=   M25P80_BIT_Clk | M25P80_BIT_Dout;
  P5DIR  &=  ~M25P80_BIT_Din;
  P5SEL  |=   M25P80_BIT_Din | M25P80_BIT_Dout | M25P80_BIT_Clk; 

  // Configure as GPIO outputs(CSn,Hold)
  P4DIR  |=  (M25P80_BIT_CS | M25P80_BIT_HOLD); 
  P4SEL  &= ~(M25P80_BIT_CS | M25P80_BIT_HOLD);

  // Configure as GPIO outputs(write)
  P1DIR  |=  (M25P80_BIT_PROTECT);
  P1SEL  &= ~(M25P80_BIT_PROTECT);

  /*****************************************/
  /* SSEL_0              0x00        UCLKI */
  /* SSEL_1              0x10        ACLK  */
  /* SSEL_2              0x20        SMCLK */
  /* SSEL_3              0x30        SMCLK */
  /* CKPH    1/2 cycle delay               */
  /* CKPL    inactive level = high         */
  /* SSEL1   bit 1                         */
  /* SSEL0   bit 0                         */
  /* STC     3-pin SPI mode (STE disabled) */
  /*****************************************/
  UCTL1   = SWRST;                       // 8-bit SPI Master **SWRST**
  UTCTL1  = CKPH | SSEL1 | SSEL0 | STC;  // SMCLK, 3-pin mode, clock idle low, data valid on rising edge, UCLK delayed
  UBR01   = 0x02;                        // 0x02: UCLK/2 (4 MHz), works also with 3 and 4
  UBR11   = 0x00;                        // -"-
  UMCTL1  = 0x00;                        // no modulation
  UCTL1   = CHAR | SYNC | MM | SWRST;    // 8-bit SPI Master **SWRST**
  UCTL1  &= ~SWRST;                      // clear SWRST
  IE2    &= ~(UTXIE1 | URXIE1);          // disable interrupts : added by Antoine
  ME2    |= USPIE1;                      // Enable USART1 SPI mode
  while (!(IFG2 & UTXIFG1));             // USART1 TX buffer ready (empty)?
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

unsigned char m25p80_init()
{
  m25p80_active();
  M25P80_HOLD_DISABLE();
  M25P80_PROTECT_DISABLE();
  return m25p80_get_signature();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

unsigned char m25p80_get_signature()
{
  unsigned char ret = 0;
  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_RES);
  spi_write_then_read(DUMMY);
  spi_write_then_read(DUMMY);
  spi_write_then_read(DUMMY);
  ret = spi_write_then_read(DUMMY);
  M25P80_CS_DISABLE();
  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* M25p80 is programmed using Hold, Write and Clock 
 * other information is data or command 
 */

unsigned char m25p80_get_state()
{ 
  unsigned char ret = 0;
  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_RDSR);
  ret = spi_write_then_read(DUMMY);
  M25P80_CS_DISABLE();
  return ret; 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int m25p80_wakeup(void)
{
  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_RES);
  M25P80_CS_DISABLE();
  return 0; 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int m25p80_power_down(void)
{ 
  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_DP);
  M25P80_CS_DISABLE();
  return 0; 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static inline void m25p80_block_wip()
{
#define WIP 0x01

  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_RDSR);
  while(spi_write_then_read(DUMMY) & WIP)
    ;
  M25P80_CS_DISABLE();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void m25p80_write_enable()
{
  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_WREN);
  M25P80_CS_DISABLE();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int 
m25p80_erase_sector(uint8_t index)
{
  m25p80_block_wip();
  m25p80_write_enable();
  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_SE);
  spi_write_then_read(index & 0xff);
  spi_write_then_read(0     & 0xff);
  spi_write_then_read(0     & 0xff);
  M25P80_CS_DISABLE();
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int 
m25p80_erase_bulk()
{
  m25p80_block_wip();
  m25p80_write_enable();
  M25P80_CS_ENABLE();
  spi_write_then_read(OPCODE_WREN);
  spi_write_then_read(OPCODE_BE);
  M25P80_CS_DISABLE();
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int
m25p80_save_page(uint16_t index, uint8_t *buffer) 
{
  uint16_t i = 0;

  m25p80_block_wip();
  m25p80_write_enable();
  M25P80_CS_ENABLE();

  spi_write_then_read(OPCODE_PP);
  spi_write_then_read((index >> 8) & 0xff);
  spi_write_then_read((index >> 0) & 0xff);
  spi_write_then_read((         0) & 0xff);

  for(i = 0; i < M25P80_PAGE_SIZE; i++)
    {
      spi_write_then_read(buffer[i]);
    }

  M25P80_CS_DISABLE();
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int
m25p80_load_page(uint16_t index, uint8_t *buffer) 
{
  uint16_t i = 0;
  
  m25p80_block_wip();
  M25P80_CS_ENABLE();

  spi_write_then_read(OPCODE_READ);
  spi_write_then_read((index >> 8) & 0xff);
  spi_write_then_read((index >> 0) & 0xff);
  spi_write_then_read((         0) & 0xff);

  for(i = 0; i < M25P80_PAGE_SIZE; i++)
    {
      buffer[i] = spi_write_then_read(DUMMY);
    }

  M25P80_CS_DISABLE();
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
