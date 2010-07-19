
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
// digiIO type declarations

// Port input pins
struct __attribute__ ((packed)) digiIO_pin_t {
    uint8_t
        pin0:1,
        pin1:1,
        pin2:1,
        pin3:1,
        pin4:1,
        pin5:1,
        pin6:1,
        pin7:1;
};

// Port direction register
struct __attribute__ ((packed)) digiIO_ddr_t {
    uint8_t
        ddr0:1,
        ddr1:1,
        ddr2:1,
        ddr3:1,
        ddr4:1,
        ddr5:1,
        ddr6:1,
        ddr7:1;
};

// Port data register
struct __attribute__ ((packed)) digiIO_portx_t {
    uint8_t
        port0:1,
        port1:1,
        port2:1,
        port3:1,
        port4:1,
        port5:1,
        port6:1,
        port7:1;
};

struct digiIOport_t {
    union {
        struct digiIO_pin_t b;
        uint8_t        s;
    } gpio_pin;

    union {
        struct digiIO_ddr_t b;
        uint8_t        s;
    } gpio_ddr;

    union {
        struct digiIO_portx_t b;
        uint8_t        s;
    } gpio_portx;
};

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


/*
 * Translate IO Address to array index range : 0 - 20
 * */
#define IO_ADDRESS_TO_IDX(X)                           \
    X %= 81;/* maps IO addresses from 97-101 to 16-20*/ \
    X %= 44;/* maps IO addresses from 48-59 to 4-15 */   \
    X %= 32 /* maps IO addresses from 32-35 to 0-3 */

// IO addresses
#define PINF      0x20 /* 32 */ /* 0 */ /* % 32 */

#define PINE      0x21 /* 33 */ /* 1 */ /* % 32 */
#define DDRE      0x22 /* 34 */ /* 2 */ /* % 32 */
#define PORTE     0x23 /* 35 */ /* 3 */ /* % 32 */

#define PIND      0x30 /* 48 */ /* 4 */ /* % 44 */
#define DDRD      0x31 /* 49 */ /* 5 */ /* % 44 */
#define PORTD     0x32 /* 50 */ /* 6 */ /* % 44 */

#define PINC      0x33 /* 51 */ /* 7 */ /* % 44 */
#define DDRC      0x34 /* 52 */ /* 8 */ /* % 44 */
#define PORTC     0x35 /* 53 */ /* 9 */ /* % 44 */

#define PINB      0x36 /* 54 */ /*10 */ /* % 44 */
#define DDRB      0x37 /* 55 */ /*11 */ /* % 44 */
#define PORTB     0x38 /* 56 */ /*12 */ /* % 44 */

#define PINA      0x39 /* 57 */ /* 13 */ /* % 44 */
#define DDRA      0x3A /* 58 */ /* 14 */ /* % 44 */
#define PORTA     0x3B /* 59 */ /* 15 */ /* % 44 */

#define DDRF      0x61 /* 97 */ /* 16 */ /* % 81 */
#define PORTF     0x62 /* 98 */ /* 17 */ /* % 81 */

#define PING      0x63 /* 99 */ /* 18 */ /* % 81 */
#define DDRG      0x64 /*100 */ /* 19 */ /* % 81 */
#define PORTG     0x65 /*101 */ /* 20 */ /* % 81 */


/**
 * direction : port switch
 * 8bit register
 * PxDIR : 0 = input direction, 1 = output direction
 */
#define DIGIIO_DIR(p)    MCU.digiIO.direction[p]


struct atmega128_digiIO_t {
    uint8_t in_updated;
    uint8_t out_updated;

    struct digiIOport_t[7];

    uint8_t selection[7];

    /** only for ports 1 & 2 **/
    uint8_t int_enable[2];
    uint8_t int_edge_select[2];
    uint8_t ifg[2];
};

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
