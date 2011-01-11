
/**
 *  \file   libelf.h
 *  \brief  Target Platform Machine Code Loader
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef LIBELF_H
#define LIBELF_H

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define DUMP_COLS                  8
#define DUMP_LINES_SECTION         8

typedef struct elf32_struct_t *elf32_t;

uint16_t libelf_swap2              (uint16_t v);
uint32_t libelf_swap4              (uint32_t v);

elf32_t  libelf_load               (const char* filename, int verbose_level);
int      libelf_close              (elf32_t elf);
      
uint32_t libelf_symtab_find_addr_by_name(elf32_t elf, const char* name);
int      libelf_symtab_find_size_by_name(elf32_t elf, const char* name);


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int      libelf_set_section_info   (int i, char *name, int addr, int offset, int size);
void     libelf_dump_section       (uint8_t* data, uint32_t start, uint32_t size, int maxlines);
int      libelf_get_section_offset (const char* name);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif /* LIBELF_H */
