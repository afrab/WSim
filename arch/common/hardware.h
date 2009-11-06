
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

#include <inttypes.h>

#include "libtracer/tracer.h"
#include "liblogger/logger.h"
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
