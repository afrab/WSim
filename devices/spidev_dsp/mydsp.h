
/**
 *  \file   mydsp.h
 *  \brief  DSP device example
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#ifndef MYDSP_H
#define MYDSP_H

#define DSP_MEM_SIZE 200
struct dsp_internal_state_t
{
  /* current spi mode */
  uint8_t  dsp_mode;          /* boolean     */

  /* table for test */
  uint8_t  dsp_data[DSP_MEM_SIZE];
  int32_t  dsp_index;
  int32_t  dsp_index_max;
};

#define MYDSP_ACTIVE  1
#define MYDSP_PASSIVE 2

void mydsp_create  (struct dsp_internal_state_t *st);
void mydsp_reset   (struct dsp_internal_state_t *st);
void mydsp_delete  (struct dsp_internal_state_t *st);

void mydsp_mode    (struct dsp_internal_state_t *st, int mode);

void mydsp_write   (struct dsp_internal_state_t *st, uint32_t val);
int  mydsp_update  (struct dsp_internal_state_t *st, uint32_t *val, uint8_t proc);

#endif
