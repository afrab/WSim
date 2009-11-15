
/**
 *  \file   debug.c
 *  \brief  WSim internals debug functions
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "hardware.h"



/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

// val might be unused if HW_DMSG is undefined
void debug_write_binary(int UNUSED val, int size)
{
  int8_t i;
  for(i = size-1 ; i >= 0 ; i--)
    {
      HW_DMSG("%c", '0' + ((val >> i) & 1));
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
  taken from man 7 signal on linux x86_64 glibc 

       Signal     Value     Action   Comment
       -------------------------------------------------------------------------
       SIGHUP        1       Term    Hangup detected on controlling terminal
                                     or death of controlling process
       SIGINT        2       Term    Interrupt from keyboard
       SIGQUIT       3       Core    Quit from keyboard
       SIGILL        4       Core    Illegal Instruction
       SIGABRT       6       Core    Abort signal from abort(3)
       SIGFPE        8       Core    Floating point exception
       SIGKILL       9       Term    Kill signal
       SIGSEGV      11       Core    Invalid memory reference
       SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
       SIGALRM      14       Term    Timer signal from alarm(2)
       SIGTERM      15       Term    Termination signal
       SIGUSR1   30,10,16    Term    User-defined signal 1
       SIGUSR2   31,12,17    Term    User-defined signal 2
       SIGCHLD   20,17,18    Ign     Child stopped or terminated
       SIGCONT   19,18,25    Cont    Continue if stopped
       SIGSTOP   17,19,23    Stop    Stop process
       SIGTSTP   18,20,24    Stop    Stop typed at tty
       SIGTTIN   21,21,26    Stop    tty input for background process
       SIGTTOU   22,22,27    Stop    tty output for background process
*/

#define SIG_NAME_MAX 30

char* unix_signal_to_str(int sig)
{
  static char sig_unknown[SIG_NAME_MAX]; 

  switch (sig)
    {
    case 0       : return "";
    case SIGINT  : return "SIGINT";
    case SIGILL  : return "SIGILL";
    case SIGABRT : return "SIGABRT";
    case SIGFPE  : return "SIGFPE";
    case SIGSEGV : return "SIGSEGV";
    case SIGTERM : return "SIGTERM";
#if defined(SIGHUP)
    case SIGHUP  : return "SIGHUP";
    case SIGQUIT : return "SIGQUIT";
    case SIGKILL : return "SIGKILL";
    case SIGPIPE : return "SIGPIPE";
    case SIGUSR1 : return "SIGUSR1";      
    case SIGUSR2 : return "SIGUSR2";      
    case SIGALRM : return "SIGALRM";
    case SIGSTOP : return "SIGSTOP";
    case SIGTSTP : return "SIGTSTP";
    case SIGCONT : return "SIGCONT";
#endif
    default      : 
      sprintf(sig_unknown,"unknown %d",sig);
      return sig_unknown;
    }
}


char* host_signal_str(int sig)
{
  static char buff[SIG_NAME_MAX];

#if defined(FUNC_STRSIGNAL_DEFINED)
  strcpy(buff,strsignal(sig));
#else
  // sprintf(buff,"%d",sig);
  strcpy(buff,unix_signal_to_str(sig));
#endif

  return buff;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static char* mcu_single_signal_str(uint32_t sig)
{
  static char sig_unknown[SIG_NAME_MAX]; 
  switch (sig)
    {
    case 0                  : return "";
    case SIG_MCU_HUP        : return "SIG_MCU_HUP";
    case SIG_MCU_INT        : return "SIG_MCU_INT";
    case SIG_MCU_QUIT       : return "SIG_MCU_QUIT";
    case SIG_MCU_ILL        : return "SIG_MCU_ILL";
    case SIG_MCU_TRAP       : return "SIG_MCU_TRAP";
    case SIG_MCU_ABRT       : return "SIG_MCU_ABRT";
    case SIG_MCU_BUS        : return "SIG_MCU_BUS";
    case SIG_MCU_TSTP       : return "SIG_MCU_TSTP";
    case SIG_MCU_LPM_CHANGE : return "SIG_MCU_LPM_CHANGE";
    case SIG_MCU            : return "SIG_MCU";
    case SIG_RUN_INSN       : return "SIG_RUN_INSN";
    case SIG_RUN_TIME       : return "SIG_RUN_TIME";
    case SIG_GDB_SINGLE     : return "SIG_GDB_TRAP";
    case SIG_GDB_IO         : return "SIG_GDB_IO";
    case SIG_CON_IO         : return "SIG_CON_IO";
    case SIG_WORLDSENS_IO   : return "SIG_WORLDSENS_IO";
    case SIG_UI             : return "SIG_UI";
    case SIG_HOST           : return "SIG_HOST";
    case SIG_MAC            : return "SIG_MAC";
    default                 : 
      snprintf(sig_unknown, sizeof(sig_unknown), "unknown %"PRIx32, sig);
      return sig_unknown;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void mcu_signal_str_add(char* buff, char* str, int *first)
{
  if ((str != NULL) && (strcmp(str,"") != 0))
    {
      if (*first == 0)
	{
	  strcat(buff,", ");
	}
      else
	{
	  *first = 0;
	}
      
      strcat(buff,str);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* mcu_signal_str(void)
{
  int i,first;
  uint32_t sig;
  char *s_sig;
  static char buff[200];

  sig = mcu_signal_get();
  buff[0] = '\0';
  first = 1;

  /* first byte depends on SIG_MCU or SIG_HOST */
  if ((sig & SIG_MCU) != 0)
    {
      strcpy(buff, "mcu(");
      for(i=0x01; i<=0x80; i<<=1)
	{
	  s_sig = mcu_single_signal_str(sig & i);
	  mcu_signal_str_add(buff, s_sig, &first);
	}
      strcat(buff, ")");
      first = 0;
    }
  else if ((sig & SIG_HOST) != 0)
    {
      s_sig = host_signal_str(sig & 0xff);
      sprintf(buff, "host(%s)", s_sig);
      first = 0;
    }
  else if (((sig & SIG_MCU) != 0) && ((sig & SIG_HOST) != 0))
    {
      ERROR("wsim: Internal and host signal received at the same time\n");
      strcpy(buff, "host+sig"); 
      first = 0;
    }

  /* signals */
  for(i=SIG_RUN_INSN; i<=SIG_UI; i<<=1)
    {
      s_sig = mcu_single_signal_str(sig & i);
      mcu_signal_str_add(buff, s_sig, &first);
    }

  /* last part, memory access control */
  if ((sig & SIG_MAC) != 0)
    {
      mcu_signal_str_add(buff, "mac(", &first);
      first = 1;
      for(i=MAC_BREAK_SOFT; i<=MAC_MUST_WRITE_FIRST; i<<=1)
	{
	  s_sig = mcu_ramctl_str(SIG_TO_MAC(sig) & i);
	  mcu_signal_str_add(buff, s_sig, &first);
	}
      strcat(buff,")");
    }
  
  if (strcmp(buff,"") == 0)
    {
      strcpy(buff,"none");
    }
  return buff;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* mcu_ramctl_str(int type)
{
  switch(type)
    {
    case MAC_BREAK_UNKNOWN   : return NULL;
    case MAC_BREAK_SOFT      : return "software";
    case MAC_BREAK_HARD      : return "hardware";
    case MAC_WATCH_READ      : return "read";
    case MAC_WATCH_WRITE     : return "write";
    case MAC_MUST_WRITE_FIRST: return "write before read";
    }
  return NULL;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define DUMP_COLS 8
void debug_dump_section(uint8_t* data, uint32_t addr, uint32_t size, int maxlines)
{
  int i;
  int line, col;
  
  for (line = 0; (line < maxlines) && (((unsigned)line)*2*DUMP_COLS < size); line ++)
    {
      uint32_t laddr = addr + line * 2 * DUMP_COLS;
      OUTPUT("%04x  ",laddr);
      
      for(i=0; i<2; i++)
	{
	  for(col = 0; col < DUMP_COLS; col ++)
	    {
	      OUTPUT("%02x ",data[addr + (line*2+i)*(DUMP_COLS) + col]);
	    }
	  OUTPUT(" ");
	}

      OUTPUT("|");
      for(col = 0; col < 2*DUMP_COLS; col ++)
	{
	  char c = data[addr + line * 2 * DUMP_COLS + col];
	  OUTPUT("%c",(isprint(c) ? c : '.'));
	}
      OUTPUT("|\n");
    }  
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
