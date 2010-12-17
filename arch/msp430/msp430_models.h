
/**
 *  \file   msp430_models.h
 *  \brief  MSP430 MCU definitions 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/


#ifndef MSP430_MODELS_H
#define MSP430_MODELS_H


/**
   MSP430f135
   MSP430f149 
   MSP430f449
   MSP430f1611   
   MSP430f2013   
*/

/**
  
  MSP430 Memory organization

  [slau056e.pdf page 17]

  0x0ffff
    interrupt vector table
  0x0ffe0
    Flash
  --
    [empty]
  --
    RAM
  0x00200
  0x001ff
    16 bits peripherals modules
  0x00100
  0x000ff
    8 bits peripherals modules 
  0x00010
  0x0000f
    special functions registers
  0x00000

**/
#define MCU_NAME         "msp430" /* used in libelf_ntv */
#define MCU_EM_ARCH_ID   105      /* elf EM_ == 0x69    */
#define MCU_BFD_ARCH_ID  62       /* bfd                */

#if defined(WSIM_USES_GNU_BFD)
#define MCU_ARCH_ID MCU_BFD_ARCH_ID
#else
#define MCU_ARCH_ID MCU_EM_ARCH_ID
#endif

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
#if defined(MSP430f135)
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */

#define MCU_BFD_MACH_ID   13 /* bfd_mach_msp13 */
#define MCU_VERSION       "f135"
#define MCU_MODEL_NAME    "msp430f135"

#define ADDR_FLASH_STOP   0xFFFFu
#define ADDR_FLASH_START  0xC000u
#define ADDR_NVM_STOP     0x10FFu
#define ADDR_NVM_START    0x1000u
#define ADDR_BOOT_STOP    0x0fffu
#define ADDR_BOOT_START   0x0c00u
#define ADDR_RAM_STOP     0x03FFu
#define ADDR_RAM_START    0x0200u

#define INTR_RESET        15
#define INTR_NMI          14
#define INTR_TIMERB3_0    13
#define INTR_TIMERB3_1    12
#define INTR_COMP_A       11
#define INTR_WATCHDOG     10
#define INTR_USART0_RX     9
#define INTR_USART0_TX     8
#define INTR_ADC12         7
#define INTR_TIMERA3_0     6 // CCR0 CCIFG
#define INTR_TIMERA3_1     5 // CCR1, CCR2, TAIFG
#define INTR_IOPORT1       4
#define INTR_UNUSED_1      3
#define INTR_UNUSED_2      2
#define INTR_IOPORT2       1
#define INTR_UNUSED_3      0

// system clock
#define __msp430_have_basic_clock
#define __msp430_have_xt2 

// 8 bit blocks
#define __msp430_have_port1
#define __msp430_have_port2
#define __msp430_have_port3
#define __msp430_have_port4
#define __msp430_have_port5
#define __msp430_have_port6
#define __msp430_have_usart0
#define __msp430_have_cmpa

// 16 bit blocks
#define __msp430_have_timera3
#define __msp430_have_timerb3
#define __msp430_have_watchdog
#define __msp430_have_adc12
#define __msp430_have_flash

// Flash erase timings
#define FLASH_WRITE_TIMING_BYTE    35
#define FLASH_WRITE_TIMING_FSTBYTE 30
#define FLASH_WRITE_TIMING_NXTBYTE 21
#define FLASH_WRITE_TIMING_LSTBYTE  6
#define FLASH_ERASE_TIMING_MASS  5297
#define FLASH_ERASE_TIMING_SEG   4819

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
#elif defined(MSP430f149)
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/**
   MSP430F149x

   149 memory 60KB
   ===
        Flash  0xffe0 - 0xffff : main : interrupt vector
               0x1100 - 0xffff : main : code memory     

        Flash  0x1000 - 0x10ff : 256 bytes information Flash
        ROM    0x0c00 - 0x0fff : 1KB Boot ROM

              [0x0a00 - 0x0bff]  empty 512 bytes

        RAM    0x0200 - 0x09ff : 2KB RAM
               0x0100 - 0x01ff : 16-bit peripherals
               0x0010 - 0x00ff :  8-bit peripherals
               0x0000 - 0x000f :  8 bit SFR
   

**/

#define MCU_BFD_MACH_ID   14 /* bfd_mach_msp14 */
#define MCU_VERSION       "f149"
#define MCU_MODEL_NAME    "msp430f149"

#define ADDR_FLASH_STOP   0xFFFFu
#define ADDR_FLASH_START  0x1100u
#define ADDR_NVM_STOP     0x10FFu
#define ADDR_NVM_START    0x1000u
#define ADDR_BOOT_STOP    0x0fffu
#define ADDR_BOOT_START   0x0c00u
#define ADDR_RAM_STOP     0x09FFu
#define ADDR_RAM_START    0x0200u

#define INTR_RESET        15
#define INTR_NMI          14
#define INTR_TIMERB7_0    13
#define INTR_TIMERB7_1    12
#define INTR_COMP_A       11
#define INTR_WATCHDOG     10
#define INTR_USART0_RX     9
#define INTR_USART0_TX     8
#define INTR_ADC12         7
#define INTR_TIMERA3_0     6 // CCR0 CCIFG
#define INTR_TIMERA3_1     5 // CCR1, CCR2, TAIFG
#define INTR_IOPORT1       4
#define INTR_USART1_RX     3
#define INTR_USART1_TX     2
#define INTR_IOPORT2       1
#define INTR_UNUSED        0 // do not define INTR_DAC12

//
#define __msp430_have_basic_clock
#define __msp430_have_xt2 

// hwmul
#define __msp430_have_hwmul

// 8 bit blocks
#define __msp430_have_port1
#define __msp430_have_port2
#define __msp430_have_port3
#define __msp430_have_port4
#define __msp430_have_port5
#define __msp430_have_port6
#define __msp430_have_usart0
#define __msp430_have_usart1
#define __msp430_have_cmpa

// 16 bit blocks
#define __msp430_have_adc12
#define __msp430_have_timera3
#define __msp430_have_timerb7
#define __msp430_have_watchdog
#define __msp430_have_flash

// Flash erase timings
#define FLASH_WRITE_TIMING_BYTE    35
#define FLASH_WRITE_TIMING_FSTBYTE 30
#define FLASH_WRITE_TIMING_NXTBYTE 21
#define FLASH_WRITE_TIMING_LSTBYTE  6
#define FLASH_ERASE_TIMING_MASS  5297
#define FLASH_ERASE_TIMING_SEG   4819

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
#elif defined(MSP430f449)
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */

#define MCU_BFD_MACH_ID   44  /* bfd_mach_msp44 */
/* bug using binutils-2.16, f449 is referenced as MACH_ID 43 */
/* #define MCU_BFD_MACH_ID   43 */ 
#define MCU_VERSION       "f449"
#define MCU_MODEL_NAME    "msp430f449"

#define ADDR_FLASH_STOP   0xFFFFu
#define ADDR_FLASH_START  0x1100u
#define ADDR_NVM_STOP     0x10FFu
#define ADDR_NVM_START    0x1000u
#define ADDR_BOOT_STOP    0x0fffu
#define ADDR_BOOT_START   0x0c00u
#define ADDR_RAM_STOP     0x09FFu
#define ADDR_RAM_START    0x0200u

#define INTR_RESET         15
#define INTR_NMI           14
#define INTR_TIMERB7_0     13
#define INTR_TIMERB7_1     12
#define INTR_COMP_A        11
#define INTR_WATCHDOG      10
#define INTR_USART0_RX      9
#define INTR_USART0_TX      8
#define INTR_ADC12          7
#define INTR_TIMERA3_0      6
#define INTR_TIMERA3_1      5
#define INTR_IOPORT1        4
#define INTR_USART1_RX      3
#define INTR_USART1_TX      2
#define INTR_IOPORT2        1
#define INTR_BT             0

/**
 * Serial ports 
 *
 * P3.3 UCLK0
 * P3.2 SOMI0
 * P3.1 SIMO0
 * P3.0 STE0
 *
 * P4.3 UCLK1
 * P4.2 SOMI1
 * P4.1 SIMO1
 * P4.0 STE1
 *
 */
#define SPI_UCLK0_PORT   2
#define SPI_SOMI0_PORT   2
#define SPI_SIMO0_PORT   2
#define SPI_STE0_PORT    2

#define SPI_UCLK0_BIT    3
#define SPI_SOMI0_BIT    2
#define SPI_SIMO0_BIT    1
#define SPI_STE0_BIT     0

#define SPI_UCLK1_PORT   3
#define SPI_SOMI1_PORT   3
#define SPI_SIMO1_PORT   3
#define SPI_STE1_PORT    3

#define SPI_UCLK1_BIT    3
#define SPI_SOMI1_BIT    2
#define SPI_SIMO1_BIT    1
#define SPI_STE1_BIT     0

// system clock
#define __msp430_have_fll_and_xt2
#define __msp430_have_xt2 

// hwmul
#define __msp430_have_hwmul

// 8 bit blocks
#define __msp430_have_port1
#define __msp430_have_port2
#define __msp430_have_port3
#define __msp430_have_port4
#define __msp430_have_port5
#define __msp430_have_port6
#define __msp430_have_usart0
#define __msp430_have_usart0_with_i2c
#define __msp430_have_usart1
#define __msp430_have_cmpa
#define __msp430_have_svs_at_0x55
#define __msp430_have_basic_timer
#define __msp430_have_lcd

// 16 bit blocks
#define __msp430_have_timera3
#define __msp430_have_timerb7
#define __msp430_have_watchdog
#define __msp430_have_adc12
#define __msp430_have_flash

// Flash erase timings
#define FLASH_WRITE_TIMING_BYTE    35
#define FLASH_WRITE_TIMING_FSTBYTE 30
#define FLASH_WRITE_TIMING_NXTBYTE 21
#define FLASH_WRITE_TIMING_LSTBYTE  6
#define FLASH_ERASE_TIMING_MASS  5297
#define FLASH_ERASE_TIMING_SEG   4819

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
#elif defined(MSP430f1611) || defined(MSP430f1612)
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
#if defined(MSP430f1611)

/**
   1611 memory 48KB Flash + 10KB RAM
   ====
        Flash  0xffe0 - 0xffff : main : interrupt vector
               0x4000 - 0xffff : main : code memory     
        
        RAM    0x1900 - 0x38ff : 8KB extended RAM
               0x1100 - 0x18ff : ram mirrored from 0x0200 - 0x09ff

        Flash  0x1000 - 0x10ff : 256 bytes information Flash
        ROM    0x0c00 - 0x0fff : 1KB Boot ROM

              [0x0a00 - 0x0bff]  empty 512 bytes

        RAM    0x0200 - 0x09ff : 2KB RAM (mirrored at 0x18ff - 0x1100)
               0x0100 - 0x01ff : 16-bit peripherals
               0x0010 - 0x00ff :  8-bit peripherals
               0x0000 - 0x000f :  8 bit SFR
**/

#define MCU_BFD_MACH_ID    16 /* bfd_mach_msp16 */
#define MCU_VERSION        "f1611"
#define MCU_MODEL_NAME     "msp430f1611"

#define ADDR_FLASH_STOP    0xFFFFu
#define ADDR_FLASH_START   0x4000u
#define ADDR_RAM_STOP      0x38FFu
#define ADDR_RAM_START     0x1100u
#define ADDR_NVM_STOP      0x10ffu
#define ADDR_NVM_START     0x1000u
#define ADDR_BOOT_STOP     0x0fffu
#define ADDR_BOOT_START    0x0c00u
#define ADDR_MIRROR_STOP   0x09ffu
#define ADDR_MIRROR_START  0x0200u

#elif defined(MSP430f1612)

/**
   1612 memory 55KB Flash + 3KB RAM
   ====
        Flash  0xffe0 - 0xffff : main : interrupt vector
               0x2500 - 0xffff : main : code memory

        RAM    0x1900 - 0x24ff : 3KB extended RAM
               0x1100 - 0x18ff : ram mirrored from 0x0200 - 0x09ff

        Flash  0x1000 - 0x10ff : 256 bytes information Flash
        ROM    0x0c00 - 0x0fff : 1KB Boot ROM

              [0x0a00 - 0x0bff]  empty 512 bytes

        RAM    0x0200 - 0x09ff : 2KB RAM (mirrored at 0x18ff - 0x1100)
               0x0100 - 0x01ff : 16-bit peripherals
               0x0010 - 0x00ff :  8-bit peripherals
               0x0000 - 0x000f :  8 bit SFR
**/

#define MCU_BFD_MACH_ID    16 /* bfd_mach_msp16 */
#define MCU_VERSION        "f1612"
#define MCU_MODEL_NAME     "msp430f1612"

#define ADDR_FLASH_STOP    0xFFFFu
#define ADDR_FLASH_START   0x2500u
#define ADDR_RAM_STOP      0x24FFu
#define ADDR_RAM_START     0x1100u
#define ADDR_NVM_STOP      0x10ffu
#define ADDR_NVM_START     0x1000u
#define ADDR_BOOT_STOP     0x0fffu
#define ADDR_BOOT_START    0x0c00u
#define ADDR_MIRROR_STOP   0x09ffu
#define ADDR_MIRROR_START  0x0200u

#endif

//                          6         5         4         3         2         1         0
//                       3210987654321098765432109876543210987654321098765432109876543210
//                                                                                          
#define PIN_ZEO        0b0000000000000000000000000000000000000000000000000000000000000000ULL
#define P1DATA         0b0000000000000000000000000000000000000000000001111111100000000000ULL
#define P2DATA         0b0000000000000000000000000000000000000111111110000000000000000000ULL
#define P3DATA         0b0000000000000000000000000000011111111000000000000000000000000000ULL
#define P4DATA         0b0000000000000000000001111111100000000000000000000000000000000000ULL
#define P5DATA         0b0000000000000111111110000000000000000000000000000000000000000000ULL
#define P6DATA         0b0001110000000000000000000000000000000000000000000000000000111110ULL
#define TIMERA3        0b0000000000000000000000000000000000000000000000000000000000000000ULL

#define INTR_RESET        15
#define INTR_NMI          14
#define INTR_TIMERB7_0    13
#define INTR_TIMERB7_1    12
#define INTR_COMP_A       11
#define INTR_WATCHDOG     10
#define INTR_USART0_RX     9
#define INTR_USART0_TX     8
#define INTR_ADC12         7
#define INTR_TIMERA3_0     6 // CCR0 CCIFG
#define INTR_TIMERA3_1     5 // CCR1, CCR2, TAIFG
#define INTR_IOPORT1       4
#define INTR_USART1_RX     3
#define INTR_USART1_TX     2
#define INTR_IOPORT2       1
#define INTR_DAC12         0

#define INTR_DMA          INTR_DAC12
#define INTR_I2C          INTR_USART0_TX

#define VECTOR_RESET      0xFFFEu
#define VECTOR_NMI        0xFFFCu
#define VECTOR_TIMERB7_0  0xFFFAu
#define VECTOR_TIMERB7_1  0xFFF8u
#define VECTOR_COMP_A     0xFFF6u
#define VECTOR_WATCHDOG   0xFFF4u
#define VECTOR_USART0_RX  0xFFF2u
#define VECTOR_USART0_TX  0xFFF0u
#define VECTOR_ADC12      0xFFEEu
#define VECTOR_TIMERA3_0  0xFFECu /* 0xFFEC Timer A CC0 */
#define VECTOR_TIMERA3_1  0xFFEAu /* 0xFFEA Timer A CC1-2, TA */
#define VECTOR_IOPORT1    0xFFE8u
#define VECTOR_USART1_RX  0xFFE6u
#define VECTOR_USART1_TX  0xFFE4u
#define VECTOR_IOPORT2    0xFFE2u
#define VECTOR_DAC12      0xFFE0u

#define VECTOR_I2C        VECTOR_USART0_TX
#define VECTOR_DMA        VECTOR_DAC12

/**
 * Serial ports
 *
 * P3.3 UCLK0
 * P3.2 SOMI0
 * P3.1 SIMO0
 * P3.0 STE0
 *
 * P5.3 UCLK1
 * P5.2 SOMI1
 * P5.1 SIMO1
 * P5.0 STE1
 *
 */
#define SPI_UCLK0_PORT   2
#define SPI_SOMI0_PORT   2
#define SPI_SIMO0_PORT   2
#define SPI_STE0_PORT    2

#define SPI_UCLK0_BIT    3
#define SPI_SOMI0_BIT    2
#define SPI_SIMO0_BIT    1
#define SPI_STE0_BIT     0

#define SPI_UCLK1_PORT   4
#define SPI_SOMI1_PORT   4
#define SPI_SIMO1_PORT   4
#define SPI_STE1_PORT    4

#define SPI_UCLK1_BIT    3
#define SPI_SOMI1_BIT    2
#define SPI_SIMO1_BIT    1
#define SPI_STE1_BIT     0

// system clock
#define __msp430_have_basic_clock
#define __msp430_have_xt2 

// hwmul
#define __msp430_have_hwmul

// 8 bit blocks
#define __msp430_have_port1
#define __msp430_have_port2
#define __msp430_have_port3
#define __msp430_have_port4
#define __msp430_have_port5
#define __msp430_have_port6
#define __msp430_have_usart0
#define __msp430_have_usart0_with_i2c
#define __msp430_have_usart1
#define __msp430_have_cmpa
#define __msp430_have_svs_at_0x55

// 16 bit blocks
#define __msp430_have_timera3
#define __msp430_have_timerb7
#define __msp430_have_watchdog
#define __msp430_have_adc12
#define __msp430_have_dac12
#define __msp430_have_dma
#define __msp430_have_flash

// Flash erase timings
#define FLASH_WRITE_TIMING_BYTE    35
#define FLASH_WRITE_TIMING_FSTBYTE 30
#define FLASH_WRITE_TIMING_NXTBYTE 21
#define FLASH_WRITE_TIMING_LSTBYTE  6
#define FLASH_ERASE_TIMING_MASS  5297
#define FLASH_ERASE_TIMING_SEG   4819

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
#elif defined(MSP430f2013)

#define MCU_BFD_MACH_ID   20
#define MCU_VERSION       "f2013"         // "f201x"
#define MCU_MODEL_NAME    "msp430f2013"

#define ADDR_FLASH_STOP   0xFFFFu
#define ADDR_FLASH_START  0xF800u
#define ADDR_NVM_STOP     0x10ffu
#define ADDR_NVM_START    0x1000u
#define ADDR_RAM_STOP     0x027fu
#define ADDR_RAM_START    0x0200u

#define INTR_RESET        15
#define INTR_NMI          14
#define INTR_TIMERB3_0    13
#define INTR_TIMERB3_1    12
#define INTR_COMP_A       11
#define INTR_WATCHDOG     10
#define INTR_USART0_RX     9
#define INTR_USART0_TX     8
#define INTR_ADC12         7
#define INTR_TIMERA3_0     6 // CCR0 CCIFG
#define INTR_TIMERA3_1     5 // CCR1, CCR2, TAIFG
#define INTR_IOPORT1       4
#define INTR_UNUSED_1      3
#define INTR_UNUSED_2      2
#define INTR_IOPORT2       1
#define INTR_UNUSED_3      0

// system clock
#define __msp430_have_basic_clock
#define __msp430_have_xt2 

// 8 bit blocks
#define __msp430_have_port1
#define __msp430_have_port2
#define __msp430_have_port3
#define __msp430_have_port4
#define __msp430_have_port5
#define __msp430_have_port6
#define __msp430_have_usart0
#define __msp430_have_cmpa

// 16 bit blocks
#define __msp430_have_timera3
#define __msp430_have_timerb3
#define __msp430_have_watchdog
#define __msp430_have_adc10
#define __msp430_have_flash

// Flash erase timings
#define FLASH_WRITE_TIMING_BYTE       30
#define FLASH_WRITE_TIMING_FSTBYTE    25
#define FLASH_WRITE_TIMING_NXTBYTE    18
#define FLASH_WRITE_TIMING_LSTBYTE     6
#define FLASH_ERASE_TIMING_MASS    10593
#define FLASH_ERASE_TIMING_SEG      4819

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
#elif defined(MSP430f2274)

#define MCU_BFD_MACH_ID   22
#define MCU_VERSION       "f2274"
#define MCU_MODEL_NAME    "msp430f2274"

#define ADDR_FLASH_STOP   0xFFFFu
#define ADDR_FLASH_START  0xF800u
#define ADDR_NVM_STOP     0x10ffu
#define ADDR_NVM_START    0x1000u
#define ADDR_RAM_STOP     0x027fu
#define ADDR_RAM_START    0x0200u

#define INTR_RESET        15
#define INTR_NMI          14
#define INTR_TIMERB3_0    13
#define INTR_TIMERB3_1    12
#define INTR_COMP_A       11
#define INTR_WATCHDOG     10
#define INTR_USART0_RX     9
#define INTR_USART0_TX     8
#define INTR_ADC12         7
#define INTR_TIMERA3_0     6 // CCR0 CCIFG
#define INTR_TIMERA3_1     5 // CCR1, CCR2, TAIFG
#define INTR_IOPORT1       4
#define INTR_USART1_RX     3
#define INTR_USART1_TX     2
#if 0
#define INTR_UNUSED_1      3
#define INTR_UNUSED_2      2
#endif
#define INTR_IOPORT2       1
#define INTR_UNUSED_3      0

// system clock
#define __msp430_have_basic_clock
#define __msp430_have_basic_clock_plus
#define __msp430_have_xt2 

// 8 bit blocks
#define __msp430_have_port1
#define __msp430_have_port2
#define __msp430_have_port3
#define __msp430_have_port4

#define __msp430_have_usart0
#define __msp430_have_usart1
#if 0
#define __msp430_have_usci_a0  /* uart/lin + IrDA + SPI */
#define __msp430_have_usci_b0  /* SPI + I2C             */
#endif
#define __msp430_have_adc10
#define __msp430_have_cmpa
#define __msp430_have_cmpa_plus

#define __msp430_have_oa0
#define __msp430_have_oa1

// 16 bit blocks
#define __msp430_have_timera3
#define __msp430_have_timerb3
#define __msp430_have_watchdog
#define __msp430_have_watchdog_plus
#define __msp430_have_adc10
#define __msp430_have_flash


// Flash erase timings
#define FLASH_WRITE_TIMING_BYTE       30
#define FLASH_WRITE_TIMING_FSTBYTE    25
#define FLASH_WRITE_TIMING_NXTBYTE    18
#define FLASH_WRITE_TIMING_LSTBYTE     6
#define FLASH_ERASE_TIMING_MASS    10593
#define FLASH_ERASE_TIMING_SEG      4819

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */

#else
#error "you must define one MSP430 mcu model"
#endif // defined(model)

/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ** Common to all models ********************************************** */
/* ********************************************************************** */
/* ********************************************************************** */
/* ********************************************************************** */

#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)
#if defined(__msp430_have_timerb7)
#define INTR_TIMERB_0 INTR_TIMERB7_0
#define INTR_TIMERB_1 INTR_TIMERB7_1
#else
#define INTR_TIMERB_0 INTR_TIMERB3_0
#define INTR_TIMERB_1 INTR_TIMERB3_1
#endif
#endif

#if defined(WSIM_USES_GNU_BFD)
#define MCU_MACH_ID MCU_BFD_MACH_ID
#else
#define MCU_MACH_ID MCU_BFD_MACH_ID
#endif

#endif // MSP430_MODELS_H
