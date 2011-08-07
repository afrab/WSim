/**
 *  \file   pcap.c
 *  \brief  PCAP packets logger 
 *  \author Antoine Fraboulet
 *  \date   2011
 **/

/**
 * This file uses a global variable to store the linktype, only one packet
 * capture type is available at this time. 
 * Use of multiple interface types will need to modify pcap.c
 **/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "pcap.h"


/* ************************************************** */
/* ************************************************** */

#define DEBUG_PCAP 1

#if DEBUG_PCAP != 0
#define PCAP_DBG(x...) DMSG_LIB(x) 
#else
#define PCAP_DBG(x...) do { } while (0)
#endif 

/* ************************************************** */
/* ************************************************** */

struct pcap_file_header {
  uint32_t magic;
  uint16_t version_major;
  uint16_t version_minor;
  int32_t  thiszone;     /* gmt to local correction */
  uint32_t sigfigs;      /* accuracy of timestamps */
  uint32_t snaplen;      /* max length saved portion of each pkt */
  uint32_t linktype;     /* data link type (LINKTYPE_*) */
};

struct pcap_pkthdr {
  //struct timeval ts;     /* time stamp */
  uint32_t tv_sec;       /* pcap uses 32 bits timeval */
  uint32_t tv_usec;
  uint32_t caplen;       /* length of portion present */
  uint32_t len;          /* length this packet (off wire) */
};

#define PCAP_MAGIC    0xa1b2c3d4
#define PCAP_MAJOR    2
#define PCAP_MINOR    4
#define PCAP_THISZONE 0
#define PCAP_SIGFIGS  0
#define PCAP_SNAPLEN  65536

/* ************************************************** */
/* ************************************************** */

static int pcap_dlt = -1;

/* ************************************************** */
/* ************************************************** */

void logpkt_pcap_start(FILE *logfile, int linktype)
{
  size_t shdr;
  struct pcap_file_header hdr; 

  if (pcap_dlt != -1)
    {
      ERROR("logpkt:pcap: only one packet type per node is allowed at this time\n");
    }

  pcap_dlt = linktype;
  
  hdr.magic         = PCAP_MAGIC; 
  hdr.version_major = PCAP_MAJOR;
  hdr.version_minor = PCAP_MINOR;
  hdr.thiszone      = PCAP_THISZONE;
  hdr.sigfigs       = PCAP_SIGFIGS; 
  hdr.snaplen       = PCAP_SNAPLEN;
  hdr.linktype      = pcap_dlt; 

  shdr = sizeof(hdr);
  if (fwrite(&hdr, shdr, 1, logfile) != 1)
    {
      ERROR("wsim:pcap:start: cannot create header\n");
    }
}

/* ************************************************** */
/* ************************************************** */

void logpkt_pcap_close(FILE UNUSED *logfile)
{
  /* nothing to do for standard pcap files */
}

/* ************************************************** */
/* ************************************************** */

void logpkt_pcap_logtx(FILE *logfile, uint8_t *pkt, int size, wsimtime_t start, wsimtime_t UNUSED stop)
{
  struct pcap_pkthdr hdr;
  uint8_t *pktdata;
  size_t shdr = sizeof(hdr);
  wsimtime_t usec = start / 1000;

  hdr.tv_sec     = usec / 1000000;
  hdr.tv_usec    = usec % 1000000;

  switch (pcap_dlt) 
    {
    case PCAP_DLT_802154:
      pktdata        = pkt + 6;
      hdr.caplen     = (size - 8);     // remove PREAMB (4) + SFD (1) + LEN (1) + [data] + FCS (2)
      hdr.len        = (size - 8) + 2; // Including the FCS fields
      break;
    default:
      pktdata        = pkt;
      hdr.caplen     = size;
      hdr.len        = size;
      break;
    }

  if (fwrite(&hdr, shdr, 1, logfile) != 1)
    {
      ERROR("logpkt:pcap:write: cannot write packet header\n");
    }
  if (fwrite(pktdata,1,hdr.caplen,logfile) != hdr.caplen)
    {
      ERROR("logpkt:pcap:write: cannot write packet\n");
    }
  PCAP_DBG("logpkt:pcap:write: hdr size=%d, pkt size=%d, start=%"PRIu64"\n",shdr,hdr.caplen,start);
}

/* ************************************************** */
/* ************************************************** */

void logpkt_pcap_logrx(FILE *logfile, uint8_t *pkt, int size, wsimtime_t start, wsimtime_t stop)
{
  /* standard pcap files do not contain RX/TX indicators */
  logpkt_pcap_logtx(logfile,pkt,size,start,stop);
}

/* ************************************************** */
/* ************************************************** */

