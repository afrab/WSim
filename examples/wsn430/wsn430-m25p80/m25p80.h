#ifndef M25P80_H
#define M25P80_H

#define M25P80_PAGE_SIZE              256
#define M25P80_PAGE_NUMBER            4096

/* sets all MCU ports on wsn430 */
void m25p80_active(void);

/* get elctronic signature */
unsigned char  m25p80_get_signature(void);

/* active + get_signature */
unsigned char  m25p80_init(void);

/* get state register */
unsigned char  m25p80_get_state(void);

/* deep power modes */
int  m25p80_wakeup      (void);
int  m25p80_power_down  (void);

/* erase */
int  m25p80_erase_sector(uint8_t index);
int  m25p80_erase_bulk  (void);

/* program and read */
int  m25p80_save_page(uint16_t index, uint8_t *buffer);
int  m25p80_load_page(uint16_t index, uint8_t *buffer);

#endif
