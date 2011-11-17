/**
 *  \file   scp1000_i2c_dev.h
 *  \brief  SCP1000 sensor in I2C Software Mode
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef SCP1000_I2C_DEV_H
#define	SCP1000_I2C_DEV_H
/* shift and masks */
#define SCP1000_I2C_SCL_SHIFT   0
#define SCP1000_I2C_SDA_SHIFT   1
#define SCP1000_I2C_DRDY_SHIFT  2

#define SCP1000_I2C_SCL_MASK    (1 << SCP1000_I2C_SCL_SHIFT)
#define SCP1000_I2C_SDA_MASK    (1 << SCP1000_I2C_SDA_SHIFT)
#define SCP1000_I2C_DRDY_MASK   (1 << SCP1000_I2C_DRDY_SHIFT)

/* States */
#define SCP1000_I2C_FSM_READSTART_0     0
#define SCP1000_I2C_FSM_READSTART_1     1
#define SCP1000_I2C_FSM_READCHIPADDR    2
#define SCP1000_I2C_FSM_SENDACK_0       3
#define SCP1000_I2C_FSM_SENDACK_1       4
#define SCP1000_I2C_FSM_READREGADDR     5
#define SCP1000_I2C_FSM_READFIRST_0     6
#define SCP1000_I2C_FSM_READFIRST_1     7
#define SCP1000_I2C_FSM_WRITEREG        8
#define SCP1000_I2C_FSM_READSTOP_0      9
#define SCP1000_I2C_FSM_READSTOP_1      10
#define SCP1000_I2C_FSM_SENDDATA        12
#define SCP1000_I2C_FSM_READXACK        13
#define SCP1000_I2C_FSM_READNACK_0      14
#define SCP1000_I2C_FSM_READACK_0       15
#define SCP1000_I2C_FSM_SENDEND         16

/* data */
struct scp1000_i2c_t {
  uint8_t registers[0xFF]; /* registers                            */
  uint8_t SCL_last; /* last clock state                     */
  uint8_t SDA_last; /* last dataline state                  */
  uint8_t SCL; /* current clock state                  */
  uint8_t SDA; /* current SDA state                    */
  uint8_t address; /* Adress of the scp1000                */
  uint8_t state; /* current state of the i2c SM          */
  uint8_t state_pos; /* current position in SM               */
  uint8_t state_next; /* next state after ACK                 */

  uint8_t read_tmp; /* temporary read value                 */
  uint8_t read_chipaddr; /* read chip addr                       */
  uint8_t read_regaddr; /* read memory addr                     */
  uint8_t read_write; /* the read/write bit                   */

  uint8_t I2C_send; /* send on I2C bus?                     */
  uint8_t SDA_send; /* value to send on the I2C SDA line    */
  uint8_t DRDY_send; /* value to send on the DRDY pin        */

  uint8_t cycle_count; /* count update cycles for the DRDY val */
};

/* methods */
int scp1000_i2c_device_size();
int scp1000_i2c_device_create(int dev_num);
int scp1000_i2c_reset(int dev);
int scp1000_i2c_delete(int dev);
void scp1000_i2c_read(int dev, uint32_t *mask, uint32_t *value);
void scp1000_i2c_write(int dev, uint32_t mask, uint32_t value);
int scp1000_i2c_update(int dev);

#endif	/* SCP1000_I2C_DEV_H */

