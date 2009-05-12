
/**
 *  \file   cc2420_rx.h
 *  \brief  CC2420 Rx methods
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_rx.h
 *  
 *
 *  Created by Nicolas Boulicault on 31/05/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_RX_H_
#define _CC2420_RX_H_

#include "cc2420_macros.h"
#include "cc2420_internals.h"

/*
 * max length of a received packet
 */

#define CC2420_MAX_RX_LEN     127


/*
 * addressing modes
 */

#define CC2420_ADDR_MODE_EMPTY     0
#define CC2420_ADDR_MODE_RESERVED  1
#define CC2420_ADDR_MODE_16_BITS   2
#define CC2420_ADDR_MODE_64_BITS   3


/* extract various fields from frame control field */
/* cf [2] p.121 */
/*
#define CC2420_FCF_FRAME_TYPE(x)    ((x & 0x0003)     )
#define CC2420_FCF_SEC_ENABLED(x)   ((x & 0x0008) >> 3)
#define CC2420_FCF_FRAME_PENDING(x) ((x & 0x0010) >> 4)
#define CC2420_FCF_ACK_REQUEST(x)   ((x & 0x0020) >> 5)
#define CC2420_FCF_INTRA_PAN(x)     ((x & 0x0040) >> 6)
#define CC2420_FCF_DST_ADDR_MODE(x) ((x & 0x0C00) >> 10)
#define CC2420_FCF_SRC_ADDR_MODE(x) ((x & 0xC000) >> 14)
*/

#define CC2420_FCF_FRAME_TYPE(x)    ((x & 0xE000) >> 13)
#define CC2420_FCF_SEC_ENABLED(x)   ((x & 0x1000) >> 12)
#define CC2420_FCF_FRAME_PENDING(x) ((x & 0x0800) >> 11)
#define CC2420_FCF_ACK_REQUEST(x)   ((x & 0x0400) >> 10)
#define CC2420_FCF_INTRA_PAN(x)     ((x & 0x0200) >>  9)
#define CC2420_FCF_DST_ADDR_MODE(x) ((x & 0x0030) >>  4)
#define CC2420_FCF_SRC_ADDR_MODE(x) ((x & 0x0003)      )

/* Length of 802.11.4 fcf fields */
#define CC2420_FCF_FRAME_TYPE_LENGTH    3
#define CC2420_FCF_SEC_ENABLED_LENGTH   1
#define CC2420_FCF_FRAME_PENDING_LENGTH 1
#define CC2420_FCF_ACK_REQUEST_LENGTH   1
#define CC2420_FCF_INTRA_PAN_LENGTH     1
#define CC2420_FCF_DST_ADDR_MODE_LENGTH 2
#define CC2420_FCF_SRC_ADDR_MODE_LENGTH 2


/**
 * update rssi values and update cca
 */

void cc2420_record_rssi(struct _cc2420_t * cc2420, double dBm);
int  cc2420_check_cca  (struct _cc2420_t * cc2420);
void cc2420_fcs_replace (struct _cc2420_t *cc2420);

#endif
