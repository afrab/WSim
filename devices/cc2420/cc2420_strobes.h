
/**
 *  \file   cc2420_strobes.h
 *  \brief  CC2420 strobe commands
 *  \author Nicolas Boulicault
 *  \date   2007
 **/

/*
 *  cc2420_strobes.h
 *  
 *
 *  Created by Nicolas Boulicault on 05/06/07.
 *  Copyright 2007 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_STROBES_H
#define _CC2420_STROBES_H

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * CC2420 command strobes, p 62
 */
#define CC2420_STROBE_SNOP             0x00 /* no op                                        */
#define CC2420_STROBE_SXOSCON          0x01 /* turn on oscillator                           */
#define CC2420_STROBE_STXCAL           0x02 /* enable and calibrate freq synthetizer for TX */
#define CC2420_STROBE_SRXON            0x03 /* enable RX                                    */
#define CC2420_STROBE_STXON            0x04 /* enable TX                                    */
#define CC2420_STROBE_STXONCCA         0x05 /* if channel clear, enable cal and TX          */
#define CC2420_STROBE_SRFOFF           0x06 /* disable RT/TX and freq synth                 */
#define CC2420_STROBE_SXOSCOFF         0x07 /* turn off crystal osc and RF                  */
#define CC2420_STROBE_SFLUSHRX         0x08 /* flush RX FIFO                                */
#define CC2420_STROBE_SFLUSHTX         0x09 /* flush TX FIFO                                */
#define CC2420_STROBE_SACK             0x0A /* send ACK with pending field clear            */
#define CC2420_STROBE_SACKPEND         0x0B /* send ACK with pending field set              */
#define CC2420_STROBE_SRXDEC           0x0C /* start RX FIFO inline decryption              */
#define CC2420_STROBE_STXENC           0x0D /* start TX FIFO inline encryption              */
#define CC2420_STROBE_SAES             0x0E /* AES standalone encryption                    */

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * Issue a command strobe
 */
void cc2420_strobe_command(struct _cc2420_t * cc2420);

/***************************************************/
/***************************************************/
/***************************************************/

#endif
