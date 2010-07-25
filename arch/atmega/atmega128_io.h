
/**
 *  \file   atmega128_io.h
 *  \brief  Atmega128 MCU Memory mapped IO 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef ATMEGA128_IO_H
#define ATMEGA128_IO_H


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

typedef int8_t  (*io_read8_t  ) (uint16_t addr);
typedef void    (*io_write8_t ) (uint16_t addr, int8_t val);

typedef int8_t  (*peripheral_read8_t  ) (uint16_t addr);
typedef void    (*peripheral_write8_t ) (uint16_t addr, int8_t val);

typedef int16_t (*peripheral_read16_t ) (uint16_t addr);
typedef void    (*peripheral_write16_t) (uint16_t addr, int16_t val);

struct atmega128_io_addr_fptr_t {
  io_read8_t     read;
  io_write8_t    write;
  char           *name;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if 0

0xFF  Reserved
/*  ..  Reserved */
0x9E  Reserved
0x9D  UCSR1C
0x9C  UDR1
0x9B  UCSR1A
0x9A  UCSR1B
0x99  UBRR1L
0x98  UBRR1H
0x97  Reserved
0x96  Reserved
0x95  UCSR0C
0x94  Reserved
0x93  Reserved
0x92  Reserved
0x91  Reserved
0x90  UBRR0H
0x8F  Reserved
0x8E  Reserved
0x8D  Reserved
0x8C  TCCR3C
0x8B  TCCR3A
0x8A  TCCR3B
0x89  TCNT3H
0x88  TCNT3L
0x87  OCR3AH
0x86  OCR3AL
0x85  OCR3BH
0x84  OCR3BL
0x83  OCR3CH
0x82  OCR3CL
0x81  ICR3H
0x80  ICR3L
0x7F  Reserved
0x7E  Reserved
0x7D  ETIMSK
0x7C  ETIFR
0x7B  Reserved
0x7A  TCCR1C
0x79  OCR1CH
0x78  OCR1CL
0x77  Reserved
0x76  Reserved
0x75  Reserved
0x74  TWCR
0x73  TWDR
0x72  TWAR
0x71  TWSR
0x70  TWBR
0x6F  OSCCAL
0x6E  Reserved
0x6D  XMCRA
0x6C  XMCRB
0x6B  Reserved
0x6A  EICRA
0x69  Reserved
0x68  SPMCSR
0x67  Reserved
0x66  Reserved
0x65  PORTG
0x64  DDRG
0x63  PING
0x62  PORTF
0x61  DDRF
0x60  Reserved

0x3F /* 0x5F */ SREG
0x3E /* 0x5E */ SPH
0x3D /* 0x5D */ SPL
0x3C /* 0x5C */ XDIV
0x3B /* 0x5B */ RAMPZ
0x3A /* 0x5A */ EICRB
0x39 /* 0x59 */ EIMSK
0x38 /* 0x58 */ EIFR
0x37 /* 0x57 */ TIMSK
0x36 /* 0x56 */ TIFR
0x35 /* 0x55 */ MCUCR
0x34 /* 0x54 */ MCUCSR
0x33 /* 0x53 */ TCCR0
0x32 /* 0x52 */ TCNT0
0x31 /* 0x51 */ OCR0
0x30 /* 0x50 */ ASSR
0x2F /* 0x4F */ TCCR1A
0x2E /* 0x4E */ TCCR1B
0x2D /* 0x4D */ TCNT1H
0x2C /* 0x4C */ TCNT1L
0x2B /* 0x4B */ OCR1AH
0x2A /* 0x4A */ OCR1AL
0x29 /* 0x49 */ OCR1BH
0x28 /* 0x48 */ OCR1BL
0x27 /* 0x47 */ ICR1H
0x26 /* 0x46 */ ICR1L
0x25 /* 0x45 */ TCCR2
0x24 /* 0x44 */ TCNT2
0x23 /* 0x43 */ OCR2
0x22 /* 0x42 */ OCDR
0x21 /* 0x41 */ WDTCR
0x20 /* 0x40 */ SFIOR
0x1F /* 0x3F */ EEARH
0x1E /* 0x3E */ EEARL
0x1D /* 0x3D */ EEDR
0x1C /* 0x3C */ EECR
0x1B /* 0x3B */ PORTA
0x1A /* 0x3A */ DDRA
0x19 /* 0x39 */ PINA
0x18 /* 0x38 */ PORTB
0x17 /* 0x37 */ DDRB
0x16 /* 0x36 */ PINB
0x15 /* 0x35 */ PORTC
0x14 /* 0x34 */ DDRC
0x13 /* 0x33 */ PINC
0x12 /* 0x32 */ PORTD
0x11 /* 0x31 */ DDRD
0x10 /* 0x30 */ PIND
0x0F /* 0x2F */ SPDR
0x0E /* 0x2E */ SPSR
0x0D /* 0x2D */ SPCR
0x0C /* 0x2C */ UDR0
0x0B /* 0x2B */ UCSR0A
0x0A /* 0x2A */ UCSR0B
0x09 /* 0x29 */ UBRR0L
0x08 /* 0x28 */ ACSR
0x07 /* 0x27 */ ADMUX
0x06 /* 0x26 */ ADCSRA
0x05 /* 0x25 */ ADCH
0x04 /* 0x24 */ ADCL
0x03 /* 0x23 */ PORTE
0x02 /* 0x22 */ DDRE
0x01 /* 0x21 */ PINE
0x00 /* 0x20 */ PINF

#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


#define ATMEGA128_BYTE(A,S,O,...)     ATMEGA128_##S##_##O##_BYTE(A,##__VA_ARGS__)

#define ATMEGA128_IO_SPACE_READ_BYTE(A)       atmega128_io_read_byte(A)
#define ATMEGA128_IO_SPACE_WRITE_BYTE(A,V)    atmega128_io_write_byte(A,V)

#define ATMEGA128_RAM_SPACE_READ_BYTE(A)      atmega128_ram_read_byte(A)
#define ATMEGA128_RAM_SPACE_WRITE_BYTE(A,V)   atmega128_ram_write_byte(A,V)

#define ATMEGA128_SHORT(A,S,O,...)    ATMEGA128_##S##_##O##_SHORT(A,##__VA_ARGS__)

#define ATMEGA128_IO_SPACE_READ_SHORT(A)      atmega128_io_read_byte(A)
#define ATMEGA128_IO_SPACE_WRITE_SHORT(A,V)   atmega128_io_write_byte(A,V)

#define ATMEGA128_RAM_SPACE_READ_SHORT(A)     atmega128_ram_read_short(A)
#define ATMEGA128_RAM_SPACE_WRITE_SHORT(A,V)  atmega128_ram_write_short(A,V)


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


#define IO_REG_Reserved_0xFF 0xFF
/*  ..  Reserved */
#define IO_REG_Reserved_0x9E 0x9E
#define IO_REG_UCSR1C        0x9D
#define IO_REG_UDR1          0x9C
#define IO_REG_UCSR1A        0x9B
#define IO_REG_UCSR1B        0x9A
#define IO_REG_UBRR1L        0x99
#define IO_REG_UBRR1H        0x98
#define IO_REG_Reserved_0x97 0x97
#define IO_REG_Reserved_0x96 0x96
#define IO_REG_UCSR0C        0x95
#define IO_REG_Reserved_0x94 0x94
#define IO_REG_Reserved_0x93 0x93
#define IO_REG_Reserved_0x92 0x92
#define IO_REG_Reserved_0x91 0x91
#define IO_REG_UBRR0H        0x90
#define IO_REG_Reserved_0x8F 0x8F
#define IO_REG_Reserved_0x8E 0x8E
#define IO_REG_Reserved_0x8D 0x8D
#define IO_REG_TCCR3C        0x8C
#define IO_REG_TCCR3A        0x8B
#define IO_REG_TCCR3B        0x8A
#define IO_REG_TCNT3H        0x89
#define IO_REG_TCNT3L        0x88
#define IO_REG_OCR3AH        0x87
#define IO_REG_OCR3AL        0x86
#define IO_REG_OCR3BH        0x85
#define IO_REG_OCR3BL        0x84
#define IO_REG_OCR3CH        0x83
#define IO_REG_OCR3CL        0x82
#define IO_REG_ICR3H         0x81
#define IO_REG_ICR3L         0x80
#define IO_REG_Reserved_0x7F 0x7F
#define IO_REG_Reserved_0x7E 0x7E
#define IO_REG_ETIMSK        0x7D
#define IO_REG_ETIFR         0x7C
#define IO_REG_Reserved_0x7B 0x7B
#define IO_REG_TCCR1C        0x7A
#define IO_REG_OCR1CH        0x79
#define IO_REG_OCR1CL        0x78
#define IO_REG_Reserved_0x77 0x77
#define IO_REG_Reserved_0x76 0x76
#define IO_REG_Reserved_0x75 0x75
#define IO_REG_TWCR          0x74
#define IO_REG_TWDR          0x73
#define IO_REG_TWAR          0x72
#define IO_REG_TWSR          0x71
#define IO_REG_TWBR          0x70
#define IO_REG_OSCCAL        0x6F
#define IO_REG_Reserved_0x6E 0x6E
#define IO_REG_XMCRA         0x6D
#define IO_REG_XMCRB         0x6C
#define IO_REG_Reserved_0x6B 0x6B
#define IO_REG_EICRA         0x6A
#define IO_REG_Reserved_0x69 0x69
#define IO_REG_SPMCSR        0x68
#define IO_REG_Reserved_0x67 0x67
#define IO_REG_Reserved_0x66 0x66

#define IO_REG_Reserved_0x60 0x60

/* io regs address starting from 0 */

#define IO_REG_SREG          0x3F /* 0x5F */
#define IO_REG_SPH           0x3E /* 0x5E */
#define IO_REG_SPL           0x3D /* 0x5D */
#define IO_REG_XDIV          0x3C /* 0x5C */
#define IO_REG_RAMPZ         0x3B /* 0x5B */
#define IO_REG_EICRB         0x3A /* 0x5A */
#define IO_REG_EIMSK         0x39 /* 0x59 */
#define IO_REG_EIFR          0x38 /* 0x58 */
#define IO_REG_TIMSK         0x37 /* 0x57 */
#define IO_REG_TIFR          0x36 /* 0x56 */
#define IO_REG_MCUCR         0x35 /* 0x55 */
#define IO_REG_MCUCSR        0x34 /* 0x54 */
#define IO_REG_TCCR0         0x33 /* 0x53 */
#define IO_REG_TCNT0         0x32 /* 0x52 */
#define IO_REG_OCR0          0x31 /* 0x51 */
#define IO_REG_ASSR          0x30 /* 0x50 */
#define IO_REG_TCCR1A        0x2F /* 0x4F */
#define IO_REG_TCCR1B        0x2E /* 0x4E */
#define IO_REG_TCNT1H        0x2D /* 0x4D */
#define IO_REG_TCNT1L        0x2C /* 0x4C */
#define IO_REG_OCR1AH        0x2B /* 0x4B */
#define IO_REG_OCR1AL        0x2A /* 0x4A */
#define IO_REG_OCR1BH        0x29 /* 0x49 */
#define IO_REG_OCR1BL        0x28 /* 0x48 */
#define IO_REG_ICR1H         0x27 /* 0x47 */
#define IO_REG_ICR1L         0x26 /* 0x46 */
#define IO_REG_TCCR2         0x25 /* 0x45 */
#define IO_REG_TCNT2         0x24 /* 0x44 */
#define IO_REG_OCR2          0x23 /* 0x43 */
#define IO_REG_OCDR          0x22 /* 0x42 */
#define IO_REG_WDTCR         0x21 /* 0x41 */
#define IO_REG_SFIOR         0x20 /* 0x40 */
#define IO_REG_EEARH         0x1F /* 0x3F */
#define IO_REG_EEARL         0x1E /* 0x3E */
#define IO_REG_EEDR          0x1D /* 0x3D */
#define IO_REG_EECR          0x1C /* 0x3C */
#define IO_REG_SPDR          0x0F /* 0x2F */
#define IO_REG_SPSR          0x0E /* 0x2E */
#define IO_REG_SPCR          0x0D /* 0x2D */
#define IO_REG_UDR0          0x0C /* 0x2C */
#define IO_REG_UCSR0A        0x0B /* 0x2B */
#define IO_REG_UCSR0B        0x0A /* 0x2A */
#define IO_REG_UBRR0L        0x09 /* 0x29 */
#define IO_REG_ACSR          0x08 /* 0x28 */
#define IO_REG_ADMUX         0x07 /* 0x27 */
#define IO_REG_ADCSRA        0x06 /* 0x26 */
#define IO_REG_ADCH          0x05 /* 0x25 */
#define IO_REG_ADCL          0x04 /* 0x24 */


#define IO_MEM_SIZE 0
#define IO_POINTERS 1

/* 
 * Atmega 128 data memory map
 *
 * Data Memory
 * 32 Registers     $0000 - $001F
 * 64 I/O Registers $0020 - $005F
 * 160 Ext I/O Reg. $0060 - $00FF
 *                  $0100
 * Internal SRAM
 *   (4096 x 8)
 *                  $10FF
 *                  $1100
 * External SRAM
 *   (0 - 64K x 8)
 *                  $FFFF
 */


void    atmega128_io_init       (void);

void     atmega128_io_register(uint8_t addr, io_read8_t reader, io_write8_t writer, char * name);

int8_t   atmega128_read_byte         (uint16_t loc, uint8_t space);
void     atmega128_write_byte        (uint16_t loc, int8_t  val, uint8_t space);

int16_t  atmega128_read_short        (uint16_t loc, uint8_t space);
void     atmega128_write_short       (uint16_t loc, int16_t val, uint8_t space);

int8_t   atmega128_io_read_byte      (uint16_t loc);
void     atmega128_io_write_byte     (uint16_t loc, int8_t val);

int8_t   atmega128_ram_read_byte     (uint16_t loc);
void     atmega128_ram_write_byte    (uint16_t loc, int8_t  val);

int16_t  atmega128_ram_read_short    (uint16_t loc); 
void     atmega128_ram_write_short   (uint16_t loc, int16_t val); 

int8_t   atmega128_flash_read_byte   (uint32_t loc);
int16_t  atmega128_flash_read_short  (uint32_t loc);


/* void    atmega128_flash_write_byte  (uint16_t loc, int8_t  val); */
/* void    atmega128_flash_write_short (uint16_t loc, int16_t val); */

#endif
