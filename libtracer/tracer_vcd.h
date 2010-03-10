
/**
 *  \file   tracer_vcd.h
 *  \brief  Simulator activity tracer
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#ifndef WSIM_TRACER_VCD_H
#define WSIM_TRACER_VCD_H

void tracer_vcd_open        (char *filename);
void tracer_vcd_start       ();
void tracer_vcd_dump_data   ();
void tracer_vcd_finish      ();
void tracer_vcd_close       ();

#endif
