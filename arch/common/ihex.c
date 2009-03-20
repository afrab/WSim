
/**
 *  \file   ihex.c
 *  \brief  Intel HEX file loader
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <stdio.h>
#include "hardware.h"

#define MAXLINE 50

#define H1L 1
#define H2L 2

/* ************************************** */
/* ************************************** */
/* ************************************** */

/*
wsim-wsn430  --trace=wsim.trc --serial1_io=udp:localhost:6000:localhost:7000 \
  --mode=time --modearg=0s --precharge=wsim.trc --logfile=stdout --verbose=3 \
  wsn430-serial.elf --help
*/

void ihex_read_line(char line[MAXLINE])
{
  VERBOSE(H2L,"wsim:ihex:line: %s",line);
}

/* ************************************** */
/* ************************************** */
/* ************************************** */

int mcu_hexfile_load(char *filename)
{
  FILE *f;
  char buff[MAXLINE];

  VERBOSE(H1L,"wsim:precharge: loading hexfile %s\n",filename);
  //	  mcu_jtag_write_section(elf->file_raw + s->sh_offset, s->sh_addr, s->sh_size);
  //	  mcu_jtag_write_zero(s->sh_addr,s->sh_size);

  if ((f = fopen(filename,"rb")) == NULL)
    {
      perror("wsim:ihex");
      return 0;
    }

  while(fgets(buff,MAXLINE,f) != NULL)
    {
      ihex_read_line(buff);
    }

  fclose(f);
  return 1;
}

/* ************************************** */
/* ************************************** */
/* ************************************** */
