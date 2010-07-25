
/**
 *  \file   atmega128_digiIO.h
 *  \brief  Atmega128 MCU Digital IO ports
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef ATMEGA128_DIGIIO_H
#define ATMEGA128_DIGIIO_H



/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/** 
 * input/output register 
 * 8bit register
 */
#define DIGIIO_REGS(X)      MCU.digiIO.gpio_regs[X]

#define DIGIIO_IS_PIN(X)    (X % 3 == 0)
#define DIGIIO_IS_DDR(X)    (X % 3 == 1)
#define DIGIIO_IS_PORTX(X)  (X % 3 == 2)

#define IDX_TO_DIGIIO_PORT(X) (X / 3)

#define DIGIIO_PIN 0
#define DIGIIO_DDR   1
#define DIGIIO_PORTX 2


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


/**
 * Digital IO port
 **/

#define DIGIIO_PA  0  /* 8-bit I/O    */
#define DIGIIO_PB  1  /* 8-bit I/O    */
#define DIGIIO_PC  2  /* 8-bit I/O    */
#define DIGIIO_PD  3  /* 8-bit I/O    */
#define DIGIIO_PE  4  /* 8-bit I/O    */
#define DIGIIO_PF  5  /* 8-bit analog */
#define DIGIIO_PG  6  /* 5-bit I/O    */

// IO addresses
enum gpio_regs_t {
    IO_REG_PINF = 32,
    IO_REG_PINE,
    IO_REG_DDRE,
    IO_REG_PORTE,
    IO_REG_PIND = 48,
    IO_REG_DDRD,
    IO_REG_PORTD,
    IO_REG_PINC,
    IO_REG_DDRC,
    IO_REG_PORTC,
    IO_REG_PINB,
    IO_REG_DDRB,
    IO_REG_PORTB,
    IO_REG_PINA,
    IO_REG_DDRA,
    IO_REG_PORTA,
    IO_REG_DDRF = 97,
    IO_REG_PORTF,
    IO_REG_PING,
    IO_REG_DDRG,
    IO_REG_PORTG
};


//#define PINE      0x21 /* 33 */ /* 0 */ /* % 32 */
//#define DDRE      0x22 /* 34 */ /* 1 */ /* % 32 */
//#define PORTE     0x23 /* 35 */ /* 2 */ /* % 32 */

//#define PIND      0x30 /* 48 */ /* 3 */ /* % 44 */
//#define DDRD      0x31 /* 49 */ /* 4 */ /* % 44 */
//#define PORTD     0x32 /* 50 */ /* 5 */ /* % 44 */

//#define PINC      0x33 /* 51 */ /* 6 */ /* % 44 */
//#define DDRC      0x34 /* 52 */ /* 7 */ /* % 44 */
//#define PORTC     0x35 /* 53 */ /* 8 */ /* % 44 */

//#define PINB      0x36 /* 54 */ /* 9 */ /* % 44 */
//#define DDRB      0x37 /* 55 */ /* 10 */ /* % 44 */
//#define PORTB     0x38 /* 56 */ /* 11 */ /* % 44 */

//#define PINA      0x39 /* 57 */ /* 12 */ /* % 44 */
//#define DDRA      0x3A /* 58 */ /* 13 */ /* % 44 */
//#define PORTA     0x3B /* 59 */ /* 14 */ /* % 44 */

//#define PINF      0x20 /* 32 */ /* 15 */ /* % 32 */
//#define DDRF      0x61 /* 97 */ /* 16 */ /* % 81 */
//#define PORTF     0x62 /* 98 */ /* 17 */ /* % 81 */

//#define PING      0x63 /* 99 */ /* 18 */ /* % 81 */
//#define DDRG      0x64 /*100 */ /* 19 */ /* % 81 */
//#define PORTG     0x65 /*101 */ /* 20 */ /* % 81 */


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


struct atmega128_digiIO_t {
    uint8_t in_updated;
    uint8_t out_updated;

    uint8_t gpio_regs[21];

    /** only for ports 1 & 2 **/
    uint8_t int_enable[2];
    uint8_t int_edge_select[2];
    uint8_t ifg[2];
};


inline uint8_t address_to_digiio_IDX(uint16_t addr);

void    atmega128_digiIO_init              (void);
void    atmega128_digiIO_reset             (void);

int8_t  atmega128_digiIO_mcu_read          (uint16_t addr);
void    atmega128_digiIO_mcu_write         (uint16_t addr, int8_t val);

int     atmega128_digiIO_dev_read          (int port_number, uint8_t *val);
void    atmega128_digiIO_dev_write         (int port_number, uint8_t val, uint8_t bitmask);

int     atmega128_digiIO_internal_dev_read (int port_number, uint8_t *val);
void    atmega128_digiIO_internal_dev_write(int port_number, uint8_t val, uint8_t bitmask);

void    atmega128_digiIO_update_done       (void);
int     atmega128_digiIO_chkifg            (void);

#endif
