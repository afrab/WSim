/**
 *  \file   libelf.c
 *  \brief  Binary format program loader for simulation 
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "libelf.h"

#ifdef DEBUG
#    define HW_DMSG_ELF(x...) HW_DMSG_MACH(x)
#else
#    define HW_DMSG_ELF(x...) do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

uint16_t libelf_swap2(uint16_t v)
{
  uint16_t   r;
  uint8_t  *pv,*pr;

  pv = (uint8_t*)&v;
  pr = (uint8_t*)&r;
  
  pr[0] = pv[1];
  pr[1] = pv[0];

  return r;
}

/***************************************************/
/***************************************************/
/***************************************************/

uint32_t libelf_swap4(uint32_t v)
{
  uint32_t   r;
  uint8_t  *pv,*pr;

  pv = (uint8_t*)&v;
  pr = (uint8_t*)&r;
  
  pr[0] = pv[3];
  pr[1] = pv[2];
  pr[2] = pv[1];
  pr[3] = pv[0];

  return r;
}

/***************************************************/
/***************************************************/
/***************************************************/

void libelf_dump_section(uint8_t* data, uint32_t addr, uint32_t size, int maxlines)
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

/***************************************************/
/***************************************************/
/***************************************************/

#define SECTION_NUMBER  20
#define SECTION_MAXNAME 100

struct section_info_t {
  char name[SECTION_MAXNAME];
  int  addr;
  int  offset;
  int  size;
};

static struct section_info_t secinfo[SECTION_NUMBER];


int libelf_set_section_info(int UNUSED level, int i,char *name, int addr, int offset, int size)
{
  /*
    VERBOSE(level,"libelf:section:reg [%02d] name %10s: addr %06x, offset %04x, size %04x\n",
    i,name,addr,offset,size);
  */

  if (i >= SECTION_NUMBER)
    {
      ERROR("libelf: too much sections in elf file\n");
    }
  strncpy(secinfo[i].name,name, SECTION_MAXNAME);
  secinfo[i].addr   = addr;
  secinfo[i].offset = offset;
  secinfo[i].size   = size;
  return 0;
}


static void strtolower(char *dst, const char *src, int max)
{
  int i;
  int l = strlen(src);
  for(i=0; i<l && i<max; i++)
    {
      dst[i] = tolower(src[i]);
    }
  dst[l] = 0;
}

/* strcasestr doesn't exist in mingw32 */
int libelf_get_section_offset(const char* name)
{
  int i;
# define BUF_MAX 200
  char n[BUF_MAX];
  char s[BUF_MAX];

  strtolower(n,name,BUF_MAX);
  for(i=0; i<SECTION_MAXNAME; i++)
    {
      strtolower(s,secinfo[i].name,BUF_MAX);
      if (strstr(s,n) != NULL)
	{
	  HW_DMSG_ELF("libelf:section:get section %s offset\n",s);
	  // return secinfo[i].addr;
	  return 0;
	}
    }
  return -1;

# undef BUF_MAX
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
