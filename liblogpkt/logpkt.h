/**
 *  \file   logpkt.h
 *  \brief  Simulator packets logger header
 *  \author Loic Lemaitre
 *  \date   2010
 **/

#ifndef _LOGPKT_H_
#define _LOGPKT_H_

#include <stdint.h>


/**************************************************************************************
=== Packets log handler ===

Overview
========

It enables to log into a custom file (by default wsim-pkt.log) full frames received
and sent by the platform interfaces. It takes care about backtrack, so the log
reflects exactly real RX and TX.


Usage
=====

User 
----
2 options refer to this functionality, --logpkt and --logpktfile.
Setting one on command line, enables packets log.

--logpkt=[rx | tx | rxtx | pcap[:type] ]
rx          : only RX are logged
tx          : only TX are logged
rxtx        : both RX and TX are logged (by default)
pcap[:type] : pcap file format, optional Data Link Type identifier (default 802.15.4)

Parameters are optional.

--logpktfile=filename
Name of file where packets log will be stored.
Parameter is mandatory.

To use packets log with all default parameters, just put --logpkt on command line.
Without any of these 2 options, packet log is disabled.

Developper
----------
To add the log in a radio model, you first have to declare the interface with 
logpkt_init_interface().

Store every rx or tx byte with logpkt_rx_byte() or logpkt_rx_byte(). When a rx or tx 
frame is completed, use logpkt_rx_complete_pkt() or logpkt_tx_complete_pkt().

If a rx or tx is aborted (for instance overflow or underflow), use logpkt_rx_abort_pkt()
or logpkt_tx_abort_pkt(). The error parameter of these functions allows to write it
at the end of the byte stream of the aborted frame in the log file.

***************************************************************************************/

void logpkt_init               (int do_log_pkt, char* logpkt, const char* logpktfilename);

/* public encapsulation functions */
void (*logpkt_init_interface)  (int interface_id, const char* interface_name, int pcap_dlt);
void (*logpkt_close)           (void);
void (*logpkt_rx_byte)         (int interface_id, uint8_t val);
void (*logpkt_tx_byte)         (int interface_id, uint8_t val);
void (*logpkt_rx_complete_pkt) (int interface_id);
void (*logpkt_tx_complete_pkt) (int interface_id);
void (*logpkt_rx_abort_pkt)    (int interface_id, const char* error);
void (*logpkt_tx_abort_pkt)    (int interface_id, const char* error);
void (*logpkt_state_save)      (void);
void (*logpkt_state_restore)   (void);

/* public real functions */
void logpkt_init_interface_op  (int interface_id, const char* interface_name, int pcap_dlt);
void logpkt_close_op           (void);
void logpkt_rx_byte_op         (int interface_id, uint8_t val);
void logpkt_tx_byte_op         (int interface_id, uint8_t val);
void logpkt_rx_complete_pkt_op (int interface_id);
void logpkt_tx_complete_pkt_op (int interface_id);
void logpkt_rx_abort_pkt_op    (int interface_id, const char* error);
void logpkt_tx_abort_pkt_op    (int interface_id, const char* error);
void logpkt_state_save_op      (void);
void logpkt_state_restore_op   (void);

/* public nop functions */
void logpkt_init_interface_nop (int interface_id, const char* interface_name, int pcap_dlt);
void logpkt_close_nop          (void);
void logpkt_rx_byte_nop        (int interface_id, uint8_t val);
void logpkt_tx_byte_nop        (int interface_id, uint8_t val);
void logpkt_rx_complete_pkt_nop(int interface_id);
void logpkt_tx_complete_pkt_nop(int interface_id);
void logpkt_rx_abort_pkt_nop   (int interface_id, const char* error);
void logpkt_tx_abort_pkt_nop   (int interface_id, const char* error);
void logpkt_state_save_nop     (void);
void logpkt_state_restore_nop  (void);


#endif //_LOGPKT_H_
