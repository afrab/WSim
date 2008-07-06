#include "taskhandler.h"

//Entries in the task table of the taskhandler module
//edit you tasks here
//
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>


// Format of tasktable entries:
// Task function (name only, its called in assembler)
//   #define TASKnn          appTask
//
// to get bitposition on for a task in "taskreg" (see taskhandler.s43)
// the "TASKxx_bits" defines from "taskhandler.h" should be used.

//reference tasks by name not by number, so that moving them becomes simpler
#define TASK_serTask    TASK00_bits
#define TASK_keyHandler TASK01_bits

#define TASK00          serTask
#define TASK01          foo
#define TASK02          foo
#define TASK03          foo
#define TASK04          foo
#define TASK05          foo
#define TASK06          foo
#define TASK07          foo
#define TASK08          foo
#define TASK09          foo
#define TASK10          foo
#define TASK11          foo
#define TASK12          foo
#define TASK12          foo
#define TASK13          foo
#define TASK14          foo
#define TASK15          foo
