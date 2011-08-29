/**
 *  \file   pcap.h
 *  \brief  PCAP packets logger 
 *  \author Antoine Fraboulet
 *  \date   2011
 **/

#ifndef _PCAP_H_
#define _PCAP_H_

/* list taken from /usr/include/pcap/pcap.h */

#define PCAP_DLT_USER0               147
#define PCAP_DLT_USER1               148
#define PCAP_DLT_USER2               149
#define PCAP_DLT_USER3               150
#define PCAP_DLT_USER4               151
#define PCAP_DLT_USER5               152

#define PCAP_DLT_802154              195

#define DEFAULT_PCAP_DLT PCAP_DLT_USER0

void logpkt_pcap_start(FILE* logfile, int linktype);
void logpkt_pcap_close(FILE* logfile);

void logpkt_pcap_logtx(FILE* logfile, uint8_t *pkt, int size, wsimtime_t start, wsimtime_t stop);
void logpkt_pcap_logrx(FILE* logfile, uint8_t *pkt, int size, wsimtime_t start, wsimtime_t stop);

#endif
