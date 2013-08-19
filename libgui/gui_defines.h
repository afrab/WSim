#pragma once


/*
 *
 */

#define UI_OK                     0
#define UI_ERROR                  1


/* UI
 *
 *                      | .... .... | .... .... | .... .... | .... .... |
 *
 *   system event                                             xxxx xxxx
 *   user event                                   xxxx xxxx   
 *
 *
 *   0 must be a value OK / No Event
 */

#define UI_EVENT_NONE             0x00000000u
#define UI_EVENT_QUIT             0x00000001u
#define UI_EVENT_UNKNOWN          0x00000002u
#define UI_EVENT_USER             0x00000100u



/*
 * User button codes
 *
 */
#define NB_BUTTONS 8

#define UI_BUTTON_1               (1 << 0)
#define UI_BUTTON_2               (1 << 1)
#define UI_BUTTON_3               (1 << 2)
#define UI_BUTTON_4               (1 << 3)
#define UI_BUTTON_5               (1 << 4)
#define UI_BUTTON_6               (1 << 5)
#define UI_BUTTON_7               (1 << 6)
#define UI_BUTTON_8               (1 << 7)
