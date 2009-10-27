/**
 *  \file   libelf_ntv.c
 *  \brief  Binary format program loader for simulation using native code
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

#include "arch/common/hardware.h"
#include "libelf.h"

#define VELF 4
#define VSYS 5
#define VDMP 6

/* ************************************************** */
/* libentry point is libelf_load_exec_code()          */
/* ************************************************** */


/* ************************************************** */
/* ** ELF Types ************************************* */
/* ************************************************** */

typedef uint32_t elf32_addr_t;
typedef uint16_t elf32_half_t;
typedef uint32_t elf32_off_t;
typedef  int32_t elf32_sword_t;
typedef uint32_t elf32_word_t;


#define ELF32_NIDENT     16
struct elf32_header_struct_t {
  unsigned char   ident[ELF32_NIDENT];
  elf32_half_t    type;
  elf32_half_t    machine;
  elf32_word_t    version;
  elf32_addr_t    entry;
  elf32_off_t     phoff;       /* start of program headers          */
  elf32_off_t     shoff;       /* start of section headers          */
  elf32_word_t    flags;
  elf32_half_t    ehsize;      /* size of this header               */
  elf32_half_t    phentsize;   /* size of program headers           */
  elf32_half_t    phnum;       /* number of program headers         */
  elf32_half_t    shentsize;   /* size of section header            */
  elf32_half_t    shnum;       /* number of section header          */
  elf32_half_t    shstrndx;    /* section header string table index */
};
typedef struct elf32_header_struct_t elf32_header_t;

#define ELF_IDENT_CLASS            4
#define ELF_IDENT_DATA             5
#define ELF_IDENT_VERSION          6
#define ELF_IDENT_ABI_TYPE         7
#define ELF_IDENT_ABI_VERSION      8
#define ELF_IDENT_PAD              9

enum elf_class_enum_t {
  elf_class_none = 0,
  elf_class_32   = 1,
  elf_class_64   = 2
};
typedef enum elf_class_enum_t elf_class_t;

enum elf_data_enum_t {
  elf_data_none  = 0,
  elf_data_lsb   = 1,
  elf_data_msb   = 2
};
typedef enum elf_data_enum_t elf_data_t;

enum elf_type_enum_t {
  elf_type_none   = 0,
  elf_type_rel    = 1,
  elf_type_exec   = 2,
  elf_type_dyn    = 3,
  elf_type_core   = 4,
  elf_type_loproc = 0xff00,    /* Processor-specific                */
  elf_type_hiproc = 0xffff     /* Processor-specific                */
};
typedef enum elf_type_enum_t elf_type_t;

enum elf_abi_type_enum_t {
  elf_abi_none =        0 ,
  elf_abi_hpux =        1 ,
  elf_abi_netbsd =      2 ,
  elf_abi_linux =       3 ,
  elf_abi_solaris =     6 ,
  elf_abi_aix =         7 ,
  elf_abi_irix =        8 ,
  elf_abi_freebsd =     9 ,
  elf_abi_openbsd =     12 ,
  elf_abi_standalone =  255 
};
typedef enum elf_abi_type_enum_t elf_abi_type_t;

/* section header */
struct elf32_sh_struct_t {
  elf32_word_t sh_name;        /* index of the section name         */
  elf32_word_t sh_type;        /* section type                      */
  elf32_word_t sh_flags;       /* section flags                     */
  elf32_addr_t sh_addr;        /* address in process memory         */
  elf32_off_t  sh_offset;      /* offset in the file                */
  elf32_word_t sh_size;        /* size in bytes                     */
  elf32_word_t sh_link;        /* section header index link         */
  elf32_word_t sh_info;        /* extra information                 */
  elf32_word_t sh_addralign;   /* address alignment                 */
  elf32_word_t sh_entsize;     /* st entry size                     */
};
typedef struct elf32_sh_struct_t elf32_sh_t;

/* string table entry */
struct elf32_st_entry_struct_t {
  elf32_word_t  st_name;
  elf32_addr_t  st_value;
  elf32_word_t  st_size;
  unsigned char st_info;
  unsigned char st_other;
  elf32_half_t  st_shndx;
};
typedef struct elf32_st_entry_struct_t elf32_st_entry_t;

/* program header */
struct elf32_ph_struct_t {
  elf32_word_t p_type;
  elf32_off_t  p_offset;
  elf32_addr_t p_vaddr;
  elf32_addr_t p_paddr;
  elf32_word_t p_filesz;
  elf32_word_t p_memsz;
  elf32_word_t p_flags;
  elf32_word_t p_align;
};
typedef struct elf32_ph_struct_t elf32_ph_t;

/* elf file */
#define ELF_MAX_FILENAME 256
struct elf32_struct_t {
  char              file_name[ELF_MAX_FILENAME];
  uint32_t          file_size;
  uint8_t          *file_raw;

  elf_class_t       class;
  elf_data_t        data;
  elf_type_t        type;

  int arch;
  int mach;

  elf32_header_t    elf_header;
  elf32_sh_t       *elf_section;
  elf32_ph_t       *elf_program;
};
typedef struct elf32_struct_t *elf32_t;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

elf32_t libelf_open(const char* filename)
{
  FILE *f;
  elf32_t e = NULL;
  struct stat s;
  size_t rcount;

  if (stat(filename,&s) == -1)
    {
      VERBOSE(VSYS,"wsim:libelf:");
      return NULL;
    }

  VERBOSE(VSYS,"libelf: file size %d\n",s.st_size);

  if ((e = (elf32_t)malloc(sizeof(struct elf32_struct_t))) == NULL)
    {
      return NULL;
    }
  memset(e,0,sizeof(struct elf32_struct_t));

  if ((e->file_raw = (uint8_t*)malloc(sizeof(uint8_t)*s.st_size)) == NULL)
    {
      free(e);
      return NULL;
    }
  memset(e->file_raw,0,sizeof(uint8_t)*s.st_size);

  VERBOSE(VSYS,"libelf: memory allocation ok\n");

  strncpy(e->file_name,filename,ELF_MAX_FILENAME);
  e->file_size = s.st_size;

  if ((f = fopen(filename,"rb")) == NULL)
    {
      perror("wsim:libelf:");
      return NULL;
    }
  

#if 0
  // mingw32 cross compile seems to have an error ?
  rcount    = 0;
  do {
    size_t rcounttmp;
    rcounttmp = 0;
    rcounttmp = fread(e->file_raw + rcount, 1, e->file_size - rcount, f);
    if (rcounttmp <= 0)
      {
	ERROR("libelf: error while reading elf file [%s]\n",filename);
	free(e->file_raw);
	free(e);
	return NULL;
      }
    ERROR("libelf: read %d bytes\n", rcounttmp);
    rcount += rcounttmp;
  } while (rcount < e->file_size);
#else
  if ((rcount = fread(e->file_raw, 1, e->file_size, f)) < e->file_size)
    {
      ERROR("libelf: error while reading elf file [%s]: read too short (%d,%d)\n",filename,
      rcount, e->file_size);
      free(e->file_raw);
      free(e);
      return NULL;
    }
#endif

  fclose(f);
  return e;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libelf_close(elf32_t e)
{
  if (e)
    {
      if (e->file_raw)
	free(e->file_raw);

      if (e->elf_section)
	free(e->elf_section);

      if (e->elf_program)
	free(e->elf_program);

      free(e);
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define S2(v) v=libelf_swap2(v)
#define S4(v) v=libelf_swap4(v)

static void swap_elf32_header(elf32_t elf)
{
#if defined(WORDS_BIGENDIAN)
  if (elf->data == elf_data_lsb)
#else
  if (elf->data == elf_data_msb)
#endif
    {
      VERBOSE(VSYS,"libelf: swaping byte order for elf header\n");
      S2(elf->elf_header.type);
      S2(elf->elf_header.machine);

      S4(elf->elf_header.version);
      S4(elf->elf_header.entry);
      S4(elf->elf_header.phoff);
      S4(elf->elf_header.shoff);
      S4(elf->elf_header.flags);

      S2(elf->elf_header.ehsize);
      S2(elf->elf_header.phentsize);
      S2(elf->elf_header.phnum);
      S2(elf->elf_header.shentsize);
      S2(elf->elf_header.shnum);
      S2(elf->elf_header.shstrndx);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void swap_elf32_section_header(elf32_t elf, int i)
{
  elf32_sh_t *s = &(elf->elf_section[i]);
#if defined(WORDS_BIGENDIAN)
  if (elf->data == elf_data_lsb)
#else
  if (elf->data == elf_data_msb)
#endif
    {
      VERBOSE(VSYS,"libelf:shdr: swaping byte order for elf section header\n");
      S4(s->sh_name);
      S4(s->sh_type);
      S4(s->sh_flags);
      S4(s->sh_addr);
      S4(s->sh_offset);
      S4(s->sh_size);
      S4(s->sh_link);
      S4(s->sh_info);
      S4(s->sh_addralign);
      S4(s->sh_entsize);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void swap_elf32_program_header(elf32_t elf, int i)
{
  elf32_ph_t *p = &(elf->elf_program[i]);
#if defined(WORDS_BIGENDIAN)
  if (elf->data == elf_data_lsb)
#else
  if (elf->data == elf_data_msb)
#endif
    {
      S4(p->p_type);
      S4(p->p_offset);
      S4(p->p_vaddr);
      S4(p->p_paddr);
      S4(p->p_filesz);
      S4(p->p_memsz);
      S4(p->p_flags);
      S4(p->p_align);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int libelf_read_elf_header(elf32_t elf)
{
  int i;
  memcpy(&(elf->elf_header),elf->file_raw,sizeof(elf32_header_t));

  /* ident */
  VERBOSE(VELF,"\nlibelf:hdr: Elf file header\n");
  VERBOSE(VELF,"libelf:hdr: ident ");
  for(i=0; i<ELF32_NIDENT; i++)
    {
      VERBOSE(VELF,"%02x ",elf->elf_header.ident[i]); 
    }
  VERBOSE(VELF,"\n");

  /* class */
  elf->class = elf->elf_header.ident[ELF_IDENT_CLASS];
  VERBOSE(VELF,"libelf:hdr:id: class       %3d ",elf->class);
  switch (elf->class)
    {
    case elf_class_none: VERBOSE(VELF,"None"); break;
    case elf_class_32:   VERBOSE(VELF,"ELF32"); break;
    case elf_class_64:   VERBOSE(VELF,"ELF64"); break;
    }
  VERBOSE(VELF,"\n");

  /* data */
  elf->data = elf->elf_header.ident[ELF_IDENT_DATA];
  VERBOSE(VELF,"libelf:hdr:id: data        %3d ",elf->data);
  switch (elf->data)
    {
    case elf_data_none: VERBOSE(VELF,"None"); break;
    case elf_data_lsb:  VERBOSE(VELF,"2's LSB"); break;
    case elf_data_msb:  VERBOSE(VELF,"2's MSB"); break;
    }
  VERBOSE(VELF,"\n");

  /* other */
  VERBOSE(VELF,"libelf:hdr:id: version     %3d\n",elf->elf_header.ident[ELF_IDENT_VERSION]);

  /* abi */
  VERBOSE(VELF,"libelf:hdr:id: abi type    %3d ",elf->elf_header.ident[ELF_IDENT_ABI_TYPE]);
  switch (elf->elf_header.ident[ELF_IDENT_ABI_TYPE])
    {
    case elf_abi_none:       VERBOSE(VELF,"None"); break;
    case elf_abi_hpux:       VERBOSE(VELF,"HP-UX"); break;
    case elf_abi_netbsd:     VERBOSE(VELF,"NetBSD"); break;
    case elf_abi_linux:      VERBOSE(VELF,"Linux"); break;
    case elf_abi_solaris:    VERBOSE(VELF,"Solaris"); break;
    case elf_abi_aix:        VERBOSE(VELF,"Aix"); break;
    case elf_abi_irix:       VERBOSE(VELF,"Irix"); break;
    case elf_abi_freebsd:    VERBOSE(VELF,"FreeBSD"); break;
    case elf_abi_openbsd:    VERBOSE(VELF,"OpenBSD"); break;
    case elf_abi_standalone: VERBOSE(VELF,"Standalone"); break;
    default: VERBOSE(VELF,"Uknown"); break;
    }
  VERBOSE(VELF,"\n");

  VERBOSE(VELF,"libelf:hdr:id: abi version %3d\n",elf->elf_header.ident[ELF_IDENT_ABI_VERSION]);
  VERBOSE(VELF,"libelf:hdr:id: pad         %3d\n",elf->elf_header.ident[ELF_IDENT_PAD]);

  /* ==================================================== */
  /* starting from here we have to take care of endianess */
  /* ==================================================== */
  swap_elf32_header(elf);

  /* type */
  elf->type = elf->elf_header.type;
  VERBOSE(VELF,"libelf:hdr: type    %d ",elf->type);
  switch (elf->type)
    {
    case elf_type_none: VERBOSE(VELF,"None"); break;
    case elf_type_rel : VERBOSE(VELF,"Relocatable"); break;
    case elf_type_exec: VERBOSE(VELF,"Executable"); break;
    case elf_type_dyn : VERBOSE(VELF,"Shared Object"); break;
    case elf_type_core: VERBOSE(VELF,"Core"); break;
    default: VERBOSE(VELF,"Unknown"); break;
    }
  VERBOSE(VELF,"\n");

  /* machine in elf is called arch in libelf      */
  /* elf->mach is a specific architecture version */
  /* this corresponds to the gnu bfd convention   */
  elf->arch = elf->elf_header.machine;
  VERBOSE(VELF,"libelf:hdr: machine 0x%x (%d)\n",elf->elf_header.machine,elf->mach);
  VERBOSE(VELF,"libelf:hdr: version 0x%x\n",     elf->elf_header.version);
  VERBOSE(VELF,"libelf:hdr: entry   0x%x\n",     elf->elf_header.entry);
  VERBOSE(VELF,"libelf:hdr: flags   0x%08x\n",   elf->elf_header.flags);

  VERBOSE(VELF,"libelf:hdr: start of program headers %d\n",elf->elf_header.phoff);
  VERBOSE(VELF,"libelf:hdr: start of section headers %d\n",elf->elf_header.shoff);

  VERBOSE(VELF,"libelf:hdr: size of this header      %d\n",elf->elf_header.ehsize);
  VERBOSE(VELF,"libelf:hdr: size of program headers  %d\n",elf->elf_header.phentsize);
  VERBOSE(VELF,"libelf:hdr: number of program header %d\n",elf->elf_header.phnum);
  VERBOSE(VELF,"libelf:hdr: size of section header   %d\n",elf->elf_header.shentsize);
  VERBOSE(VELF,"libelf:hdr: number of section header %d\n",elf->elf_header.shnum);
  VERBOSE(VELF,"libelf:hdr: section str table index  %d\n",elf->elf_header.shstrndx);

  /*****************************************/
  /* machine specific part                 */
  /*****************************************/
#define MACH_FLAGS  0x7f 
  elf->mach = elf->elf_header.flags & MACH_FLAGS;

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum elf32_sh_type_enum_t {
  elf_sh_type_NULL      =  0, /* inactive                        */
  elf_sh_type_PROGBITS  =  1, /* defined by the program          */
  elf_sh_type_SYMTAB    =  2, /* symbol table                    */
  elf_sh_type_STRTAB    =  3, /* string table                    */
  elf_sh_type_RELA      =  4, /* relocation entries              */
  elf_sh_type_HASH      =  5, /* symbol hash table               */
  elf_sh_type_DYNAMIC   =  6, /* information for dynamic linking */
  elf_sh_type_NOTE      =  7, /* notes ?                         */
  elf_sh_type_NOBITS    =  8, /* empty section                   */
  elf_sh_type_REL       =  9, /* relocation entries              */
  elf_sh_type_SHLIB     = 10, /* unspecified semantics           */
  elf_sh_type_DYNSYM    = 11  /* symbol table                    */
};
typedef enum elf32_sh_type_enum_t elf32_sh_type_t;

char* elf_sh_str_type[] = 
{
  "NULL",
  "PROGBITS",
  "SYMTAB",
  "STRTAB",
  "RELA",
  "HASH",
  "DYNAMIC",
  "NOTE",
  "NOBITS",
  "REL",
  "SHLIB",
  "DYNSYM"  
};

enum elf32_sh_flags_enum_t {
  elf_sh_flags_WRITE     = 1, /* data that should be writable during execution */
  elf_sh_flags_ALLOC     = 2, /* occupies memory during execution              */
  elf_sh_flags_EXECINSTR = 4, /* executable machine instruction                */
  elf_sh_flags_MASKPROC  = 0xf0000000 /* processor specific semantics          */
};
typedef enum elf32_sh_flags_enum_t elf32_sh_flags_t;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libelf_find_section_by_name(elf32_t elf, char* name);
int libelf_find_section_by_type(elf32_t elf, elf32_sh_type_t type);

char* libelf_get_elf_section_name(elf32_t elf, int i)
{
  return (char*)&( elf->file_raw[ elf->elf_section[ elf->elf_header.shstrndx ].sh_offset + elf->elf_section[ i ].sh_name ]);
}

static int libelf_read_elf_section_headers(elf32_t elf)
{
  int i;
  char buff[100];

  elf->elf_section = (elf32_sh_t*) malloc(elf->elf_header.shnum * sizeof(elf32_sh_t));
  memset(elf->elf_section,0,elf->elf_header.shnum * sizeof(elf32_sh_t));
  for(i=0; i<elf->elf_header.shnum; i++)
    {
      memcpy(&(elf->elf_section[i]),
	     elf->file_raw + elf->elf_header.shoff + i * elf->elf_header.shentsize,
	     elf->elf_header.shentsize);
      swap_elf32_section_header(elf,i);
    }

  VERBOSE(VELF,"\nlibelf:sh: Section Headers:\n");
  VERBOSE(VELF,"libelf:sh: [Nr] Name           Type     Flg addr   offset size lnk info al es\n");
  for(i=0; i<elf->elf_header.shnum; i++)
    {
      VERBOSE(VELF,"libelf:sh: [%2d] ",i);
      if (elf->elf_header.shstrndx)
	{
	  strncpy(buff, libelf_get_elf_section_name(elf,i), 100);
	}
      else
	{
	  sprintf(buff,"%2ud",(unsigned) elf->elf_section[i].sh_name);
	}
      VERBOSE(VELF,"%-14s ",buff);

      if (elf->elf_section[i].sh_type <= elf_sh_type_DYNSYM)
	{
	  VERBOSE(VELF,"%-8s ",elf_sh_str_type[elf->elf_section[i].sh_type]);
	}
      else
	{
	  VERBOSE(VELF,"%-08x    ",elf->elf_section[i].sh_type);
	}

      /* flags */
      VERBOSE(VELF,"%s",(elf->elf_section[i].sh_flags & elf_sh_flags_WRITE)?"W":" ");
      VERBOSE(VELF,"%s",(elf->elf_section[i].sh_flags & elf_sh_flags_ALLOC)?"A":" ");
      VERBOSE(VELF,"%s",(elf->elf_section[i].sh_flags & elf_sh_flags_EXECINSTR)?"X":" ");

      VERBOSE(VELF," %06x ",elf->elf_section[i].sh_addr);
      VERBOSE(VELF,"%06x ",elf->elf_section[i].sh_offset);
      VERBOSE(VELF,"%04x ",elf->elf_section[i].sh_size);
      VERBOSE(VELF," %2d ",elf->elf_section[i].sh_link);
      VERBOSE(VELF," %3d ",elf->elf_section[i].sh_info);
      VERBOSE(VELF,"%2x ",elf->elf_section[i].sh_addralign);
      VERBOSE(VELF,"%2x ",elf->elf_section[i].sh_entsize);
      VERBOSE(VELF,"\n");

      libelf_set_section_info(VELF, i, buff,
			      elf->elf_section[i].sh_addr,
			      elf->elf_section[i].sh_offset,
			      elf->elf_section[i].sh_size);

    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum elf32_ph_type_enum_t {
  elf_ph_type_NULL      = 0, /* inactive                                */
  elf_ph_type_LOAD      = 1, /* loadable segment using filesz and memsz */
  elf_ph_type_DYNAMIC   = 2, /* dynamic linking information             */
  elf_ph_type_INTERP    = 3, /* location of interpreter                 */
  elf_ph_type_NOTE      = 4, /* note                                    */
  elf_ph_type_SHLIB     = 5, /* unspecified semantics                   */
  elf_ph_type_PHDR      = 6  /* location and size of the program header */
};
typedef enum elf32_ph_type_enum_t elf32_ph_enum_t;

char* elf_ph_str_type[] = 
{
  "NULL",
  "LOAD",
  "DYNAMIC",
  "INTERP",
  "NOTE",
  "SHLIB",
  "PHDR"
};

enum elf32_ph_flags_enum_t {
  elf_ph_flags_EXEC   = 1,
  elf_ph_flags_WRITE  = 2,
  elf_ph_flags_READ   = 4
};
typedef enum elf32_ph_flags_enum_t elf32_ph_flags_t;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int libelf_read_elf_program_headers(elf32_t elf)
{
  int i;
  char buff[100];
  
  elf->elf_program = (elf32_ph_t*)malloc(elf->elf_header.phnum * sizeof(elf32_ph_t));
  memset(elf->elf_program,0,elf->elf_header.phnum * sizeof(elf32_ph_t));
  for(i=0; i<elf->elf_header.phnum; i++)
    {
      memcpy(&(elf->elf_program[i]),
	     elf->file_raw + elf->elf_header.phoff + i * elf->elf_header.phentsize,
	     elf->elf_header.phentsize);
      swap_elf32_program_header(elf,i);
    }

  VERBOSE(VELF,"\nlibelf:ph: Program Headers:\n");
  VERBOSE(VELF,"libelf:ph: [Nr] Type     Offset Vaddr  Paddr  Filesz Memsz  Flg Al\n");
  for(i=0; i<elf->elf_header.phnum; i++)
    {
      VERBOSE(VELF,"libelf:ph: [%2d] ",i);

      if (elf->elf_program[i].p_type <= elf_ph_type_PHDR)
	{
	  strncpy(buff,elf_ph_str_type[elf->elf_program[i].p_type],100);
	}
      else
	{
	  sprintf(buff,"%ud",(unsigned) elf->elf_program[i].p_type);
	}

      VERBOSE(VELF,"%-8s ",buff);
      VERBOSE(VELF,"%06x ",elf->elf_program[i].p_offset);
      VERBOSE(VELF,"%06x ",elf->elf_program[i].p_vaddr);
      VERBOSE(VELF,"%06x ",elf->elf_program[i].p_paddr);
      VERBOSE(VELF,"%06x ",elf->elf_program[i].p_filesz);
      VERBOSE(VELF,"%06x ",elf->elf_program[i].p_memsz);
      
      /* flags */
      VERBOSE(VELF,"%s",(elf->elf_program[i].p_flags & elf_ph_flags_READ )?"R":" ");
      VERBOSE(VELF,"%s",(elf->elf_program[i].p_flags & elf_ph_flags_WRITE)?"W":" ");
      VERBOSE(VELF,"%s",(elf->elf_program[i].p_flags & elf_ph_flags_EXEC )?"E":" ");

      VERBOSE(VELF," %2x ",elf->elf_program[i].p_align);

      VERBOSE(VELF,"\n");
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct elf32_symtab_struct_t
{
  elf32_word_t  st_name;                /* Symbol name (string tbl index) */
  elf32_addr_t  st_value;               /* Symbol value */
  elf32_word_t  st_size;                /* Symbol size */
  unsigned char st_info;                /* Symbol type and binding [] */
  unsigned char st_other;               /* Symbol visibility */
  elf32_half_t  st_shndx;               /* Section index */
};
typedef struct elf32_symtab_struct_t elf32_symtab_t;

#define STT_NOTYPE      0               /* Symbol type is unspecified */
#define STT_OBJECT      1               /* Symbol is a data object */
#define STT_FUNC        2               /* Symbol is a code object */
#define STT_SECTION     3               /* Symbol associated with a section */
#define STT_FILE        4               /* Symbol's name is file name */
#define STT_COMMON      5               /* Symbol is a common data object */
#define STT_TLS         6               /* Symbol is thread-local data object*/
#define STT_NUM         7               /* Number of defined types.  */
#define STT_LOOS        10              /* Start of OS-specific */
#define STT_HIOS        12              /* End of OS-specific */
#define STT_LOPROC      13              /* Start of processor-specific */
#define STT_HIPROC      15              /* End of processor-specific */

/* How to extract and insert information held in the st_info field.  */

#define ELF32_ST_BIND(val)              (((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)              ((val) & 0xf)
#define ELF32_ST_INFO(bind, type)       (((bind) << 4) + ((type) & 0xf))

char* sttype_chr(int n)
{
  switch (n)
    {
    case STT_NOTYPE  : return "NOTYPE";
    case STT_OBJECT  : return "OBJECT";
    case STT_FUNC    : return "FUNC";
    case STT_SECTION : return "SECTION";
    case STT_FILE    : return "FILE";
    case STT_COMMON  : return "COMMON";
    case STT_TLS     : return "TLS";
    case STT_NUM     : return "NUM";
    case STT_LOOS    : return "LOOS";
    case STT_HIOS    : return "HIOS";
    case STT_LOPROC  : return "LOPROC";
    case STT_HIPROC  : return "HIPROC";
    }
  return "unknown";
}

#define STB_LOCAL       0               /* Local symbol */
#define STB_GLOBAL      1               /* Global symbol */
#define STB_WEAK        2               /* Weak symbol */
#define STB_NUM         3               /* Number of defined types.  */
#define STB_LOOS        10              /* Start of OS-specific */
#define STB_HIOS        12              /* End of OS-specific */
#define STB_LOPROC      13              /* Start of processor-specific */
#define STB_HIPROC      15              /* End of processor-specific */

char* stbind_chr(int n)
{
  switch (n)
    {
    case STB_LOCAL  : return "LOCAL";
    case STB_GLOBAL : return "GLOBAL";
    case STB_WEAK   : return "WEAK";
    case STB_NUM    : return "NUM";
    case STB_LOOS   : return "LOOS";
    case STB_HIOS   : return "HIOS";
    case STB_LOPROC : return "LOPROC";
    case STB_HIPROC : return "HIPROC";
    }
  return "unknown";
}

#define STV_DEFAULT     0               /* Default symbol visibility rules */
#define STV_INTERNAL    1               /* Processor specific hidden class */
#define STV_HIDDEN      2               /* Sym unavailable in other modules */
#define STV_PROTECTED   3               /* Not preemptible, not exported */

#define ELF32_ST_VISIBILITY(o)  ((o) & 0x03)

char* stvis_chr(int n)
{
  switch (n)
    {
    case STV_DEFAULT  : return "DEFAULT";
    case STV_INTERNAL : return "INTERNAL";
    case STV_HIDDEN   : return "HIDDEN";
    case STV_PROTECTED: return "PROTECTED";
    }
  return "unknown";
}

#define SHN_UNDEF       0               /* Undefined section */
#define SHN_LORESERVE   0xff00          /* Start of reserved indices */
#define SHN_LOPROC      0xff00          /* Start of processor-specific */
#define SHN_BEFORE      0xff00          /* Order section before all others (Solaris).  */
#define SHN_AFTER       0xff01          /* Order section after all others (Solaris).  */
#define SHN_HIPROC      0xff1f          /* End of processor-specific */
#define SHN_LOOS        0xff20          /* Start of OS-specific */
#define SHN_HIOS        0xff3f          /* End of OS-specific */
#define SHN_ABS         0xfff1          /* Associated symbol is absolute */
#define SHN_COMMON      0xfff2          /* Associated symbol is common */
#define SHN_XINDEX      0xffff          /* Index is in extra table.  */
#define SHN_HIRESERVE   0xffff          /* End of reserved indices */

char* stshn_chr(int n)
{
# define BUF_MAX 200
  static char numvalue[BUF_MAX];
  switch (n)
    {
    case SHN_UNDEF     : return "UND";
    case SHN_LORESERVE : return "LORESERVE";
      //case SHN_LOPROC    : return "LOPROC";
      //case SHN_BEFORE    : return "BEFORE";
    case SHN_AFTER     : return "AFTER";
    case SHN_HIPROC    : return "HIPROC";
    case SHN_LOOS      : return "LOOS";
    case SHN_HIOS      : return "HIOS";
    case SHN_ABS       : return "ABS";
    case SHN_COMMON    : return "COMMON";
    case SHN_XINDEX    : return "XINDEX";
      //case SHN_HIRESERVE : return "HIRESERVE";
    }
  snprintf(numvalue,BUF_MAX,"%d",n);
  return numvalue;
}

static void libelf_read_symtab(elf32_t elf, int n, int UNUSED verbose_level)
{
  uint32_t i;
  int size             = sizeof(elf32_symtab_t);
  elf32_sh_t     *s    = &(elf->elf_section[n]);

  VERBOSE(VELF,"libelf: looking at section %s\n",libelf_get_elf_section_name(elf,n));
  VERBOSE(VELF,"libelf: offset 0x%x size 0x%x (%d), sizeof(struct) = %d, num entries %d\n", 
	  s->sh_offset, s->sh_size, s->sh_size, size, s->sh_size / size);

  if (s->sh_size % size != 0)
    {
      VERBOSE(VELF,"libelf: symtab read error, section size is not a multiple of struct size\n");
      return;
    }

  int             strtab_n = libelf_find_section_by_name(elf,".strtab");
  elf32_sh_t     *strtab_s = &(elf->elf_section[ strtab_n ]);
  char*           strtab_p = ((char*)elf->file_raw) + strtab_s->sh_offset;

  //VERBOSE(VELF,"libelf: strtab == section number %d (%x+%x = %x)\n", strtab_n, elf->file_raw, strtab_s->sh_offset, strtab_p );
  //libelf_dump_section((uint8_t*)strtab_p, 0, 64, 8);
  //VERBOSE(VELF,"libelf: strtab == section number %d\n", strtab_n);

  VERBOSE(VELF,"libelf:   Num:    Value  Size Type    Bind   Vis      Ndx Name\n");

  for(i=0; i < (s->sh_size / size); i++)
    {
      uint8_t *section_start = ((uint8_t*)elf->file_raw) + s->sh_offset;
      elf32_symtab_t *stab = (elf32_symtab_t *)(section_start);

      VERBOSE(VELF,"libelf:   %3d: %08x   %3d %-7s %-6s %-8s %3s", 
	      i, stab[i].st_value, stab[i].st_size,
	      sttype_chr(ELF32_ST_TYPE(stab[i].st_info)),
	      stbind_chr(ELF32_ST_BIND(stab[i].st_info)),
	      stvis_chr (ELF32_ST_VISIBILITY(stab[i].st_other)),
	      stshn_chr (stab[i].st_shndx)
	      );

      //VERBOSE(VELF," 0x%08x", stab[i].st_name);

      if ( (stab[i].st_name > 0) && (stab[i].st_name < strtab_s->sh_size) )
	{
	  VERBOSE(VELF," %s", strtab_p + stab[i].st_name);
	}

      VERBOSE(VELF,"\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define MSP430_DATA_INIT_WORKAROUND 1

static void libelf_load_section(elf32_t elf, int n, int UNUSED verbose_level)
{
  elf32_sh_t *s = &(elf->elf_section[n]);
#if defined(MSP430_DATA_INIT_WORKAROUND)
  static uint16_t text_end    = 0; /* section text end in ram adress */
#endif

  switch (s->sh_type)
    {
    case elf_sh_type_PROGBITS:
      {
	if (s->sh_flags & elf_sh_flags_ALLOC)
	  {
	    VERBOSE(VELF,"libelf: looking at section %s\n",libelf_get_elf_section_name(elf,n));
	    VERBOSE(VELF,"libelf:    - allocating %d bytes at 0x%04x\n",s->sh_size,s->sh_addr);
	    VERBOSE(VELF,"libelf:    - copying 0x%04x bytes from file to 0x%04x in mem\n",s->sh_size,s->sh_addr);
	    mcu_jtag_write_section(elf->file_raw + s->sh_offset, s->sh_addr, s->sh_size);
	    if (verbose_level >= VDMP)
	      {
		VERBOSE(VDMP,"\n");
		libelf_dump_section(elf->file_raw, s->sh_offset, s->sh_size, 8);
	      }
	    
#if defined(MSP430_DATA_INIT_WORKAROUND)
	    if (strcmp(mcu_name(),"msp430") == 0)
	      {
		if (strcmp(libelf_get_elf_section_name(elf,n),".text") == 0)
		  {
		    text_end    = s->sh_addr + s->sh_size;
		  }
		
		if (strcmp(libelf_get_elf_section_name(elf,n),".data") == 0)
		  {
		    if (text_end == 0)
		      {
			VERBOSE(VELF,"libelf: msp430 init data : section text is missing\n");
		      }
		    VERBOSE(VELF,"libelf: msp430 init data section workaround: copy %d data bytes at 0x%04x\n",s->sh_size,text_end);
		    mcu_jtag_write_section(elf->file_raw + s->sh_offset, text_end, s->sh_size);
		  }
	      }
#endif
	    VERBOSE(VELF,"\n");
	  }
      }
      break;
    case elf_sh_type_SYMTAB:
      VERBOSE(VELF,"libelf: looking at section %-12s -- symbol table\n",libelf_get_elf_section_name(elf,n));
      libelf_read_symtab(elf,n,verbose_level);
      break;
    default:
      VERBOSE(VELF,"libelf: looking at section %-12s (%d) -- not loaded\n",libelf_get_elf_section_name(elf,n), n);
      if (verbose_level >= VDMP)
	{
	  VERBOSE(VDMP,"\n");
	  libelf_dump_section(elf->file_raw, s->sh_offset, s->sh_size, 8);
	}
      break;
    }

#if defined(INITIALIZE_ALLOCATED_SECTION)
  if (s->sh_type == elf_sh_type_NOBITS)
    {
      if (s->sh_flags & elf_sh_flags_ALLOC)
	{
	  VERBOSE(VELF,"libelf: looking at section %s\n",libelf_get_elf_section_name(elf,n));
	  VERBOSE(VELF,"libelf:    - allocating %d bytes at 0x%04x\n",s->sh_size,s->sh_addr);
	  VERBOSE(VELF,"libelf:    - zeroing bits\n");
	  mcu_jtag_write_zero(s->sh_addr,s->sh_size);
	}
    }
#endif
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void libelf_map_over_section(elf32_t elf, 
			     void (*f)(elf32_t elf, int n, int verbose_level), 
			     int verbose_level)
{
  int i;
  for(i=0; i < elf->elf_header.shnum; i++)
    {
      (*f)(elf,i,verbose_level);
    }
}

int libelf_find_section_by_name(elf32_t elf, char* name)
{
  int i;
  for(i=0; i < elf->elf_header.shnum; i++)
    {
      if (strcmp(libelf_get_elf_section_name(elf,i),name) == 0)
	{
	  return i;
	}
    }
  return -1;
}

int libelf_find_section_by_type(elf32_t elf, elf32_sh_type_t type)
{
  int i;
  for(i=0; i < elf->elf_header.shnum; i++)
    {
      if (elf->elf_section[i].sh_type == type)
	{
	  return i;
	}
    }
  return -1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define CHECK(test,msg)                                               \
if ( test )                                                           \
{                                                                     \
   ERROR("%s\n", msg );                                               \
   return 1;                                                          \
}

/* ************************************************** */
/* ** ENTRY POINT *********************************** */
/* ************************************************** */

int libelf_load_exec_code(const char* filename, int verbose_level)
{
  elf32_t elf;

  if ((elf = libelf_open(filename)) == NULL)
    {
      ERROR("libelf: error while opening elf file [%s]\n",filename);
      return 1;
    }

  VERBOSE(VSYS,"libelf: opening elf file %s\n",filename);
  libelf_read_elf_header(elf);  

  /* check we are using elf32 */
  CHECK(elf->class != elf_class_32,"Elf class is not elf32");
  /* check we have an executable */
  CHECK(elf->elf_header.type != elf_type_exec,"Elf file is not executable");
  /* standalone application : disabled for atmega binaries */
  // CHECK(elf->elf_header.ident[ELF_IDENT_ABI_TYPE] != elf_abi_standalone,
  //           "Elf file is not a standalone application");

  libelf_read_elf_section_headers(elf);
  libelf_read_elf_program_headers(elf);

  /* check that the file type is ok */
  if (elf->arch != mcu_arch_id())
    {
      ERROR("===================================================================\n");
      ERROR("The software you are using is compiled for a different architecture\n");
      ERROR("Elf arch id : %d\n",elf->arch);
      ERROR("Simulator arch id : %d (%s)\n",mcu_arch_id(),mcu_name());
      ERROR("===================================================================\n");
      return 1;
    }
  else if (elf->mach != mcu_mach_id())
    {
      ERROR("===================================================================\n");
      ERROR("The software you are using is compiled for a different machine\n");
      ERROR("Elf machine id : %ld\n",elf->mach);
      ERROR("Simulator machine id : %d (%s)\n",mcu_mach_id(),mcu_modelname());
      ERROR("===================================================================\n");
      return 1;
    }

  VERBOSE(VELF,"\nlibelf: all tests passed\n");
  VERBOSE(VELF,"libelf:    arch %3d 0x%02x : %s\n",mcu_arch_id(),mcu_arch_id(),mcu_name());
  VERBOSE(VELF,"libelf:    mach %3d 0x%02x : %s\n",mcu_mach_id(),mcu_mach_id(),mcu_modelname());
  VERBOSE(VELF,"\n");

  /* 
   * ok, now load the file 
   * we use physical address and
   * for now we use only section
   */
  mcu_set_pc_next(elf->elf_header.entry);
  VERBOSE(VELF,"libelf: PC set to 0x%x\n",elf->elf_header.entry);
  libelf_map_over_section(elf,libelf_load_section,verbose_level);

  libelf_close(elf);
  VERBOSE(VELF,"\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
