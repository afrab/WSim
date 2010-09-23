
/**
 *  \file   atmega128_models.h
 *  \brief  Atmega128 MCU definitions for models and platforms
 *  \author Antoine Fraboulet
 *  \date   2007
 **/


#ifndef ATMEGA128_MODELS_H
#define ATMEGA128_MODELS_H

#define MCU_NAME        "atmega128"
#define MCU_EM_ARCH_ID  83
#define MCU_BFD_ARCH_ID 53
#define MCU_IO_SIZE     126

#define INTEGRATE_PERIPHERALS()                 \
    MCU_DIGIIO()                                \
    MCU_WATCHDOG()


/* ********************************************************************** */
/* ********************************************************************** */
#if defined(ATMEGA128)
/* ********************************************************************** */
/* ********************************************************************** */

#define MCU_EM_MACH_ID      5 /* 133 */
#define MCU_BFD_MACH_ID     5
#define MCU_VERSION         "128"
#define MCU_MODEL_NAME      "Atmega128"
#define ATMEGA_PC_16BITS    1

// Clocks & Timers
#define __atmega128_have_watchdog

/* ********************************************************************** */
/* ********************************************************************** */
#elif defined(ATMEGA128L)
/* ********************************************************************** */
/* ********************************************************************** */

// #define bfd_mach_avr5 5
#define MCU_EM_MACH_ID      5 /* 133 */
#define MCU_BFD_MACH_ID     5
#define MCU_VERSION         "128"
#define MCU_MODEL_NAME      "Atmega128L"
#define ATMEGA_PC_16BITS    1

// Clocks & Timers
#define __atmega128_have_watchdog

/* ********************************************************************** */
/* ********************************************************************** */
#else
#error "you must define one atmega mcu model"
#endif // defined(model)

#if defined(WSIM_USES_GNU_BFD)
#define MCU_ARCH_ID MCU_BFD_ARCH_ID
#else
#define MCU_ARCH_ID MCU_EM_ARCH_ID
#endif

#if defined(WSIM_USES_GNU_BFD)
#define MCU_MACH_ID MCU_BFD_MACH_ID
#else
#define MCU_MACH_ID MCU_EM_MACH_ID
#endif

/* ********************************************************************** */
/* ********************************************************************** */
#endif // ATMEGA_MODELS
