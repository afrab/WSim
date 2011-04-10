
/**
 *  \file   msp430_sfr.h
 *  \brief  MSP430 Special functions registers definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef MSP430_SFR_H
#define MSP430_SFR_H

#define SFR_START 0x0
#define SFR_END   0x5

#define SFR_IE1   0x0
#define SFR_IE2   0x1
#define SFR_IFG1  0x2
#define SFR_IFG2  0x3
#define SFR_MER1  0x4
#define SFR_MER2  0x5

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) ie1_t {
  uint8_t
    utxie0:1,
    urxie0:1,
    accvie:1,
    nmiie:1,
    padding1:2,
    ofie:1,
    wdtie:1;
};
#else
struct __attribute__ ((packed)) ie1_t {
  uint8_t
    wdtie:1,
    ofie:1,
    padding1:2,
    nmiie:1,
    accvie:1,
    urxie0:1,
    utxie0:1;
};
#endif


#if defined(__msp430_have_uscia0) || defined(__msp430_have_uscib0)
    #if defined(WORDS_BIGENDIAN)
    struct __attribute__ ((packed)) ie2_t {
      uint8_t
	unused:4,
	ucb0txie:1,
	ucb0rxie:1,
	uca0txie:1,
	uca0rxie:1;
    };
    #else
    struct __attribute__ ((packed)) ie2_t {
      uint8_t
	uca0rxie:1,
	uca0txie:1,
	ucb0rxie:1,
	ucb0txie:1,
	unused:4;
    };
    #endif  
#else
    #if defined(WORDS_BIGENDIAN)
    struct __attribute__ ((packed)) ie2_t { 
      uint8_t
	btie:1,
	u6:1,
	utxie1:1,
	urxie1:1,
	u3:1,
	u2:1,
	u1:1,
	u0:1;
    };
    #else
    struct __attribute__ ((packed)) ie2_t { 
      uint8_t
	u0:1,
	u1:1,
	u2:1,
	u3:1,
	urxie1:1,
	utxie1:1,
	u6:1,
	btie:1;
    };
    #endif   
#endif
    

#if defined(__msp430_have_uscia0) || defined(__msp430_have_uscib0)
    #if defined(WORDS_BIGENDIAN)
    struct __attribute__ ((packed)) ifg1_t {
      uint8_t
        unused:3;
        nmiifg:1,
        rstifg:1,
        porifg:1,        
        ofifg:1,
        wdtifg:1;
    };
    #else
    struct __attribute__ ((packed)) ifg1_t {
      uint8_t
        wdtifg:1,
        ofifg:1,
        porifg:1,
        rstifg:1,        
        nmiifg:1,
        unused:3;        
    };
    #endif
#else
    #if defined(WORDS_BIGENDIAN)
    struct __attribute__ ((packed)) ifg1_t {
      uint8_t
        utxifg0:1,
        urxifg0:1,
        padding1:1,
        nmiifg:1,
        padding2:2,
        ofifg:1,
        wdtifg:1;
    };
    #else
    struct __attribute__ ((packed)) ifg1_t {
      uint8_t
        wdtifg:1,
        ofifg:1,
        padding2:2,
        nmiifg:1,
        padding1:1,
        urxifg0:1,
        utxifg0:1;
    };
    #endif
#endif


#if defined(__msp430_have_uscia0) || defined(__msp430_have_uscib0)
    #if defined(WORDS_BIGENDIAN)
    struct __attribute__ ((packed)) ifg2_t {
      uint8_t
	unused:4,
	ucb0txifg:1,
	ucb0rxifg:1,
	uca0txifg:1,
	uca0rxifg:1;
    };
    #else
    struct __attribute__ ((packed)) ifg2_t {
      uint8_t
	uca0rxifg:1,
	uca0txifg:1,
	ucb0rxifg:1,
	ucb0txifg:1,
	unused:4;
    };
    #endif
#else
    #if defined(WORDS_BIGENDIAN)
    struct __attribute__ ((packed)) ifg2_t {
      uint8_t
	btifg:1,
	u6:1,
	utxifg1:1,
	urxifg1:1,
	u3:1,
	u2:1,
	u1:1,
	u0:1;
    };
    #else
    struct __attribute__ ((packed)) ifg2_t {
      uint8_t
	u0:1,
	u1:1,
	u2:1,
	u3:1,
	urxifg1:1,
	utxifg1:1,
	u6:1,
	btifg:1;
    };
    #endif
#endif


#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) me1_t {
  uint8_t
    utxe0:1,
    urxe0:1,   // uspie0
    padding:6;    
};
#else
struct __attribute__ ((packed)) me1_t {
  uint8_t
    padding:6,
    urxe0:1,   // uspie0
    utxe0:1;
};
#endif

#if defined(WORDS_BIGENDIAN)
struct __attribute__ ((packed)) me2_t {
  uint8_t
    u7:1,
    u6:1,
    utxe1:1,
    urxe1:1, // uspie1
    padding:4;    
};
#else
struct __attribute__ ((packed)) me2_t {
  uint8_t
    padding:4,
    urxe1:1, // uspie1
    utxe1:1,
    u6:1,
    u7:1;
};
#endif

/**
 * SFR Data Structure
 **/
struct msp430_sfr_t
{
  union {
    struct ie1_t     b;
    int8_t           s;
  } ie1;
  union {
    struct ie2_t     b;
    uint8_t          s;
  } ie2;
  union {
    struct ifg1_t    b;
    uint8_t          s;
  } ifg1;
  union {
    struct ifg2_t    b;
    uint8_t          s;
  } ifg2;
  union {
    struct me1_t     b;
    uint8_t          s;
  } me1;
  union {
    struct me2_t     b;
    uint8_t          s;
  } me2;
};


void    msp430_sfr_create();
void    msp430_sfr_reset();
#define msp430_sfr_update() do { } while (0)

int8_t  msp430_sfr_read (uint16_t addr);
void    msp430_sfr_write(uint16_t addr, int8_t val);

#endif
