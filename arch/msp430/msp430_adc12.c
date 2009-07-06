/**
 *  \file   msp430_adc12.c
 *  \brief  MSP430 Adc12 controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include "arch/common/hardware.h"
#include "msp430.h"
#include "src/options.h"

#if defined(__msp430_have_adc12)


#if defined(DEBUG)
#define HW_DMSG_ADC12(x...) HW_DMSG(x)
#else 
#define HW_DMSG_ADC12(x...) do { } while (0)
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

tracer_id_t MSP430_TRACER_ADC12STATE;
tracer_id_t MSP430_TRACER_ADC12INPUT[ADC12_CHANNELS];
int         msp430_adc12_channels_valid[ADC12_CHANNELS];
char        msp430_adc12_channels_name[ADC12_CHANNELS][MAX_FILENAME];
uint16_t*   msp430_adc12_channels_data[ADC12_CHANNELS];
uint32_t    msp430_adc12_channels_data_max[ADC12_CHANNELS];

#define ADC12_CHANNEL_NAMES 20
char trace_names[ADC12_CHANNELS][ADC12_CHANNEL_NAMES] = {
  "adc12_input00", "adc12_input01", "adc12_input02", "adc12_input03",
  "adc12_input04", "adc12_input05", "adc12_input06", "adc12_input07",
  "adc12_input08", "adc12_input09", "adc12_input10", "adc12_input11",
  "adc12_input12", "adc12_input13", "adc12_input14", "adc12_input15"
};

#define ADC12_MODES        4
#define ADC12_MODES_NAMES 40
char adc12_modes[ADC12_MODES][ADC12_MODES_NAMES] = {
  "Single Channel Single Conversion",  "Sequence of Channels",
  "Repeat Single Channel",  "Repeat Sequence of Channels"
};

enum adc12_modes {
  adc12_mode_single,
  adc12_mode_seq_chan,
  adc12_mode_repeat_single,
  adc12_mode_repead_seq
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static struct moption_t adc12_in_opt = {
  .longname    = "msp430_adc12",
  .type        = required_argument,
  .helpstring  = "msp430 adc12 input",
  .value       = NULL
};

int msp430_adc12_option_add (void)
{
  options_add( &adc12_in_opt );
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_adc12_find_inputs()
{
  char delim1[] = ",";
  char delim2[] = ":";
  char *str1,*str2;
  char *token,*subtoken;
  char *saveptr1,*saveptr2;
  char *filename;
  char name[MAX_FILENAME];
  int  j;
  int id;
  strncpy(name, adc12_in_opt.value, MAX_FILENAME);

  /* --msp430_adc12=1:file,2:file,3:file ... */

  for (j = 1, str1 = name; ; j++, str1 = NULL) 
    {
      token = strtok_r(str1, delim1, &saveptr1);
      if (token == NULL)
	break;
      HW_DMSG_ADC12("msp430:adc12:%d: %s\n", j, token);
    
      str2 = token;
      subtoken = strtok_r(str2, delim2, &saveptr2);
      if (subtoken == NULL) 
	{ 
	  ERROR("msp430:adc12: wrong channel id \n");
	  return 1;	
	}
      id = atoi(subtoken);
      if ((id < 0) || (id >= ADC12_CHANNELS))
	{
	  ERROR("msp430:adc12: wrong channel id %s (%d)\n",subtoken,id);
	  return 1;
	}

      subtoken = strtok_r(NULL, delim2, &saveptr2);
      filename = subtoken;
      if (subtoken == NULL) 
	{
	  ERROR("msp430:adc12: wrong channel filename\n");
	  return 1;	
	}

      subtoken = strtok_r(NULL, delim2, &saveptr2);
      if (subtoken != NULL) 
	{
	  ERROR("msp430:adc12: wrong channel filename trailer %s\n",subtoken);
	  return 1;	
	}

      HW_DMSG_ADC12("msp430:adc12: channel %02d = %s\n",id, filename);
      msp430_adc12_channels_valid[id]    = 1;
      strncpy(msp430_adc12_channels_name[id], filename, MAX_FILENAME);
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_adc12_read_inputs()
{
  int i;
  uint32_t j;
  for(i=0; i < ADC12_CHANNELS; i++)
    {
      if (msp430_adc12_channels_valid[i])
	{
	  FILE* f;
	  int   val;

	  f = fopen(msp430_adc12_channels_name[i],"rb");
	  if (f==NULL)
	    {
	      ERROR("msp430:adc12: cannot open file %s\n",msp430_adc12_channels_name[i]);
	      return 1;
	    }
	  while(fscanf(f,"%d",&val) > 0)
	    {
	      msp430_adc12_channels_data_max[i] ++;
	    }
	  msp430_adc12_channels_data[i] = malloc(msp430_adc12_channels_data_max[i]*sizeof(uint16_t));
	  if (msp430_adc12_channels_data[i] == NULL)
	    {
	      ERROR("msp430:adc12: cannot allocate memory for input %s\n",msp430_adc12_channels_name[i]);
	      fclose(f);
	      return 1;
	    }

	  fseek(f,0,SEEK_SET);
	  for(j=0; j<msp430_adc12_channels_data_max[i]; j++)
	    {
	      fscanf(f,"%d",&val);
	      msp430_adc12_channels_data[i][j] = (uint16_t)val;
	    }

	  fclose(f);
	  HW_DMSG_ADC12("msp430:adc12: channel %02d filled with %d samples\n",
			i,msp430_adc12_channels_data_max[i]);
	}
    }
  return 0;
}

int msp430_adc12_delete_inputs()
{
  int i;
  for(i=0; i < ADC12_CHANNELS; i++)
    {
      if (msp430_adc12_channels_valid[i])
	{
	  free(msp430_adc12_channels_data[i]);
	  msp430_adc12_channels_data[i] = NULL;
	}
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_adc12_init(void)
{
  int i;

  for(i=0; i<ADC12_CHANNELS; i++)
    {
      MSP430_TRACER_ADC12INPUT[i]       = 0;
      msp430_adc12_channels_valid[i]    = 0;
      msp430_adc12_channels_data[i]     = NULL;
      msp430_adc12_channels_data_max[i] = 0;
      strcpy(msp430_adc12_channels_name[i], "none");

      MCU.adc12.chann_ptr[i] = 0;
      MCU.adc12.chann_time[i] = 0;
      MCU.adc12.chann_period[i] = 0;
    }

  if (adc12_in_opt.isset)
    {
      msp430_adc12_find_inputs();
      msp430_adc12_read_inputs();
    }

  MSP430_TRACER_ADC12STATE    = tracer_event_add_id(1,  "adc12_enable",   "msp430");
  for(i=0; i<ADC12_CHANNELS; i++)
    {
      if (msp430_adc12_channels_valid[i])
	{
	  MSP430_TRACER_ADC12INPUT[i] = tracer_event_add_id(16,  trace_names[i],   "msp430");
	}

      MCU.adc12.mem[i].s = 0;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_reset(void)
{
  int i;
  /* set initial values */
  HW_DMSG_ADC12("msp430:adc12:reset()\n");
  MCU.adc12.ctl0.s = 0;
  MCU.adc12.ctl1.s = 0;
  MCU.adc12.ifg    = 0;
  MCU.adc12.ie     = 0;
  MCU.adc12.iv     = 0;
  for(i=0; i<ADC12_CHANNELS; i++)
    {
      MCU.adc12.mctl[i].s = 0;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_update(void)
{
  //  HW_DMSG_ADC12("msp430:adc12:update\n");
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_adc12_read16(uint16_t addr)
{
  int16_t ret = 0;

  /*
  static int tinyos_read_this_too_many_times = 0;
  if (tinyos_read_this_too_many_times == 0)
    {
      HW_DMSG_ADC12("msp430:adc12: read16 at [0x%04x] \n",addr);
      tinyos_read_this_too_many_times = 1;
    }
  */

  switch (addr)
    {
    case ADC12CTL0    : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:read16: ADC12CTL0 = 0x%04x\n",MCU.adc12.ctl0.s);
      ret = MCU.adc12.ctl0.s;
      break;

    case ADC12CTL1    : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:read16: ADC12CTL1 = 0x%04x\n",MCU.adc12.ctl1.s);
      ret = MCU.adc12.ctl1.s;
      break;

    case ADC12IFG     : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:read16: ADC12IFG = 0x%04x\n",MCU.adc12.ifg);
      ret = MCU.adc12.ifg;
      break;

    case ADC12IE      : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:read16: ADC12IFG = 0x%04x\n",MCU.adc12.ie);
      ret = MCU.adc12.ie;
      break;

    case  ADC12IV     : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:read16: ADC12IV = 0x%04x\n",MCU.adc12.iv);
      ret = MCU.adc12.iv;
      break;

    case  ADC12MEM0   : /* 16 */
    case  ADC12MEM1   :
    case  ADC12MEM2   :
    case  ADC12MEM3   :
    case  ADC12MEM4   :
    case  ADC12MEM5   :
    case  ADC12MEM6   :
    case  ADC12MEM7   :
    case  ADC12MEM8   :
    case  ADC12MEM9   :
    case  ADC12MEM10  :
    case  ADC12MEM11  :
    case  ADC12MEM12  :
    case  ADC12MEM13  :
    case  ADC12MEM14  :
    case  ADC12MEM15  :
      HW_DMSG_ADC12("msp430:adc12:read16: ADC12MEM%d = 0x%04x\n",addr - ADC12MEM0, 
		    MCU.adc12.mem[addr - ADC12MEM0].s);
      ret = MCU.adc12.mem[addr - ADC12MEM0].s;
      break;

    case  ADC12MCTL0  : /*  8 */
    case  ADC12MCTL1  :
    case  ADC12MCTL2  :
    case  ADC12MCTL3  :
    case  ADC12MCTL4  :
    case  ADC12MCTL5  :
    case  ADC12MCTL6  :
    case  ADC12MCTL7  :
    case  ADC12MCTL8  :
    case  ADC12MCTL9  :
    case  ADC12MCTL10 :
    case  ADC12MCTL11 :
    case  ADC12MCTL12 :
    case  ADC12MCTL13 :
    case  ADC12MCTL14 :
    case  ADC12MCTL15 :
      HW_DMSG_ADC12("msp430:adc12:read16: ADC12MEMCTL%d = 0x%02x\n", addr - ADC12MCTL0,
		    MCU.adc12.mctl[addr - ADC12MCTL0].s);
      ret = MCU.adc12.mctl[addr - ADC12MCTL0].s & 0xff;
      break;

    default:
      HW_DMSG_ADC12("msp430:adc12:read16: at [0x%04x], unknown address\n",addr);
      break;
    }

  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_ctl0details(char* msg, int16_t *val)
{
  struct adc12ctl0_t *ctl0 = (struct adc12ctl0_t*) val;
  HW_DMSG_ADC12("msp430:adc12:%s sht1x      %d\n",msg,ctl0->sht1x);
  HW_DMSG_ADC12("msp430:adc12:%s sht0x      %d\n",msg,ctl0->sht0x);
  HW_DMSG_ADC12("msp430:adc12:%s msc        %d\n",msg,ctl0->msc);
  HW_DMSG_ADC12("msp430:adc12:%s ref2_5v    %d\n",msg,ctl0->ref2_5v);
  HW_DMSG_ADC12("msp430:adc12:%s refon      %d\n",msg,ctl0->refon);
  HW_DMSG_ADC12("msp430:adc12:%s adc12on    %d\n",msg,ctl0->adc12on);
  HW_DMSG_ADC12("msp430:adc12:%s adc12ovie  %d\n",msg,ctl0->adc12ovie);
  HW_DMSG_ADC12("msp430:adc12:%s adc12tovie %d\n",msg,ctl0->adc12tovie);
  HW_DMSG_ADC12("msp430:adc12:%s enc        %d\n",msg,ctl0->enc);
  HW_DMSG_ADC12("msp430:adc12:%s adc12sc    %d\n",msg,ctl0->adc12sc);
}

void msp430_adc12_ctl1details(char* msg, int16_t *val)
{
  struct adc12ctl1_t *ctl1 = (struct adc12ctl1_t*) val;
  HW_DMSG_ADC12("msp430:adc12:%s cstartaddx %d\n",msg,ctl1->cstartaddx);
  HW_DMSG_ADC12("msp430:adc12:%s shsx       %d\n",msg,ctl1->shsx);
  HW_DMSG_ADC12("msp430:adc12:%s shp        %d\n",msg,ctl1->shp);
  HW_DMSG_ADC12("msp430:adc12:%s issh       %d\n",msg,ctl1->issh);
  HW_DMSG_ADC12("msp430:adc12:%s adc12divx  %d\n",msg,ctl1->adc12divx);
  HW_DMSG_ADC12("msp430:adc12:%s adc12sselx %d\n",msg,ctl1->adc12sselx);
  HW_DMSG_ADC12("msp430:adc12:%s conseqx    %d\n",msg,ctl1->conseqx);
  HW_DMSG_ADC12("msp430:adc12:%s adc12busy  %d\n",msg,ctl1->adc12busy);
}

void msp430_adc12_write16(uint16_t addr, int16_t val)
{
  switch (addr)
    {
    case ADC12CTL0    : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:write16: ADC12CTL0 = 0x%04x\n",val);
      if (val != MCU.adc12.ctl0.s)
	{
	  msp430_adc12_ctl0details("    ctl0 set ",&val);
	}
      MCU.adc12.ctl0.s = val;
      break;

    case ADC12CTL1    : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:write16: ADC12CTL1 = 0x%04x\n",val);
      if (val != MCU.adc12.ctl1.s)
	{
	  msp430_adc12_ctl1details("    ctl1 set ",&val);
	}
      MCU.adc12.ctl1.s = val;
      break;

    case ADC12IFG     : /* 16 */
      if (MCU.adc12.ifg != val)
	{
	  HW_DMSG_ADC12("msp430:adc12:write16: ADC12IFG changed from 0x%04x to 0x%04x\n",MCU.adc12.ifg,val);
	}
      MCU.adc12.ifg = val;
      break;

    case ADC12IE      : /* 16 */
      if (MCU.adc12.ie != val)
	{
	  HW_DMSG_ADC12("msp430:adc12:write16: ADC12IE changed from 0x%04x to 0x%04x\n",MCU.adc12.ie,val);
	}
      MCU.adc12.ie = val;
      break;

    case  ADC12IV     : /* 16 */
      HW_DMSG_ADC12("msp430:adc12:write16: write to ADC12IV, read only register\n");
      break;

    case  ADC12MEM0   : /* 16 */
    case  ADC12MEM1   :
    case  ADC12MEM2   :
    case  ADC12MEM3   :
    case  ADC12MEM4   :
    case  ADC12MEM5   :
    case  ADC12MEM6   :
    case  ADC12MEM7   :
    case  ADC12MEM8   :
    case  ADC12MEM9   :
    case  ADC12MEM10  :
    case  ADC12MEM11  :
    case  ADC12MEM12  :
    case  ADC12MEM13  :
    case  ADC12MEM14  :
    case  ADC12MEM15  :
      HW_DMSG_ADC12("msp430:adc12:write16: ADC12MEM%d = 0x%04x\n",addr - ADC12MEM0, val);
      MCU.adc12.mem[addr - ADC12MEM0].b.value = val & 0x0FFF;
      break;

    case  ADC12MCTL0  : /*  8 */
    case  ADC12MCTL1  :
    case  ADC12MCTL2  :
    case  ADC12MCTL3  :
    case  ADC12MCTL4  :
    case  ADC12MCTL5  :
    case  ADC12MCTL6  :
    case  ADC12MCTL7  :
    case  ADC12MCTL8  :
    case  ADC12MCTL9  :
    case  ADC12MCTL10 :
    case  ADC12MCTL11 :
    case  ADC12MCTL12 :
    case  ADC12MCTL13 :
    case  ADC12MCTL14 :
    case  ADC12MCTL15 :
      HW_DMSG_ADC12("msp430:adc12:write16: write 16 bits value on 8 bits registers [0x%04x] = 0x%04x\n",
	    addr & 0xffff, val & 0xffff);
      break;

    default:
      HW_DMSG_ADC12("msp430:adc12:write16: at [0x%04x] = 0x%04x, unknown address\n",addr,val & 0xffff);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_adc12_read8(uint16_t addr)
{
  int8_t ret = 0;
  static int tinyos_read_this_too_many_times = 0;
  if (tinyos_read_this_too_many_times == 0)
    {
      HW_DMSG_ADC12("msp430:adc12: read8 at [0x%04x] \n",addr);
      tinyos_read_this_too_many_times = 1;
    }

  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_write8(uint16_t addr, int8_t val)
{
  HW_DMSG_ADC12("msp430:adc12: write8 at [0x%04x] = 0x%02x\n",addr,val & 0xff);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
