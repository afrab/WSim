/**
 *  \file   console_cmd.h
 *  \brief  WSim Console utils functions
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#ifndef __CONSOLE_CMD_H_
#define __CONSOLE_CMD_H_

con_cmd_val console_cmd_help     (console_cmd_params);
con_cmd_val console_cmd_status   (console_cmd_params);
con_cmd_val console_cmd_set      (console_cmd_params);
con_cmd_val console_cmd_show     (console_cmd_params);
con_cmd_val console_cmd_reset    (console_cmd_params);
con_cmd_val console_cmd_loadelf  (console_cmd_params);
con_cmd_val console_cmd_quit     (console_cmd_params);

con_cmd_val console_cmd_run      (console_cmd_params);
con_cmd_val console_cmd_run_insn (console_cmd_params);
con_cmd_val console_cmd_run_time (console_cmd_params);

#endif
