/**
 *  \file   console_utils.h
 *  \brief  WSim Console utils functions
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef __CONSOLE_UTILS_H_
#define __CONSOLE_UTILS_H_

/* ************************************************** */

typedef struct console_state_t      *console_state;
typedef struct console_cmd_t        *console_cmd;
typedef struct console_cmd_params_t *console_cmd_params;

/* ************************************************** */

#define CON_CMD_OPT_MAX 20

struct console_cmd_params_t {
  console_state  cs;
  int            nopt;
  char          *options[CON_CMD_OPT_MAX];
};

/* ************************************************** */

enum enum_con_cmd_val_t {
  CON_CMD_NONE      = 0,
  CON_CMD_OK        = 1,
  CON_CMD_OK_SIG    = 2,
  CON_CMD_ERROR     = 3,
  CON_CMD_ERROR_SIG = 4,
  CON_CMD_QUIT      = 5
};

typedef enum enum_con_cmd_val_t con_cmd_val;

struct console_cmd_t {
  char        *name;
  int          nb_option; 
  char        *text;
  con_cmd_val (*fun)(console_cmd_params);
};

/* ************************************************** */

#define CON_CMD_MAX 30
#define CON_PS1_MAX 10

struct console_state_t {
  int                  cmd_max;
  struct console_cmd_t commands[CON_CMD_MAX];

  char                 ps1     [CON_PS1_MAX];

  int                  fd_input; 
  int                  fd_output;
};

/* ************************************************** */

#define CON_IO_ERROR -1
#define CON_IO_OK     0

int console_read(console_state cs, char *buff, int sizemax);
int console_write(console_state cs, char* fmt, ...);

/* ************************************************** */

int console_atoi(char *buff, long *v);
int console_htoi(char *buff, long *v);

/* ************************************************** */

#endif
