/**
 *  \file   ez430chronos.c
 *  \brief  Platform definition for ez430-chronos platform
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"

#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "devices/cma3000_spi/cma3000_spi_dev.h"
#include "devices/scp1000_i2c/scp1000_i2c_dev.h"
#include "devices/ez430_lcd/ez430_lcd_dev.h"
#include "src/options.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ****************************************
 * platform description for button devices
 *
 * Port 1
 * ======
 *   1.4 :
 *   1.3 :
 *   1.2 :
 *   1.1 :
 *   1.0 :
 *
 * Port 2
 * ======
 *   2.7 : BUZZER (?) -> LED
 *   2.6 :
 *   2.5 :
 *   2.4 : KEY_S1 (arrow up)
 *   2.3 : KEY_BL (light)
 *   2.2 : KEY_M1 (*)
 *   2.1 : KEY_M2 (#)
 *   2.0 : KEY_S2 (arrow down)
 *
 * Port 3
 * ======
 *   3.7 :
 *   3.6 :
 *   3.5 :
 *   3.4 :
 *   3.3 :
 *   3.2 :
 *   3.1 :
 *   3.0 :
 *
 * Port 4
 * ======
 *   4.7 :
 *   4.6 :
 *   4.5 :
 *   4.4 :
 *   4.3 :
 *   4.2 :
 *   4.1 :
 *   4.0 :
 *
 * Extra LCD (connected through memory)
 * ==============================
 * ***************************************/

/* ****************************************
 *
 * XIN is set to GDO2/radio
 *
 * ***************************************/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM     0
#define LED_RED    1
#define SCA3000    2
#define SCP1000    3
#define LCD        4

#define END_DEV           LCD
#define BOARD_DEVICE_MAX (END_DEV+1)

#define NAME "ez430chronos"

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int devices_options_add(void)
{
    return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

struct ez430chronos_struct_t {
    int button_lastvalue;
    uint8_t cma3000_csn;
    uint8_t cma3000_wn;
};

#define SYSTEM_DATA         ((struct ez430chronos_struct_t*)(machine.device[SYSTEM].data))
#define SYSTEM_BUTTON_LAST  (SYSTEM_DATA->button_lastvalue)
#define SCA3000_CSn  (SYSTEM_DATA->cma3000_csn)
#define SCA3000_Wn   (SYSTEM_DATA->cma3000_wn)

int system_reset(int UNUSED dev)
{
    SYSTEM_BUTTON_LAST = 0xff;
    return 0;
}

int system_delete(int UNUSED dev)
{
    return 0;
}

int system_create(int dev_num)
{
    machine.device[dev_num].reset = system_reset;
    machine.device[dev_num].delete = system_delete;
    machine.device[dev_num].state_size = sizeof (struct ez430chronos_struct_t);
    machine.device[dev_num].name = "System Platform";

    STDOUT("%s:\n", NAME);
    STDOUT("%s: =========================\n", NAME);
    STDOUT("%s: '*' = '1'\n", NAME);
    STDOUT("%s: '#' = '2'\n", NAME);
    STDOUT("%s: 'arrow up' = '3'\n", NAME);
    STDOUT("%s: 'light' = '4'\n", NAME);
    STDOUT("%s: 'arrow down' = '5'\n", NAME);
    STDOUT("%s:\n", NAME);
    STDOUT("%s: 'q' to close\n", NAME);
    STDOUT("%s: =========================\n", NAME);
    STDOUT("%s:\n", NAME);
    return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int devices_create(void)
{
    int res = 0;
    int xin_freq, xt2in_freq, vlo_freq, refo_freq;

    xin_freq = 32768; /* 32 kHz */
    xt2in_freq = 8000000;
    vlo_freq = 12000; /* 12 kHz */
    refo_freq = 32768;

    /*********************************/
    /* MSP430 MCU                    */
    /*********************************/

    res += msp430_mcu_create(xin_freq, xt2in_freq, vlo_freq, refo_freq);

    /*********************************/
    /* fix peripheral sizes          */
    /*********************************/

    machine.device_max = BOARD_DEVICE_MAX;
    machine.device_size[SYSTEM] = sizeof (struct ez430chronos_struct_t);
    machine.device_size[LED_RED] = led_device_size();
    machine.device_size[SCA3000] = cma3000_spi_device_size();
    machine.device_size[SCP1000] = scp1000_i2c_device_size();
    machine.device_size[LCD] = ez430_lcd_device_size();

    /*********************************/
    /* allocate memory               */
    /*********************************/

    res += devices_memory_allocate();

    /*********************************/
    /* create peripherals            */
    /*********************************/

#define BKG 0xffffff
#define OFF 0x202020

    res += system_create(SYSTEM);
    res += led_device_create(LED_RED, 0xee0000, OFF, BKG, "red");
    res += cma3000_spi_device_create(SCA3000, 0);
    res += scp1000_i2c_device_create(SCP1000);
    res += ez430_lcd_device_create(LCD);

    /*********************************/
    /* place peripherals Gui         */
    /*********************************/

    {
        int led_w, led_h;
        machine.device[LED_RED].ui_get_size(LED_RED, &led_w, &led_h);

        machine.device[LED_RED].ui_set_pos(LED_RED, 0, 0);
        machine.device[LCD].ui_set_pos(LCD, 0, led_h);
    }

    /*********************************/
    /* end of platform specific part */
    /*********************************/

    return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* this function is called after devices reset    */

/* devices init conditions should be written here */
int devices_reset_post(void)
{
    int refresh = 0;

    SYSTEM_BUTTON_LAST = 0xff;
    SCA3000_CSn = 0;
    SCA3000_Wn = 0;


    REFRESH(LED_RED);
    machine.device[SCA3000].reset(SCA3000);
    machine.device[SCP1000].reset(SCP1000);
    machine.device[LCD].reset(LCD);
    REFRESH(LCD);

    ui_refresh(refresh);
    return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int devices_update(void)
{
    int res = 0;
    int refresh = 0;
    uint8_t val8;

    /* *************************************************************************** */
    /* MCU -> devices                                                              */
    /* *************************************************************************** */

    /* port 1 :                          */
    /* ========                          */

    /* port 2 :                          */
    /* ========                          */

    if (msp430_digiIO_dev_read(PORT1, &val8)) {
        machine.device[LED_RED].write(LED_RED, LED_DATA, !BIT(val8, 7));
        etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
        UPDATE(LED_RED);
        REFRESH(LED_RED);
    }

    /* port 3 :                          */
    /* ========                          */
    /* port 4 :                          */
    /* ========                          */

    /* USCIA (UART Mode)                 */
    /* ==============                    */


    /* USCIB (SPI Mode)                  */
    /* PJ.1 : CSb of accel               */
    /* PJ.2 : SDA of scp1000             */
    /* PJ.3 : SCL of scp1000             */
    /* ==============                    */

    if (msp430_digiIO_dev_read(PORTJ, &val8)) {
        machine.device[SCA3000].write(SCA3000, CMA3000_SPI_CSb, BIT(val8, 1) << CMA3000_SPI_CSb_SHIFT);
        machine.device[SCP1000].write(SCP1000, SCP1000_I2C_SDA_MASK, BIT(val8, 2) << SCP1000_I2C_SDA_SHIFT);
        machine.device[SCP1000].write(SCP1000, SCP1000_I2C_SCL_MASK, BIT(val8, 3) << SCP1000_I2C_SCL_SHIFT);
    }

    /*if (msp430_uscib0_dev_read_spi(&val8)) {
        machine.device[SCA3000].write(SCA3000, CMA3000_SPI_D, val8);
    } */

    /* LCD :                             */
    /* =====                             */
    {
        ez430_lcd_regwrite(LCD, MCU.lcdb.mem, MCU.lcdb.bmem);

        UPDATE(LCD);
        REFRESH(LCD);
    }



    /* *************************************************************************** */
    /* devices -> MCU                                                              */
    /* *************************************************************************** */

    /* SPI SCA3000 */
    {
        uint32_t mask = 0;
        uint32_t value = 0;
        machine.device[ SCA3000 ].read(SCA3000, &mask, &value);
        /*if (mask & CMA3000_SPI_D) {
            msp430_uscib0_dev_write_spi(value & CMA3000_SPI_D);
        }*/
        /* INT -> P2.5*/
        if (mask & CMA3000_SPI_INT_MASK) {
            msp430_digiIO_dev_write(PORT2, (CMA3000_SPI_INT_MASK & value) ? 0x00 : 0x20, 0x20);
        }
    }

    {
        uint32_t mask = 0;
        uint32_t value = 0;
        machine.device[ SCP1000 ].read(SCP1000, &mask, &value);
        if (mask & SCP1000_I2C_SDA_MASK) {
            // PJ.2 : SDA
            msp430_digiIO_dev_write(PORTJ, (SCP1000_I2C_SDA_MASK & value) ? 0x04 : 0x00, 0x04);
        }
        if (mask & SCP1000_I2C_DRDY_MASK) {
            // P2.6 : DRDY
            msp430_digiIO_dev_write(PORT2, (SCP1000_I2C_DRDY_MASK & value) ? 0x00 : 0x40, 0x40);
        }
    }
    /* input on UI */
    /* input on buttons */
    {
        int ev;
        switch ((ev = ui_getevent())) {
        case UI_EVENT_USER:
        {
            uint8_t b = 0xff;

            if ((machine.ui.b_down & UI_BUTTON_1) != 0) {
                b &= ~UI_BUTTON_1;
            }
            if ((machine.ui.b_down & UI_BUTTON_2) != 0) {
                b &= ~UI_BUTTON_2;
            }
            if ((machine.ui.b_down & UI_BUTTON_3) != 0) {
                b &= ~UI_BUTTON_3;
            }
            if ((machine.ui.b_down & UI_BUTTON_4) != 0) {
                b &= ~UI_BUTTON_4;
            }
            if ((machine.ui.b_down & UI_BUTTON_5) != 0) {
                b &= ~UI_BUTTON_5;
            }

            if (b != SYSTEM_BUTTON_LAST) {
                //*   2.2 : KEY_M1 (*)
                if (((b & (UI_BUTTON_1)) == 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_1)) != 0)) {
                    INFO("%s: button * pressed\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x04, 0x04);
                }
                if (((b & (UI_BUTTON_1)) != 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_1)) == 0)) {
                    INFO("%s: button * released\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x00, 0x04);
                }

                //*   2.1 : KEY_M2 (#)
                if (((b & (UI_BUTTON_2)) == 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_2)) != 0)) {
                    INFO("%s: button # pressed\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x02, 0x02);
                }
                if (((b & (UI_BUTTON_2)) != 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_2)) == 0)) {
                    INFO("%s: button # released\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x00, 0x02);
                }

                //*   2.4 : KEY_S1 (arrow up)
                if (((b & (UI_BUTTON_3)) == 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_3)) != 0)) {
                    INFO("%s: button arrow up pressed\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x10, 0x10);
                }
                if (((b & (UI_BUTTON_3)) != 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_3)) == 0)) {
                    INFO("%s: button arrow up released\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x00, 0x10);
                }

                //*   2.3 : KEY_BL (light)
                if (((b & (UI_BUTTON_4)) == 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_4)) != 0)) {
                    INFO("%s: button light pressed\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x08, 0x08);
                }
                if (((b & (UI_BUTTON_4)) != 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_4)) == 0)) {
                    INFO("%s: button light released\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x00, 0x08);
                }

                //*   2.0 : KEY_S2 (arrow down)
                if (((b & (UI_BUTTON_5)) == 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_5)) != 0)) {
                    INFO("%s: button arrow down pressed\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x01, 0x01);
                }
                if (((b & (UI_BUTTON_5)) != 0) && ((SYSTEM_BUTTON_LAST & (UI_BUTTON_5)) == 0)) {
                    INFO("%s: button arrow down released\n", NAME);
                    msp430_digiIO_dev_write(PORT2, 0x00, 0x01);
                }

                SYSTEM_BUTTON_LAST = b;
            }
        }
            break;
        case UI_EVENT_QUIT: /* q */
            mcu_signal_add(SIG_UI);
            break;
        case UI_EVENT_NONE:
            break;
        default:
            ERROR("%s: unknown ui event\n", NAME);
            break;
        }

    }

    /* *************************************************************************** */
    /* update                                                                      */
    /* *************************************************************************** */
    LIBSELECT_UPDATE();
    LIBWSNET_UPDATE();

    UPDATE(SCA3000);
    UPDATE(SCP1000);

    ui_refresh(refresh);

    return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
