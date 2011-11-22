/**
 *  \file   scp1000_i2c_dev.c
 *  \brief  SCP1000 sensor in I2C Software Mode
 *  \author Bernhard Dick
 *  \date   2011
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/scp1000_i2c/scp1000_i2c_dev.h"
#include "src/options.h"

int scp1000_i2c_device_size()
{
  return sizeof (struct scp1000_i2c_t);
}

int scp1000_i2c_device_create(int dev_num)
{
  HW_DMSG_DEV("scp1000 : device create");
  machine.device[dev_num].reset = scp1000_i2c_reset;
  machine.device[dev_num].delete = scp1000_i2c_delete;

  machine.device[dev_num].update = scp1000_i2c_update;

  machine.device[dev_num].read = scp1000_i2c_read;
  machine.device[dev_num].write = scp1000_i2c_write;

  machine.device[dev_num].state_size = scp1000_i2c_device_size();

  machine.device[dev_num].name = "scp1000 device";

  struct scp1000_i2c_t *scp1000 = (struct scp1000_i2c_t*) machine.device[dev_num].data;
  scp1000->SCL_last = 0xff;
  scp1000->SDA_last = 0xff;
  scp1000->SCL = 0xff;
  scp1000->SDA = 0xff;
  scp1000->address = 0x11;
  scp1000->state = 0;
  scp1000->state_pos = 0;
  scp1000->state_next = SCP1000_I2C_FSM_READCHIPADDR;
  scp1000->read_tmp = 0;
  scp1000->read_chipaddr = 0;
  scp1000->read_regaddr = 0;
  scp1000->read_write = 0;
  scp1000->I2C_send = 0;
  scp1000->SDA_send = 0;
  scp1000->DRDY_send = 0;
  scp1000->cycle_count = 0;

  int i = 0;
  for (i = 0; i <= 0xff; i++) {
    scp1000->registers[i] = 0x00;
  }
  scp1000->registers[0x00] = 0x03;
  scp1000->registers[0x01] = 0x00;
  scp1000->registers[0x02] = 0x00;
  scp1000->registers[0x03] = 0x00;
  scp1000->registers[0x04] = 0x00;
  scp1000->registers[0x06] = 0x00;
  scp1000->registers[0x07] = 0x00;
  scp1000->registers[0x1F] = 0x05;

  scp1000->registers[0x7F] = 0x01;
  scp1000->registers[0x80] = 0x23;
  scp1000->registers[0x81] = 0x45;
  scp1000->registers[0x82] = 0x67;
  scp1000->registers[0x83] = 0x89;
  return 0;
}

int scp1000_i2c_reset(int UNUSED dev)
{
  HW_DMSG_DEV("scp1000 : device reset");
  return 0;
}

void scp1000_i2c_read(int dev, uint32_t* mask, uint32_t* value)
{
  struct scp1000_i2c_t *scp1000 = (struct scp1000_i2c_t*) machine.device[dev].data;
  if (scp1000->I2C_send == 1) {
    *mask |= SCP1000_I2C_SDA_MASK;
    *value |= (scp1000->SDA_send << SCP1000_I2C_SDA_SHIFT);
  }
  *mask |= SCP1000_I2C_DRDY_MASK;
  *value |= (scp1000->DRDY_send << SCP1000_I2C_DRDY_SHIFT);
  if (*mask != 0) {
    //HW_DMSG_DEV("scp1000 : device write to mcu [val=0x%04x,mask=0x%04x] \n", *value, *mask);
  }
}

void scp1000_i2c_write(int dev, uint32_t mask, uint32_t value)
{
  struct scp1000_i2c_t *scp1000 = (struct scp1000_i2c_t*) machine.device[dev].data;
  if ((mask & SCP1000_I2C_SCL_MASK) != 0) {
    scp1000->SCL = ((value & SCP1000_I2C_SCL_MASK) == SCP1000_I2C_SCL_MASK) ? 1 : 0;
  }
  if ((mask & SCP1000_I2C_SDA_MASK) != 0) {
    scp1000->SDA = ((value & SCP1000_I2C_SDA_MASK) == SCP1000_I2C_SDA_MASK) ? 1 : 0;
  }

  HW_DMSG_DEV("scp1000 : SDA=0x%02x SCL=0x%02x \n", scp1000->SDA, scp1000->SCL);
}

int scp1000_i2c_update(int dev)
{
  struct scp1000_i2c_t *scp1000 = (struct scp1000_i2c_t*) machine.device[dev].data;
  switch (scp1000->state) {

  case SCP1000_I2C_FSM_READSTART_0:
    if (scp1000->SCL == 1 && scp1000->SDA == 1) {
      scp1000->state = SCP1000_I2C_FSM_READSTART_1;
    }
    break;

  case SCP1000_I2C_FSM_READSTART_1:
    if (scp1000->SCL == 1 && scp1000->SDA == 0) {
      scp1000->state = scp1000->state_next;
    } else if (scp1000->SCL == 0) {
      scp1000->state = SCP1000_I2C_FSM_READSTART_0;
    }
    break;

  case SCP1000_I2C_FSM_READCHIPADDR:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1) {
      scp1000->read_tmp <<= 1;
      scp1000->read_tmp |= scp1000->SDA;
      scp1000->state_pos++;
      if (scp1000->state_pos == 8) {
        scp1000->read_chipaddr = (scp1000->read_tmp >> 1);
        scp1000->read_write = (scp1000->read_tmp & 0x01);
        scp1000->read_tmp = 0;
        scp1000->state_pos = 0;
        if (scp1000->read_write == 1) { //read
          scp1000->state_next = SCP1000_I2C_FSM_SENDDATA;
        } else { //write
          scp1000->state_next = SCP1000_I2C_FSM_READREGADDR;
        }
        scp1000->state = SCP1000_I2C_FSM_SENDACK_0;
        //HW_DMSG_DEV("scp1000 : switch FULLFILLED_READCHIPADDR address=0x%02x\n",scp1000->read_chipaddr);
      }
    }
    break;

  case SCP1000_I2C_FSM_SENDACK_0:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1) {
      scp1000->I2C_send = 1;
      scp1000->SDA_send = 0;
      scp1000->state = SCP1000_I2C_FSM_SENDACK_1;
    }
    break;

  case SCP1000_I2C_FSM_SENDACK_1:
    if (scp1000->SCL_last == 1 && scp1000->SCL == 0) {
      scp1000->I2C_send = 0;
      scp1000->SDA_send = 1;
      scp1000->state = scp1000->state_next;
    }
    break;

  case SCP1000_I2C_FSM_READREGADDR:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1) {
      scp1000->read_tmp <<= 1;
      scp1000->read_tmp |= scp1000->SDA;
      scp1000->state_pos++;
      if (scp1000->state_pos == 8) {
        scp1000->read_regaddr = scp1000->read_tmp;
        if (scp1000->read_regaddr == 0x81) {
          scp1000->read_regaddr = 0x82;
        }
        if (scp1000->read_regaddr == 0x80) { /* pressure read -> reset DRDY */
          scp1000->DRDY_send = 0;
          scp1000->cycle_count = 0;
        }
        scp1000->read_tmp = 0;
        scp1000->state_pos = 0;
        scp1000->state_next = SCP1000_I2C_FSM_READFIRST_0;
        scp1000->state = SCP1000_I2C_FSM_SENDACK_0;
        //HW_DMSG_DEV("scp1000 : switch FULLFILLED_READREGADDR address=0x%02x\n",scp1000->read_regaddr);
      }
    }
    break;

  case SCP1000_I2C_FSM_READFIRST_0:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1) {
      scp1000->state = SCP1000_I2C_FSM_READFIRST_1;
    }
    break;

  case SCP1000_I2C_FSM_READFIRST_1:
    if (scp1000->SCL == 0) {
      scp1000->read_tmp |= scp1000->SDA_last;
      scp1000->state_pos = 1;
      scp1000->state = SCP1000_I2C_FSM_WRITEREG;
    } else if (scp1000->SCL == 1 && scp1000->SDA == 0 && scp1000->SDA_last == 1) {
      scp1000->state_next = SCP1000_I2C_FSM_READCHIPADDR;
      scp1000->state = SCP1000_I2C_FSM_READSTART_1;
    }
    break;

  case SCP1000_I2C_FSM_WRITEREG:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1) {
      scp1000->read_tmp <<= 1;
      scp1000->read_tmp |= scp1000->SDA;
      scp1000->state_pos++;
      if (scp1000->state_pos == 8) {
        scp1000->registers[scp1000->read_regaddr] = scp1000->read_tmp;
        scp1000->read_tmp = 0;
        scp1000->state_pos = 0;
        scp1000->state_next = SCP1000_I2C_FSM_READSTOP_0;
        scp1000->state = SCP1000_I2C_FSM_SENDACK_0;
        //HW_DMSG_DEV("scp1000 : switch FULLFILLED_WRITEREG data=0x%02x\n",scp1000->registers[scp1000->read_regaddr]);
      }
    }
    break;

  case SCP1000_I2C_FSM_READSTOP_0:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1 && scp1000->SDA == 0) {
      scp1000->state = SCP1000_I2C_FSM_READSTOP_1;
    }
    break;

  case SCP1000_I2C_FSM_READSTOP_1:
    if (scp1000->SCL == 1 && scp1000->SDA == 1) {
      scp1000->state_next = SCP1000_I2C_FSM_READCHIPADDR;
      scp1000->state = SCP1000_I2C_FSM_READSTART_0;
      //HW_DMSG_DEV("scp1000 : switch FULLFILLED_READSTOP_1\n");
    }
    break;

  case SCP1000_I2C_FSM_SENDDATA:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1) {
      scp1000->I2C_send = 1;
      scp1000->SDA_send = ((scp1000->registers[scp1000->read_regaddr] >> (7 - scp1000->state_pos)) & 0x01);
      scp1000->state_pos++;
      if (scp1000->state_pos == 8) {
        scp1000->read_tmp = 0;
        scp1000->state_pos = 0;
        scp1000->state = SCP1000_I2C_FSM_SENDEND;
        //HW_DMSG_DEV("scp1000 : switch FULLFILLED_SENDDATA data=0x%02x\n",scp1000->registers[scp1000->read_regaddr]);
      }
    }
    break;

  case SCP1000_I2C_FSM_SENDEND:
    if (scp1000->SCL_last == 1 && scp1000->SCL == 0) {
      scp1000->I2C_send = 0;
      scp1000->SDA_send = 1;
      scp1000->state = SCP1000_I2C_FSM_READXACK;
    }
    break;

  case SCP1000_I2C_FSM_READXACK:
    if (scp1000->SCL_last == 0 && scp1000->SCL == 1) {
      if (scp1000->SDA == 1) {
        scp1000->state = SCP1000_I2C_FSM_READNACK_0;
      } else {
        scp1000->read_regaddr++;
        scp1000->state = SCP1000_I2C_FSM_READACK_0;
      }
    }
    break;

  case SCP1000_I2C_FSM_READNACK_0:
    if (scp1000->SCL_last == 1 && scp1000->SCL == 0) {
      scp1000->state_next = SCP1000_I2C_FSM_READCHIPADDR;
      scp1000->state = SCP1000_I2C_FSM_READSTOP_0;
    }
    break;

  case SCP1000_I2C_FSM_READACK_0:
    if (scp1000->SCL_last == 1 && scp1000->SCL == 0) {
      scp1000->state = SCP1000_I2C_FSM_SENDDATA;
    }
    break;

  default:
    ERROR("scp1000: STATE NOT IMPLEMENTED");
  }

  scp1000->SCL_last = scp1000->SCL;
  scp1000->SDA_last = scp1000->SDA;

  scp1000->cycle_count++;
  if (scp1000->cycle_count == 200000) { /* "ready" always 1000 cycles after pressure read */
    scp1000->DRDY_send = 1;
  }
  return 0;
}

int scp1000_i2c_delete(int UNUSED dev)
{
  return 0;
}
