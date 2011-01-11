
/**
 *  \file   hardware.h
 *  \brief  WSim hardware definitions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef HW_HARDWARE_H
#define HW_HARDWARE_H

#include "config.h"
#include "missing.h"

typedef uint64_t wsimtime_t;

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#include <inttypes.h>

#include "libtracer/tracer.h"
#include "liblogger/logger.h"
#include "liblogpkt/logpkt.h"
#include "libselect/libselect.h"
#include "libetrace/libetrace.h"
#include "libwsnet/libwsnet.h"
#include "libgui/ui.h"
#include "arch/common/debug.h"
#include "arch/common/mcu.h"
#include "devices/devices_fd.h"
#include "machine/machine_fd.h"

#define BIT(w,n)   ((w >> n) & 1)

#endif
