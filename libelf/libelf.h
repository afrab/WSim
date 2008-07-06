
/**
 *  \file   libelf.h
 *  \brief  Target Platform Machine Code Loader
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef LIBELF_H
#define LIBELF_H

#define DUMP_COLS                 8
#define DUMP_LINES_SECTION        8

void     libelf_dump_section      (uint8_t* data, uint32_t start, uint32_t size, int maxlines);

uint16_t libelf_swap2             (uint16_t v);
uint32_t libelf_swap4             (uint32_t v);

int      libelf_load_exec_code    (const char* filename, int verbose_level);
int      libelf_set_section_info  (int level, int i, char *name, int addr, int offset, int size);
int      libelf_get_section_offset(const char* name);

#endif /* LIBELF_H */
