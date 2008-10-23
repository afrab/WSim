/**
 *  \file   gdbremote.c
 *  \brief  GDB Remote functions
 *  \author Antoine Fraboulet
 *  \date   2005
 * 
 *  All rights reserved
 **/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "arch/common/hardware.h"
#include "machine/machine.h"
#include "libselect/libselect_socket.h"

#include "gdbremote.h"
#include "gdbremote_utils.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define GDB_CMD_BUFFER_SIZE     2048
#define HW_BREAKPOINT_SUPPORT      1

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* scan for the sequence $<data>#<checksum>     */

#define GDB_PKT_OK                 0
#define GDB_PKT_DEPRECATED         1
#define GDB_PKT_CHECKSUM_ERROR     2
#define GDB_PKT_WRITE_ERROR        3
#define GDB_PKT_READ_ERROR         4

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int
gdbremote_getchar(struct gdbremote_t *gdb)
{
  return libselect_skt_getchar(& gdb->skt);
}

static int
gdbremote_putchar(struct gdbremote_t *gdb, unsigned char c)
{
  return libselect_skt_putchar(& gdb->skt, c);
}

#define GDBREMOTE_PUTCHAR_OK    LIBSELECT_SOCKET_PUTCHAR_OK
#define GDBREMOTE_PUTCHAR_ERROR LIBSELECT_SOCKET_PUTCHAR_ERROR

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int
gdbremote_getpacket (struct gdbremote_t *gdb, char* buffer, int size)
{
  char          ch;
  int           count    = 0;
  unsigned char checksum = 0;

  /* wait around for the start character, ignore all other characters */
  do {
    ch = gdbremote_getchar (gdb);
    switch (ch)
      {
      case 0:
	return GDB_PKT_READ_ERROR;
      default:
	break;
      }
  } while ((ch != '$'));
  

  /* now, read until a # or end of buffer is found */
  do {
    ch = gdbremote_getchar (gdb);
    switch (ch)
      {
      case 0:
	return GDB_PKT_READ_ERROR;
      case '$':
	return GDB_PKT_READ_ERROR;
      case '#':
	/* End of data */
	break;
      default:
	buffer[count] = ch;
	count         = count + 1;
	checksum      = checksum + ch;
	break;
      }
  } while ((ch != '#') && (count < size));

  if (count == size)
    {
      DMSG_GDB("GDB:rcv: packet too long (limit = %d)\n",size);
      return GDB_PKT_READ_ERROR;
    }

  buffer[count] = 0;

  /* sequence id, deprecated since GDB 5.0 */
  if (buffer[2] == ':')
    return GDB_PKT_DEPRECATED;

  /* packet checksum validation */
  if (ch == '#')
    {
      int ch1, ch2;
      unsigned char sum = 0;
      ch1 = gdbremote_getchar (gdb);
      ch2 = gdbremote_getchar (gdb);

      sum = (gdbremote_hexchar2int(ch1) << 4) | gdbremote_hexchar2int(ch2);

      if (checksum == sum)
	{
	  if (gdbremote_putchar (gdb, '+') == GDBREMOTE_PUTCHAR_ERROR)
	    return GDB_PKT_WRITE_ERROR;
	  /* successful transfer */
	  return GDB_PKT_OK;
	}
      else
	{
	  if (gdbremote_putchar (gdb, '-') == GDBREMOTE_PUTCHAR_ERROR)
	    return GDB_PKT_WRITE_ERROR;
	  /* failed checksum */
	  return GDB_PKT_CHECKSUM_ERROR;
	}
    }

  /* packet without checksum */
  return GDB_PKT_READ_ERROR;
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* send the packet in buffer.  */

#define GDB_WRITE(cx)						\
  do {								\
    if (gdbremote_putchar(gdb,cx) == GDBREMOTE_PUTCHAR_ERROR)	\
      return;							\
  } while(0)

static void
gdbremote_putpacket (struct gdbremote_t *gdb, char *buffer)
{
  int retry = -1;
  /*  $<packet info>#<checksum>. */
  DMSG_GDB_CMD("GDB:snd: reply -%s-\n",buffer);
  do
    {
      int count;
      unsigned char ch;
      unsigned char checksum;

      checksum = 0;
      count    = 0;
      retry    = retry + 1;

      GDB_WRITE('$');
      while ((ch = buffer[count]) != 0)
	{
	  GDB_WRITE(ch);
	  checksum += ch;
	  count    += 1;
	}

      GDB_WRITE('#');
      GDB_WRITE(gdbremote_hexchars[checksum >>  4]);
      GDB_WRITE(gdbremote_hexchars[checksum & 0xf]);
    }
  while (gdbremote_getchar(gdb) != '+');
}

#undef GDB_WRITE

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define DONT_SUPPORT                                   \
  do {                                                 \
    DMSG_GDB("GDB:     unsupported command\n");        \
    gdbremote_putpacket(gdb,"");                       \
  } while (0) 

#define GDB_REPLY_OK                                   \
  do {						       \
    gdbremote_putpacket(gdb,"OK");		       \
  } while (0)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void gdbremote_getsectionoffsets(struct gdbremote_t *gdb)
{
  /*
   *	query section offsets qOffsets	
   *    Get section offsets.  
   *    Reply is Text=xxx;Data=yyy;Bss=zzz
   */

  int i;
  char tmp_buffer[GDB_CMD_BUFFER_SIZE] = "";
  char ret_buffer[GDB_CMD_BUFFER_SIZE] = "";

  if ((i = libelf_get_section_offset("text")) >= 0)
    {
      sprintf(tmp_buffer,"Text=%08x",i);
    }
  strcat(ret_buffer,tmp_buffer);

  if ((i = libelf_get_section_offset("data")) >= 0)
    {
      sprintf(tmp_buffer,";Data=%08x",i);
    }
  strcat(ret_buffer,tmp_buffer);

  if ((i = libelf_get_section_offset("bss")) >= 0)
    {
      sprintf(tmp_buffer,";Bss=%08x",i);
    }
  strcat(ret_buffer,tmp_buffer);

  gdbremote_putpacket(gdb,ret_buffer);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void 
gdbremote_general_query_packet(struct gdbremote_t *gdb, char* buffer, int UNUSED size)
{
  char* token;
  char delim[] = " ,;:";

  token = strtok(&buffer[1],delim); 

  if (strcmp(token,"C") == 0)
    {
      /* query current thread id */
      DMSG_GDB_CMD("GDB:     query current thread id\n");
      gdbremote_putpacket(gdb,"QC01");
    }
  else if (strcmp(token,"Offsets") == 0)
    {
      /* section offsets */
      gdbremote_getsectionoffsets(gdb);
    }
  else if (strcmp(token,"CRC") == 0)
    {
      DONT_SUPPORT;
    }
  else if (strcmp(token,"fThreadInfo") == 0)
    {
      /* thread list */
      gdbremote_putpacket(gdb,"m01");
    }
  else if (strcmp(token,"sThreadInfo") == 0)
    {
      /* end of thread list */
      gdbremote_putpacket(gdb,"l");
    }
  else if (strcmp(token,"ThreadExtraInfo") == 0)
    {
      int threadid;
      token = strtok(NULL,delim);
      threadid = strtol(token,NULL,16);
      if (threadid == 1)
	{
	  gdbremote_putpacket(gdb,"msp430 running thread");
	}
      else
	{
	  DONT_SUPPORT;
	}
    }
  else
    {
      DMSG_GDB_CMD("GDB:   unknown param name %s\n",token);
      DONT_SUPPORT;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_read_memory(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  int i;
  char *token;
  char delim[] = " ,;";
  char ret_buffer[GDB_CMD_BUFFER_SIZE];
  unsigned short addr;
  unsigned short length;
  
  ret_buffer[0] = '\0';
  /* buffer[0] == 'm' */
  token  = strtok(&buffer[1],delim);
  addr   = strtol(token,NULL,16);
  token  = strtok(NULL,delim);
  length = strtol(token,NULL,16);
  /* should check packet length */

  DMSG_GDB_CMD("GDB:     read memory at addr 0x%04x length 0x%x\n",addr,length);

  for(i=0; i<length; i++)
    {
      char hexb[3];
      uint8_t byte = mcu_jtag_read_byte(addr + i);

      sprintf(hexb,"%02x",byte);
      strcat(ret_buffer,hexb);
    } 
  gdbremote_putpacket(gdb,ret_buffer);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_write_memory(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  char *token;
  char delim[] = " ,;:";
  uint32_t addr;
  uint32_t i,length;
  
  /* buffer[0] == 'M'                    */
  /* `M'ADDR,LENGTH`:'XX... -- write mem */
  token  = strtok(&buffer[1],delim);
  addr   = strtol(token,NULL,16);
  token  = strtok(NULL,delim);
  length = strtol(token,NULL,16);
  /* should check packet length */
  token  = strtok(NULL,delim);

  DMSG_GDB_CMD("GDB:     write memory at addr 0x%04x length 0x%x = %s\n",addr,length,token);

  for(i=0; i< length; i++)
    {
      char hexb[3];
      uint8_t byte;
      
      hexb[0] = *token++;
      hexb[1] = *token++;
      byte = strtol(hexb,NULL,16);
      mcu_jtag_write_byte(addr + i, byte);
    }
  gdbremote_putpacket(gdb,"OK");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void gdbremote_write_memory_binary(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  char *token;
  char delim[] = " ,;:";
  uint32_t addr;
  uint32_t length;
  
  /* buffer[0] == 'X'                 */
  /* X ADDR,LENGTH:XX... -- write mem */
  token  = strtok(&buffer[1],delim);
  addr   = strtol(token,NULL,16);
  token  = strtok(NULL,delim);
  length = strtol(token,NULL,16);
  /* should check packet length */

  DMSG_GDB_CMD("GDB:     write binary at addr 0x%04x length 0x%x = -%s-\n",addr,length,token);

  /* gdbremote_putpacket(gdb,"OK"); */
  DONT_SUPPORT;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_read_registers(struct gdbremote_t *gdb)
{
  int i;
  char buffer[GDB_CMD_BUFFER_SIZE];

  buffer[0] = '\0';

  /* define TARGET_BYTE_ORDER_DEFAULT for MSP430 */
  /* define WORDS_BIGENDIAN for HOST             */

  for(i=0; i < mcu_registers_number(); i++)
    {
      char hexb[3];
      short regval = mcu_register_get(i);      

      sprintf(hexb,"%02x",regval & 0xff);
      strcat(buffer,hexb);

      sprintf(hexb,"%02x",(regval >> 8 ) & 0xff);
      strcat(buffer,hexb);
    }
  gdbremote_putpacket(gdb,buffer);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_read_one_register(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  char *token;
  int numreg, regval;
  char hexb[3];
  char delim[] = " ,;:";
  char ret_buffer[GDB_CMD_BUFFER_SIZE];

  ret_buffer[0] = '\0';
  
  token = strtok(&buffer[1],delim);
  numreg = strtol(token,NULL,16);

  regval = mcu_register_get(numreg);

  sprintf(hexb,"%02x",regval & 0xff);
  strcat(ret_buffer,hexb);

  sprintf(hexb,"%02x",(regval >> 8 ) & 0xff);
  strcat(ret_buffer,hexb);

  gdbremote_putpacket(gdb,ret_buffer);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static uint16_t gdbremote_4hex2short(char *token)
{
  char    hexb[3];
  uint8_t fst_byte = 0;
  uint8_t snd_byte = 0;

  if (strlen(token) > 0)
    {
      hexb[0]  = token[0];
      hexb[1]  = token[1];
      hexb[2]  = '\0';
      fst_byte = strtol(hexb,NULL,16);
    }

  if (strlen(token) > 2)
    {
      hexb[0]  = token[2];
      hexb[1]  = token[3];
      hexb[2]  = '\0';
      snd_byte = strtol(hexb,NULL,16);
    }

  /* define TARGET_BYTE_ORDER_DEFAULT for MSP430 */
  return (snd_byte << 8) | fst_byte;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void gdbremote_write_one_register(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  int numreg;
  char *token;
  char delim[] = " ,;:=";
  uint16_t regval;

  /* buffer[0] == 'P' */
  token  = strtok(&buffer[1],delim);
  numreg = strtol(token,NULL,16);
  
  token  = strtok(NULL,delim);
  regval = gdbremote_4hex2short(token); /* ASSUME: register size == 2 */

  /*  DMSG_GDB_CMD("   reg[%02x] = %04x \n",numreg,(high_byte << 8) | low_byte); */
  mcu_register_set(numreg, regval);

  gdbremote_putpacket(gdb,"OK");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_write_registers(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  int i;
  char *token;
  
  /* buffer[0] == 'G' */
  token  = &buffer[1];
  /* should check packet length */

  for(i=0; i< mcu_registers_number(); i++)
    {
      uint16_t regval;
      regval = gdbremote_4hex2short(token);
      token += 4;
      mcu_register_set(i, regval);
    }
  gdbremote_putpacket(gdb,"OK");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void 
gdbremote_stop_reply_packet(struct gdbremote_t *gdb)
{
  /*
    see arch/common/debug.c 

    POSIX.1-1990

       SIGHUP     1       Term    Déconnexion détectée sur le terminal
                                  de contrôle ou mort du processus de
                                  contrôle.
       SIGINT     2       Term    Interruption depuis le clavier.
       SIGQUIT    3       Core    Demande « Quitter » depuis le clavier.
       SIGILL     4       Core    Instruction illégale.
       SIGABRT    6       Core    Signal d'arrêt depuis abort(3).
       SIGFPE     8       Core    Erreur mathématique virgule flottante.
       SIGKILL    9       Term    Signal « KILL ».
       SIGSEGV   11       Core    Référence mémoire invalide.
       SIGPIPE   13       Term    Écriture dans un tube sans lecteur.
       SIGALRM   14       Term    Temporisation alarm(2) écoulée.
       SIGTERM   15       Term    Signal de fin.
       SIGUSR1   10       Term    Signal utilisateur 1.
       SIGUSR2   12       Term    Signal utilisateur 2.
       SIGCHLD   17       Ign     Fils arrêté ou terminé.
       SIGCONT   18       Cont    Continuer si arrêté.
       SIGSTOP   19       Stop    Arrêt du processus.
       SIGTSTP   20       Stop    Stop invoqué depuis tty.
       SIGTTIN   21       Stop    Lecture sur tty en arrière-plan.
       SIGTTOU   22       Stop    Écriture sur tty en arrière-plan.

     not in POSIX.1-1990, but in SUSv2 and POSIX.1-2001 :

       SIGBUS     7       Core    Erreur de bus (mauvais accès mémoire).
       SIGPOLL            Term    Événement « pollable » (Système V).
                                  Synonyme de SIGIO.
       SIGPROF   27       Term    Expiration de la temporisation
                                  pour le suivi.
       SIGSYS     -       Core    Mauvais argument de fonction (SVr4).
       SIGTRAP    5       Core    Point d'arrêt rencontré.
  */
  
  switch (gdb->last_signal)
    {
    case SIG_MCU_HUP:
      gdbremote_putpacket(gdb,"S01"); 
      break;
    case SIG_MCU_INT:
      gdbremote_putpacket(gdb,"S02"); 
      break;
    case SIG_MCU_QUIT:
      gdbremote_putpacket(gdb,"S03"); 
      break;
    case SIG_MCU_ABRT:
      gdbremote_putpacket(gdb,"S06"); 
      break;
    case SIG_MCU_ILL:
      gdbremote_putpacket(gdb,"S04"); 
      break;
    case SIG_MCU_TRAP:
      gdbremote_putpacket(gdb,"S05"); /* T05 */
      break;
    case SIG_MCU_BUS:
      gdbremote_putpacket(gdb,"S07"); 
      break;
    default: /* used by step */
      gdbremote_putpacket(gdb,"S05");  /* T05 */
      break;
    }
    
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_single_step(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  char *token;
  char delim[]    = " ,;:";
  int last_signal = SIG_MCU_TRAP;

  token = strtok(&buffer[1],delim);
  if (token != NULL)
    { 
      /* change the resume address */
      uint16_t s;
      s = gdbremote_4hex2short(token);
      DMSG_GDB_CMD("GDB:     resume at address %s\n",token);
      DMSG_GDB("gdbremote: resume at PC = 0x%x, current = 0x%04x\n",s,mcu_get_pc());
      mcu_set_pc_next(s);
    }
  else
    {
      DMSG_GDB("gdbremote: single step at PC current = 0x%04x , sig = 0x%0x %s\n",mcu_get_pc(),
	       mcu_signal_get(),mcu_signal_str());
    }

  /* we run for a single step by setting the GDB_SINGLE from the start 
   * end of the run will just have to determine the exact cause of the
   * stop (at this time this is left to GDB_SINGLE signal) */

  mcu_signal_add(SIG_GDB_SINGLE); 
  machine_run_free();

  DMSG_GDB("gdbremote: exit single step at 0x%04x (next 0x%4x, reg[0] 0x%04x) with signal = 0x%x (%s)\n",
	   mcu_get_pc(),mcu_get_pc_next(),mcu_register_get(0),mcu_signal_get(),mcu_signal_str());
  
  if ((mcu_signal_get() & ~SIG_GDB_SINGLE) != 0) 
    {
      /* we expected only GDB_SINGLE */
      DMSG_GDB("gdbremote: ==================================== \n");
      DMSG_GDB("gdbremote: out of single step with signal %s\n",mcu_signal_str());
      DMSG_GDB("gdbremote: ==================================== \n");
    }

  mcu_signal_remove(SIG_GDB_SINGLE);     /* step breakpoint        */

  if ((mcu_signal_get() & SIG_MCU) != 0)
    {
      if (mcu_signal_get() & SIG_MCU_ILL)
	last_signal = SIG_MCU_ILL;

      mcu_signal_remove(SIG_MCU_ALL   ); 
      mcu_signal_remove(SIG_MCU       );
    }

  if ((mcu_signal_get() & SIG_MAC) != 0)
    {
      mcu_signal_remove(SIG_MAC           );
      mcu_signal_remove(MAC_TO_SIG(MAC_ALL));
    }

  if ((mcu_signal_get() & SIG_GDB_IO) != 0)
    {
      mcu_signal_remove(SIG_GDB_IO    ); /* trafic on GDB TCP link */
    }

  gdb->last_signal = last_signal;

  gdbremote_stop_reply_packet(gdb);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_step_with_signal(struct gdbremote_t *gdb, char UNUSED *buffer, int UNUSED size)
{
  DONT_SUPPORT;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
gdbremote_continue(struct gdbremote_t *gdb, char UNUSED *buffer, int UNUSED size)
{
  char *token;
  char delim[]    = " ,;:";
  int last_signal = SIG_MCU_TRAP;

  token = strtok(&buffer[1],delim);
  if (token != NULL)
    { 
      /* change the resume address */
      uint16_t s;
      s = gdbremote_4hex2short(token);
      DMSG_GDB_CMD("GDB:     continue at address %s\n",token);
      DMSG_GDB("gdbremote: continue at PC = 0x%04x, current = 0x%04x\n",s,mcu_get_pc());
      mcu_set_pc_next(s);
    }
  else
    {
      DMSG_GDB("gdbremote: continue at PC current = 0x%04x\n",mcu_get_pc());
    }

#define GDB_STOP_BITMASK ~(SIG_WORLDSENS_IO)

  assert(libselect_register_signal(gdb->skt.socket, SIG_GDB_IO) == 0);   
  do 
    {
      machine_run_free();
    }
  while ((mcu_signal_get() & GDB_STOP_BITMASK) == 0);  /* we stop on anything but WORLDSENS_IO */ 
  assert(libselect_unregister(gdb->skt.socket) == 0);

  DMSG_GDB("gdbremote: exit continue at 0x%04x with signal = 0x%x (%s)\n",
	   mcu_get_pc(),mcu_signal_get(),mcu_signal_str());

  if ((mcu_signal_get() & SIG_MCU) != 0)
    {
      if (mcu_signal_get() & SIG_MCU_ILL)
	last_signal = SIG_MCU_ILL;

      mcu_signal_remove(SIG_MCU_ALL   ); 
      mcu_signal_remove(SIG_MCU       );
    }

  if ((mcu_signal_get() & SIG_MAC) != 0)
    {
      mcu_signal_remove(SIG_MAC           );
      mcu_signal_remove(MAC_TO_SIG(MAC_ALL));
    }

  if ((mcu_signal_get() & SIG_GDB_IO) != 0) 
    {
      /* we were interrupted, mostly using C-c, print some debug */
      int i;
      uint8_t ctl;
      for(i=0; i < 0xffff; i++)
	{
	  ctl = mcu_ramctl_read_ctl((uint16_t)i);
	  if ((ctl & MAC_BREAK_WATCH_FETCH) != 0)
	    {
	      DMSG_GDB("gdbremote: break registered at 0x%04x\n",i & 0xffff);
	    }
	}
    }

  mcu_signal_remove(SIG_GDB_IO    );  /* trafic on GDB TCP link */
  mcu_signal_remove(SIG_GDB_SINGLE);  /* should not happen here */
  gdb->last_signal = last_signal;

  gdbremote_stop_reply_packet(gdb);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(HW_BREAKPOINT_SUPPORT)
int breakpoint_insert(int type, uint32_t addr, int length)
{
  int i;
  DMSG_GDB_CMD("GDB:     insert breakpoint at 0x%x size %d\n", addr,length);

  for(i=0; i<length; i++)
    {
      mcu_ramctl_set_bp(addr + i, type);
    }
  return 0;
}
#endif

#if defined(HW_BREAKPOINT_SUPPORT)
int breakpoint_delete(int type, uint32_t addr, int length)
{
  int i;
  DMSG_GDB_CMD("GDB:     delete breakpoint at 0x%x size %d\n", addr,length);
  for(i=0; i<length; i++)
    {
      mcu_ramctl_unset_bp(addr + i, type);
    }
  return 0;
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(HW_BREAKPOINT_SUPPORT)
static void gdbremote_breakpoint(struct gdbremote_t *gdb, char *buffer, int UNUSED size)
{
  uint32_t type   =  0;
  uint32_t addr   =  0;
  uint32_t length =  0;

  int bpt;
  int (*bphandler)(int t, uint32_t addr, int size);
  char *format;

  /* z0,ADDR,LENGTH : remove breakpoint */
  /* Z0,ADDR,LENGTH : insert breakpoint */
  switch (buffer[0]) {
  case 'z':
    format    = "z%x,%x,%x";
    bphandler = breakpoint_delete;
    break;
  case 'Z':
    format    = "Z%x,%x,%x";
    bphandler = breakpoint_insert;
    break;
  default:
    gdbremote_putpacket(gdb,"E01");
    return;
  }

  if (sscanf(buffer,format,&type,&addr,&length) < 3)
    {
      DONT_SUPPORT;
      return ;
    }

  switch (type)
    {
    case   0: bpt = MAC_BREAK_SOFT;      break; /* BREAKPOINT_SOFTWARE; */
    case   1: bpt = MAC_BREAK_HARD;      break; /* BREAKPOINT_HARDWARE; */
    case   2: bpt = MAC_WATCH_READ;      break; /* BREAKPOINT_WRITE;    */
    case   3: bpt = MAC_WATCH_WRITE;     break; /* BREAKPOINT_READ;     */
    case   4: bpt = MAC_WATCH_ACCESS;    break; /* BREAKPOINT_ACCESS;   */
    default : bpt = MAC_BREAK_UNKNOWN;   break; /* BREAKPOINT_UNKNOWN   */
    }
  
  if (bphandler(bpt,addr,length) != 0)
    {
      DONT_SUPPORT;
    }
  else
    {
      gdbremote_putpacket(gdb,"OK");
    }
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int 
gdbremote_getcmd(struct gdbremote_t *gdb)
{
  int err;
  int size = GDB_CMD_BUFFER_SIZE;
  char buffer[GDB_CMD_BUFFER_SIZE];

  buffer[0] = 0;
  if (gdb->skt.socket == -1)
    return GDB_CMD_ERROR;

  if ((err = gdbremote_getpacket(gdb,buffer,size)) != GDB_PKT_OK)
    {
      ERROR("packet received with error %d. Closing Connection\n",err);
      libselect_skt_close_client(& gdb->skt);
      return GDB_CMD_DETACH;
    }

  DMSG_GDB_CMD("GDB:rcv: received pkt -%s-\n",buffer);

  switch (buffer[0])
    {
      /* **************** */
      /* REQUIRED COMMAND */
      /* **************** */

    case 'c': /* c addr : continue at address */
      /* Continue. addr is address to resume. If addr is omitted, resume at current address. */
      DMSG_GDB_CMD("GDB:     ** continue at address **\n");
      gdbremote_continue(gdb,buffer,size);
      break;

    case 's': /* s addr : stop */
      /* Single step. addr is the address at which to resume. If addr is omitted, resume at same address. */
      DMSG_GDB_CMD("GDB:     ** single step **\n");
      gdbremote_single_step(gdb,buffer,size);
      break;

    case 'g': /* read general registers */
      DMSG_GDB_CMD("GDB:     read general registers\n");
      gdbremote_read_registers(gdb);
      break;

    case 'G': /* write general registers */
      DMSG_GDB_CMD("GDB:     write general registers\n");
      gdbremote_write_registers(gdb,buffer,size);
      break;

    case 'm': /* `m addr,length '
		 Read length bytes of memory starting at address addr. Note that addr may
		 not be aligned to any particular boundary. */
      gdbremote_read_memory(gdb,buffer,size);
      break;

    case 'M': /* `M addr,length :XX...'
		 Write length bytes of memory starting at address addr. XX. . . is the data;
		 each byte is transmitted as a two-digit hexidecimal number. */
      gdbremote_write_memory(gdb,buffer,size);
      break;

      /* ***************** */
      /* OPTIONAL COMMANDS */
      /* ***************** */

    case 'd': /* toggle debug flag */
      DONT_SUPPORT;
      break;

    case 'e': /* unknown command, not documented anywhere ? */
      DONT_SUPPORT;
      break;

    case 'D': /* detach gdb from the remote target */
      DMSG_GDB_CMD("  detach gdb from remote target");
      libselect_skt_close_client(& gdb->skt);
      gdbremote_putpacket(gdb,"OK");
      return GDB_CMD_DETACH;
      break;

    case 'H':
      switch (buffer[1])
	{
	case 'c': /* step and continue */
	  {
	    int threadid = strtol (&buffer[2], NULL, 16);
	    switch (threadid)
	      {
	      case -1: /* all the threads */
		DMSG_GDB_CMD("GDB:     step and continue thread=%d : all the threads\n",threadid);
		gdbremote_putpacket(gdb,"OK");
		break;
	      case 0:  /* pick any thread */
		DMSG_GDB_CMD("GDB:     step and continue thread=%d : pick any thread\n",threadid);
		gdbremote_putpacket(gdb,"OK");
		break;
	      case 1:  /* thread 1 */
		DMSG_GDB_CMD("GDB:     step and continue thread=%d : selected thread\n",threadid);
		gdbremote_putpacket(gdb,"OK");
		break;
	      default: /* threadid > 1 */
		DMSG_GDB_CMD("GDB:     step and continue thread=%d : selected thread > 1\n",threadid);
		DONT_SUPPORT;
		break;
	      }
	  }
	  break;
	case 'g': /* other operations */
	  {
	    int threadid = strtol (&buffer[2], NULL, 16);
	    switch (threadid)
	      {
	      case -1: /* all the threads */
		gdbremote_putpacket(gdb,"OK");
		break;
	      case 0:  /* pick any thread */
		gdbremote_putpacket(gdb,"OK");
		break;
	      case 1:  /* thread 1 */
		gdbremote_putpacket(gdb,"OK");
		break;
	      default: /* threadid > 1 */
		DONT_SUPPORT;
		break;
	      }
	  }
	  break;
	default:
	  DONT_SUPPORT;
	  break;
	}
      break;

    case 'k': /* kill request */
      DMSG_GDB_CMD("GDB:     kill gdb from remote target");
      libselect_skt_close_client(& gdb->skt);
      return GDB_CMD_KILL;

    case 'p': /* read register */
      DMSG_GDB_CMD("GDB:     read one register\n");
      gdbremote_read_one_register(gdb,buffer,size);
      break;

    case 'P': /* write register */
      DMSG_GDB_CMD("GDB:     write one register\n");
      gdbremote_write_one_register(gdb,buffer,size);
      break;

    case 'r': /* reset */
      DMSG_GDB_CMD("GDB:     reset the entire system\n");
      machine_reset();
      gdbremote_putpacket(gdb,"OK");
      break;

    case 'R': /* Restart */
      DMSG_GDB_CMD("GDB:     restart the program being debugged\n");
      DONT_SUPPORT;
      /*
       * only available in extended mode
	 machine_reset();
	 gdbremote_putpacket(gdb,"OK");
      */
      break;

    case 'S': /* step with signal */
      DMSG_GDB_CMD("GDB:     step with signal\n");
      gdbremote_step_with_signal(gdb,buffer,size);
      break;

    case '?':
      gdbremote_stop_reply_packet(gdb);
      break;

    case '!':
      /* gdb->extended_mode = 1;  */
      /* remote server persistent */
      DONT_SUPPORT;
      break;

    case 'q': /* general query packet */
      gdbremote_general_query_packet(gdb,buffer,size);
      break;

    case 'Q': /* general set packet */
      DONT_SUPPORT;
      break;
      
    case 'X': /* `X addr,length :XX...'
		 Write data to memory, where the data is transmitted in binary. addr is address,
		 length is number of bytes, `XX ...' is binary data. The bytes 0x23 (ascii `#'), */
      gdbremote_write_memory_binary(gdb,buffer,size);
      break;

    case 'z': /* DMSG_GDB_CMD("GDB:     remove breakpoint\n");  */
#if defined(HW_BREAKPOINT_SUPPORT)
      gdbremote_breakpoint(gdb,buffer,size);
#else
      DONT_SUPPORT;
#endif
      break;

    case 'Z': /* DMSG_GDB_CMD("GDB:     insert breakpoint\n"); */
#if defined(HW_BREAKPOINT_SUPPORT)
      gdbremote_breakpoint(gdb,buffer,size);
#else
      DONT_SUPPORT;
#endif
      break;

    default:
      DMSG_GDB("GDB: *** remote : unknown command\n");
      DONT_SUPPORT;
      break;
    }

  return GDB_CMD_OK;
}

/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
