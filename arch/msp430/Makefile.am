if BUILD_MSP430

noinst_LIBRARIES = \
	libmsp430f135.a		\
	libmsp430f149.a		\
	libmsp430f449.a		\
	libmsp430f1611.a	\
	libmsp430f2013.a	\
	libmsp430f2274.a

INCLUDES=-I$(top_srcdir)

MSPSRC= 				          \
	msp430.h             msp430.c		  \
	msp430_debug.h       msp430_debug.c       \
	msp430_alu.h         msp430_alu.c	  \
	msp430_hwmul.h       msp430_hwmul.c       \
	msp430_intr.h        msp430_intr.c        \
	msp430_io.h          msp430_io.c          \
	msp430_basic_clock.h msp430_basic_clock.c \
	msp430_fll_clock.h   msp430_fll_clock.c   \
	msp430_basic_timer.h msp430_basic_timer.c \
	msp430_timer.h       msp430_timer.c       \
	msp430_digiIO.h      msp430_digiIO.c      \
	msp430_lcd.h         msp430_lcd.c         \
	msp430_sfr.h         msp430_sfr.c         \
	msp430_usart.h       msp430_usart.c       \
	msp430_watchdog.h    msp430_watchdog.c    \
	msp430_dma.h         msp430_dma.c         \
	msp430_flash.h       msp430_flash.c	  \
	msp430_svs.h         msp430_svs.c	  \
	msp430_cmpa.h        msp430_cmpa.c        \
	msp430_adc12.h       msp430_adc12.c       \
	msp430_adc10.h       msp430_adc10.c       \
        msp430_dac12.h       msp430_dac12.c

libmsp430f135_a_CFLAGS=-DMSP430f135
libmsp430f135_a_SOURCES=${MSPSRC}

libmsp430f149_a_CFLAGS=-DMSP430f149
libmsp430f149_a_SOURCES=${MSPSRC}

libmsp430f449_a_CFLAGS=-DMSP430f449
libmsp430f449_a_SOURCES=${MSPSRC}

libmsp430f1611_a_CFLAGS=-DMSP430f1611
libmsp430f1611_a_SOURCES=${MSPSRC}

libmsp430f2013_a_CFLAGS=-DMSP430f2013
libmsp430f2013_a_SOURCES=${MSPSRC}

libmsp430f2274_a_CFLAGS=-DMSP430f2274
libmsp430f2274_a_SOURCES=${MSPSRC}

endif
