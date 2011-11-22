/**
 *  \file   msp430_adc.c
 *  \brief  MSP430 Adc10/12 Common
 *  \author Julien Carpentier
 *  \date   2011
 **/
 
#include <stdlib.h>
#include <string.h>
#include "arch/common/hardware.h"
#include "msp430.h"
#include "src/options.h"

#if defined(__msp430_have_adc12) || defined(__msp430_have_adc10)


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define HW_DMSG_ADC(x...) HW_DMSG_MCUDEV(x)

#define ADC_DEBUG_LEVEL_2 0

#if ADC_DEBUG_LEVEL_2 != 0
#define HW_DMSG_2_DBG(x...) HW_DMSG_ADC(x)
#else
#define HW_DMSG_2_DBG(x...) do { } while (0)
#endif


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_adc_init(struct adc_channels_t* channels, int UNUSED width, struct moption_t* opt)
{
  
  int i;
  for(i=0; i<ADC_CHANNELS; i++)
    {
      //MSP430_TRACER_ADC10INPUT[i]       = 0;
      channels->channels_valid[i]    = ADC_NONE;
      channels->channels_data[i]     = NULL;
      channels->channels_data_max[i] = 0;
      strcpy(channels->channels_name[i], "none");
      
      channels->chann_ptr[i]    = 0;
      channels->chann_time[i]   = 0;
      channels->chann_period[i] = 0;
    }
    
    if (opt->isset)
    {
      msp430_adc_find_inputs(channels, opt);
      msp430_adc_read_inputs(channels);
    }


  return 0;
}

int msp430_adc_find_inputs(struct adc_channels_t* channels, struct moption_t* opt)
{
  const char delim1[] = ",";
  const char delim2[] = ":";
  char *str1,*str2;
  char *token,*subtoken;
  char *saveptr1,*saveptr2;
  char *filename;
  char name[MAX_FILENAME];
  int  j;
  int id;
  strncpyz(name, opt->value, MAX_FILENAME);

  /* --msp430_adc=1:file,2:file,3:file ... */

  for (j = 1, str1 = name; ; j++, str1 = NULL) 
    {
      token = strtok_r(str1, delim1, &saveptr1);
      if (token == NULL)
	break;
      HW_DMSG_ADC("msp430:adc:%d: %s\n", j, token);
    
      str2 = token;
      subtoken = strtok_r(str2, delim2, &saveptr2);
      if (subtoken == NULL) 
	{ 
	  ERROR("msp430:adc: wrong channel id \n");
	  return 1;	
	}
      id = atoi(subtoken);
      if ((id < 0) || (id >= ADC_CHANNELS))
	{
	  ERROR("msp430:adc: wrong channel id %s (%d)\n",subtoken,id);
	  return 1;
	}

      subtoken = strtok_r(NULL, delim2, &saveptr2);
      filename = subtoken;
      if (subtoken == NULL) 
	{
	  ERROR("msp430:adc: wrong channel filename\n");
	  return 1;	
	}

      subtoken = strtok_r(NULL, delim2, &saveptr2);
      if (subtoken != NULL) 
	{
	  ERROR("msp430:adc: wrong channel filename trailer %s\n",subtoken);
	  return 1;	
	}

      HW_DMSG_ADC("msp430:adc: channel %02d = %s\n",id, filename);
      channels->channels_valid[id]    = ADC_CHANN_PTR;
      strncpyz(channels->channels_name[id], filename, MAX_FILENAME);
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_adc_read_inputs(struct adc_channels_t* channels)
{
  int chan;
  uint32_t smpl;
  for(chan=0; chan < ADC_CHANNELS; chan++)
    {
      if (channels->channels_valid[ chan ] == ADC_CHANN_PTR)
	{
	  FILE* f;
	  int   val;

	  /* size */
	  f = fopen(channels->channels_name[ chan ],"rb");
	  if (f==NULL)
	    {
	      ERROR("msp430:adc: cannot open file %s\n",channels->channels_name[ chan ]);
	      return 1;
	    }
	  while(fscanf(f,"%d",&val) > 0)
	    {
	      channels->channels_data_max[ chan ] ++;
	    }
	  channels->channels_data[ chan ] = malloc(channels->channels_data_max[ chan ]*sizeof(uint16_t));
	  if (channels->channels_data[ chan ] == NULL)
	    {
	      ERROR("msp430:adc: cannot allocate memory for input %s\n",channels->channels_name[ chan ]);
	      fclose(f);
	      return 1;
	    }

	  /* read */
	  fseek(f,0,SEEK_SET);
	  for(smpl=0; smpl < channels->channels_data_max[ chan ]; smpl++)
	    {
	      if (fscanf(f,"%d",&val) != 1)
		{
		  val = 0;
		  ERROR("msp430:adc: ======================================\n");
		  ERROR("msp430:adc: cannot read value from data input file\n");
		  ERROR("msp430:adc: ======================================\n");
		}
	      channels->channels_data[ chan ][ smpl ] = val & 0xfff; /* 12 bits */
	    }

	  fclose(f);
	  HW_DMSG_ADC("msp430:adc: channel %02d filled with %d samples\n",
			chan ,channels->channels_data_max[ chan ]);
	}
    }
  return 0;
}

int msp430_adc_delete_inputs(struct adc_channels_t* channels)
{
  int i;
  for(i=0; i < ADC_CHANNELS; i++)
    {
      if (channels->channels_valid[i] == ADC_CHANN_PTR)
	{
	  free(channels->channels_data[i]);
	  channels->channels_data[i] = NULL;
	}
    }
  return 0;
}

uint16_t msp430_adc_sample_input(struct adc_channels_t* channels, int hw_channel_x, int UNUSED current_x)
{
  uint16_t sample = 0;
  switch (channels->channels_valid[hw_channel_x])
    {
    case ADC_NONE:
      /* default mode */
      HW_DMSG_ADC("msp430:adc:     0xeaea sample for input channel %d\n", hw_channel_x);
      sample = 0xeaea;
      break;

    case ADC_CHANN_PTR:
      HW_DMSG_2_DBG("msp430:adc:     sample for input channel %d - %s\n",
		    hw_channel_x, channels->channels_name[hw_channel_x]);

      sample = channels->channels_data[ hw_channel_x ][ MCU.channels.chann_ptr[ hw_channel_x ] ] ;

      MCU.channels.chann_ptr[ hw_channel_x ] ++;
      if (MCU.channels.chann_ptr[ hw_channel_x ] == channels->channels_data_max[ hw_channel_x ])
	{
	  MCU.channels.chann_ptr[ hw_channel_x ] = 0;
	}
      break;

    case ADC_RND:
      HW_DMSG_ADC("msp430:adc:     random sample for input channel %d\n", hw_channel_x);
#if !defined(WIN32)
      sample = random();
#else
      sample = rand();
#endif
      break;

    case ADC_WSNET:
      break;
    }
  
  return sample & 0x0FFF; /* 12 bits */
}

#endif
