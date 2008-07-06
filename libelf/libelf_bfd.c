/**
 *  \file   libelf_bfd.c
 *  \brief  Binary format program loader for simulation using GNU BFD
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "arch/common/hardware.h"
#include "libelf.h"

#if defined(WSIM_USES_GNU_BFD)

/***************************************************/
/***************************************************/
/***************************************************/

#include "bfd.h"

#undef  FALSE
#undef  TRUE
#define FALSE 0
#define TRUE 1

#define LVL_SECT_INIT         1
#define LVL_SECT_INFO         2
#define LVL_SECT_DUMP         3
#define LVL_RAM_DUMP          8

/***************************************************/
/***************************************************/
/***************************************************/

#define DEBUG_BFD

#ifdef DEBUG_BFD
#    define HW_DMSG_MBFD(l,x...) VERBOSE(l,x)
#else
#    define HW_DMSG_MBFD(x...)   do { } while (0)
#endif

struct wsim_bfd_option {
  FILE    *out;
  int      verbose_level;
};

/***************************************************/
/***************************************************/
/***************************************************/


  // 001 SEC_ALLOC
  // 002 SEC_LOAD
  // 004 SEC_RELOC
  // 008 SEC_READONLY
  // 010 SEC_CODE
  // 020 SEC_DATA
  // 040 SEC_ROM


/***************************************************/
/***************************************************/
/***************************************************/

static void 
libelf_load_sections (bfd * exec, asection * sect, PTR x)
{
  uint8_t       mem[0x10000];
  char          *name        = (char*)bfd_section_name(exec,sect);
  bfd_size_type size         = bfd_section_size(exec,sect);
  bfd_vma       vma          = bfd_section_vma (exec,sect);
  int           align        = 1 << bfd_section_alignment(exec,sect);
  flagword flags             = sect->flags;
  struct wsim_bfd_option *o  = (struct wsim_bfd_option*)x;

  HW_DMSG_MBFD(2,"  -- ");
  HW_DMSG_MBFD(2,"section %14s ",name); 
  HW_DMSG_MBFD(2,"vma 0x%04x ",(unsigned short)vma);
  HW_DMSG_MBFD(2,"size 0x%04x ",(unsigned short)size);
  HW_DMSG_MBFD(2,"align %d ",align);
  HW_DMSG_MBFD(2,"flags 0x%04x -- ", flags);
 
  if (flags & SEC_LOAD)
    { 
      if (size > 0)
	{ 
	  if (bfd_get_section_contents (exec, sect, mem + vma, 0 , size) == TRUE)
	    {
	      HW_DMSG_MBFD(2,"%d bytes",(int)size);
	      if (o->verbose_level >= LVL_SECT_DUMP)
		{
		  HW_DMSG_MBFD(2,"\n\n");
		  libelf_dump_section(mem,vma,size,DUMP_LINES_SECTION);
		}
	      mcu_jtag_write_section(mem+vma,vma,size);
	    }
	  else 
	    {
	      HW_DMSG_MBFD(2,"read error");
	      ERROR("BFD: get_content error");
	    }
	}
      else 
	{
	  HW_DMSG_MBFD(2,"empty section");
	}
    }
  else
    {
      HW_DMSG_MBFD(2,"not loaded");
    }
  
  HW_DMSG_MBFD(2,"\n");
}

/***************************************************/
/***************************************************/
/***************************************************/

int libelf_load_exec_code(const char* filename, int verbose_level)
{
  static int     init_done = 0;
  bfd                *exec = NULL;
  bfd_vma            start = 0;
  struct wsim_bfd_option   option;
  struct wsim_bfd_option   *o = &option;


  option.out           = stdout;
  option.verbose_level = verbose_level;
  
  HW_DMSG_MBFD(LVL_SECT_INIT,"== Loading elf file : %s\n",filename);

  if (!init_done)
    {
      bfd_init();
      init_done = 1;
    }

  exec = bfd_openr (filename, NULL);

  if (!exec)
    {
      ERROR(" ** Cannot open file %s\n", filename);
      return 1;
    }

  if (!bfd_check_format (exec, bfd_object) && !(exec->flags & EXEC_P))
    {
      ERROR(" ** File %s is not an executable file\n",filename);
      bfd_close (exec);
      return 1;
    }

  if (bfd_get_arch(exec) != mcu_arch_id())
    {
      ERROR("===================================================================\n");
      ERROR("The software you are using is compiled for a different architecture\n");
      ERROR("Elf arch id : %d\n",bfd_get_arch(exec));
      ERROR("Simulator arch id : %d (%s)\n",mcu_arch_id(),mcu_name());
      ERROR("===================================================================\n");
      bfd_close(exec);
      return 1;
    }
  else if (bfd_get_mach(exec) != mcu_mach_id())
    {
      ERROR("===================================================================\n");
      ERROR("The software you are using is compiled for a different machine\n");
      ERROR("Elf machine id : %ld\n",bfd_get_mach(exec));
      ERROR("Simulator machine id : %d (%s)\n",mcu_mach_id(),mcu_name());
      ERROR("===================================================================\n");
      bfd_close(exec);
      return 1;
    }

  start = bfd_get_start_address(exec);
  mcu_set_pc_next(start);

  HW_DMSG_MBFD(2,"  Loading sections of executable '%s' for '%s' architecture in format '%s'\n",
	       bfd_get_filename (exec), bfd_printable_name (exec), exec->xvec->name);
  HW_DMSG_MBFD(2,"     architecture id : %d\n",bfd_get_arch(exec));
  HW_DMSG_MBFD(2,"     machine id      : %ld\n",bfd_get_mach(exec));
  HW_DMSG_MBFD(2,"     start address   : 0x%06x\n",(unsigned int)start);

  bfd_map_over_sections (exec, libelf_load_sections, o);

  bfd_close(exec);

#if 0
  if (! bfd_close (exec))
    {
      bfd_perror(exec->filename);
      return 1;
    }
#endif

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/
#endif /* WSIM_USES_GNU_BFD */
/***************************************************/
/***************************************************/
/***************************************************/
