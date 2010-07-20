
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


#define DIGIIO_IS_INPUT(X)  (X == 0) ||                       \
                            (((X-1) % 3 == 0) &&              \
                            (X < 16)) || X == 18

#define DIGIIO_IS_DDR(X)    (((X-2) % 3 == 0) && (X < 16)) || \
                            (((X-1) % 3 == 0) && (X >= 16))

#define DIGIIO_IS_PORTX(X)  (X != 0) &&                       \
                            (((X % 3 == 0) && (X < 16)) ||    \
                            ((X+1 % 3 == 0) && (X >= 16)))

/*
 * Translate IO Address to array index range : 0 - 20
 * */
#define IO_ADDRESS_TO_IDX(X)                                  \
    X % 81 % 44 % 32

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
    PINF = 0,
    PINE,
    DDRE,
    PORTE,
    PIND = 13,
    DDRD,
    PORTD,
    PINC,
    DDRC,
    PORTC,
    PINB,
    DDRB,
    PORTB,
    PINA,
    DDRA,
    PORTA,
    DDRF = 65,
    PORTF,
    PING,
    DDRG,
    PORTG
};

//#define PINF      0x20 /* 32 */ /* 0 */ /* % 32 */
//
//#define PINE      0x21 /* 33 */ /* 1 */ /* % 32 */
//#define DDRE      0x22 /* 34 */ /* 2 */ /* % 32 */
//#define PORTE     0x23 /* 35 */ /* 3 */ /* % 32 */

//#define PIND      0x30 /* 48 */ /* 4 */ /* % 44 */
//#define DDRD      0x31 /* 49 */ /* 5 */ /* % 44 */
//#define PORTD     0x32 /* 50 */ /* 6 */ /* % 44 */

//#define PINC      0x33 /* 51 */ /* 7 */ /* % 44 */
//#define DDRC      0x34 /* 52 */ /* 8 */ /* % 44 */
//#define PORTC     0x35 /* 53 */ /* 9 */ /* % 44 */

//#define PINB      0x36 /* 54 */ /* 10 */ /* % 44 */
//#define DDRB      0x37 /* 55 */ /* 11 */ /* % 44 */
//#define PORTB     0x38 /* 56 */ /* 12 */ /* % 44 */

//#define PINA      0x39 /* 57 */ /* 13 */ /* % 44 */
//#define DDRA      0x3A /* 58 */ /* 14 */ /* % 44 */
//#define PORTA     0x3B /* 59 */ /* 15 */ /* % 44 */

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
