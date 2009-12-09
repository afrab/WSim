
/**
 *  \file    main.c
 *  \brief   Terminal for wsim serial line emulation 
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <signal.h>

#include "fifo.h"
#include "log.h"
#include "ui.h"

#define CONSOLE_DEFAULT_WIDTH  70
#define CONSOLE_DEFAULT_HEIGHT 8
#define CONSOLE_DEFAULT_IOMODE UI_MODE_OUTPUT
#define CONSOLE_DEFAULT_REOPEN 0
#define CONSOLE_DEFAULT_LOG    "wconsole.log"
#define CONSOLE_DEFAULT_FIFO   NULL

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define ERROR(x...) fprintf(stderr,x)

#if defined(DEBUG)
#define DMSG(x...)  fprintf(stderr,x)
#else
#define DMSG(x...)  do { } while(0)
#endif

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

struct s_console_t {
  int            io_mode;
  int            reopen;
  int            stop;

  char          *fifo_name;

  struct fifo_t *fifo;
  struct ui_t   *ui;

  char          *logfile;
  FILE          *log;

  int            screen_width;
  int            screen_height;
};

typedef struct s_console_t console_t;

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void usage(char* name)
{
  fprintf(stdout,"usage: %s               \n\
  --help      -h : this message           \n\
  --mode      -m : mode i/o/d (default o) \n\
  --fifo=name -f : open an existing fifo  \n\
  --reopen    -r : reopen on exit         \n\
  --log=name  -l : logfile name           \n\
  --width=n   -W : window width           \n\
  --height=n  -H : window height          \n",name);
  exit(1);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

char* io_mode_name(int mode)
{
  switch (mode)
    {
    case UI_MODE_IO     : return "duplex";
    case UI_MODE_INPUT  : return "input from keyboard";
    case UI_MODE_OUTPUT : return "output from fifo";
    }
    return "unknown";
}

int options_parse(console_t *console, int argc, char* argv[])
{
  int c;

  console->screen_width     = CONSOLE_DEFAULT_WIDTH;
  console->screen_height    = CONSOLE_DEFAULT_HEIGHT;
  console->io_mode          = CONSOLE_DEFAULT_IOMODE;
  console->reopen           = CONSOLE_DEFAULT_REOPEN;
  console->logfile          = CONSOLE_DEFAULT_LOG;
  console->fifo_name        = CONSOLE_DEFAULT_FIFO;

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = 
	{
	  {"help",   no_argument,       0, 'h'},
	  {"output", no_argument,       0, 'o'},
	  {"fifo",   required_argument, 0, 'f'},
	  {"log",    required_argument, 0, 'l'},
	  {"reopen", no_argument,       0, 'r'},
	  {"width",  required_argument, 0, 'W'},
	  {"height", required_argument, 0, 'H'},
	  {0, 0, 0, 0}
	};

      c = getopt_long (argc, argv, "hm:rf:l:W:H:",long_options, &option_index);

      if (c == -1)
	break;

      switch (c)
	{
	case 0:
	  printf("option %s", long_options[option_index].name);
	  if (optarg)
	    printf(" avec argument %s",optarg);
	  printf("\n");
	  break;
	case 'h':
	  usage(argv[0]);
	  break;
	case 'm':
	  if (optarg)
	    {
	      switch (optarg[0])
		{
		case 'i':
		  console->io_mode = UI_MODE_INPUT;
		  break;
		case 'o':
		  console->io_mode = UI_MODE_OUTPUT;
		  break;
		case 'd':
		  console->io_mode = UI_MODE_IO;
		  break;
		}
	    }
	  break;
	case 'r':
	  console->reopen = 1;
	  break;
	case 'f':
	  console->fifo_name = optarg;
	  break;
	case 'l':
	  console->logfile = optarg;
	  break;

	case 'W':
	  console->screen_width = atoi(optarg);
	  break;
	case 'H':
	  console->screen_height = atoi(optarg);
	  break;

	default:
	  ERROR("console: unknown option %s\n", long_options[option_index].name);
	}
    }

  return 0; 
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void options_print(console_t *c)
{
  DMSG("console:opt: mode      %d\n",c->io_mode);
  DMSG("console:opt: reopen    %d\n",c->reopen);
  DMSG("console:opt: fifo name %s\n",c->fifo_name ? c->fifo_name : "null");
  DMSG("console:opt: logfile   %s\n",c->logfile);
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define UI_FIFO       0
#define UI_KBD        1

#define CONSOLE_AGAIN 0
#define CONSOLE_QUIT  1

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int thread_func_input_kbd(void *data)
{
  int c;
  int ret = CONSOLE_AGAIN;
  console_t *console = data;
  SDL_mutex *lock = console->ui->lock;

  while (! console->stop)
    {
      c = ui_kbd_event(console->ui,KBD_BLOCKING);
      switch (c)
	{
	case UI_ERROR:
	case UI_QUIT:
	  console->stop = 1;
	  DMSG("wconsole:kbd:stop\n");
	  ret = CONSOLE_QUIT;
	  break;
	case UI_NOEVENT:
	  break;
	default:
	  SDL_mutexP(lock);    /* Lock  the potty */
	  ui_putchar(console->ui,UI_KBD,c & 0xffu);
	  fifo_putchar(console->fifo,c & 0xffu);
	  SDL_mutexV(lock);
	  break;
	}
    }
  DMSG("wconsole:kbd: thread return\n");
  DMSG("wconsole:kbd: send quit to fifo thread\n");
  fifo_post_quit(console->fifo); 
  return ret;
}

int thread_func_input_fifo(void *data)
{
  int c;
  int ret = CONSOLE_AGAIN;
  console_t *console = data;
  SDL_mutex *lock = console->ui->lock;

  while (! console->stop)
    {
      c = fifo_event(console->fifo, FIFO_BLOCKING); 
      switch (c)
	{
	case FIFO_SELECT_ERROR:
	  DMSG("wconsole:console_loop: fifo select error, quit\n");
	  console->stop = 1;
	  DMSG("wconsole:fifo:stop\n");
	  ret = CONSOLE_QUIT;
	  break;

	case FIFO_IO_ERROR:
	  DMSG("wconsole:console_loop: fifo IO error\n");
	  console->stop = 1;
	  DMSG("wconsole:fifo:stop\n");
	  ret = CONSOLE_AGAIN; 
	  break;

	case FIFO_QUIT:
	  DMSG("wconsole:console_loop: fifo quit\n");
	  console->stop = 1;
	  DMSG("wconsole:fifo:stop\n");
	  ret = CONSOLE_AGAIN; 
	  break;

	case FIFO_NOEVENT:
	  /* timeout on nonblocking mode, should not happen in blocking mode */
	  break;

	default:
	  SDL_mutexP(lock);    /* Lock  the potty */
	  log_putchar(console->log,c & 0xffu);
	  ui_putchar(console->ui,UI_FIFO,c & 0xffu);
	  SDL_mutexV(lock);
	  break;
	}
    }
  DMSG("wconsole:fifo: thread return\n");
  DMSG("wconsole:fifo: send quit to kbd thread\n");
  ui_post_quit(console->ui);
  return ret;
}


int console_loop(console_t *console)
{
  int ret = CONSOLE_AGAIN;
  int ret_kbd;
  int ret_fifo;
  SDL_Thread *input_kbd = NULL;
  SDL_Thread *input_fifo = NULL;

  /* Create the synchronization lock */
  console->stop = 0;
  console->ui->lock = SDL_CreateMutex();

  /* Create threads */
  switch (console->io_mode)
    {
    case UI_MODE_IO:
      input_kbd  = SDL_CreateThread(thread_func_input_kbd,  console);
      input_fifo = SDL_CreateThread(thread_func_input_fifo, console);
      break;
    case UI_MODE_INPUT:
      input_kbd  = SDL_CreateThread(thread_func_input_kbd,  console);
      break;
    case UI_MODE_OUTPUT:
      input_fifo = SDL_CreateThread(thread_func_input_fifo, console);
      break;
    }
  
  /* Waiting for threads */
  switch (console->io_mode)
    {
    case UI_MODE_IO:
      SDL_WaitThread(input_kbd,  &ret_kbd);
      SDL_WaitThread(input_fifo, &ret_fifo);
      break;
    case UI_MODE_INPUT:
      SDL_WaitThread(input_kbd,  &ret_kbd);
      break;
    case UI_MODE_OUTPUT:
      SDL_WaitThread(input_fifo, &ret_fifo);
      break;
    }

  /* Leave */
  SDL_DestroyMutex(console->ui->lock);
  if ((ret_kbd == CONSOLE_QUIT) || (ret_fifo == CONSOLE_QUIT))
    {
      ret = CONSOLE_QUIT;
    }

  return ret;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

console_t *glob_console;

void sighandler(int n)
{
  fprintf(stdout,"wconsole: received signal %d\n",n);
  glob_console->reopen = 0;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

int
main(int argc, char *argv[])
{
  console_t console;

  options_parse(&console,argc,argv);
  options_print(&console);

  glob_console = &console;

  /******************************/
  /* open UI keyboard interface */
  /******************************/

  if ((console.ui = ui_create("Wsim serial",console.screen_width,
			      console.screen_height,console.io_mode)) == NULL)
    {
      fprintf(stderr,"Cannot create user interface\n");
      exit(1);
    }

  if (console.io_mode == UI_MODE_OUTPUT)
    {
      /* we use only SDL as an output only */
      signal(SIGQUIT,sighandler);
      signal(SIGUSR1,sighandler);
      signal(SIGUSR2,sighandler);
    }

  console.log = log_open(console.logfile);
      
  /***********************/
  /* do something useful */
  /***********************/
  do 
    {
      if ((console.fifo = fifo_init(console.fifo_name)) == NULL)
	{
	  fprintf(stderr,"Cannot open serial fifo\n");
	  break;
	}
      
      log_print(console.log,"==== Wsim serial console ====\n");
      fprintf(stdout,"==== Wsim serial console ====\n");

      fprintf(stdout,"Local fifo is %s\n",console.fifo->local_name);
      fprintf(stdout,"Remote fifo is %s\n",console.fifo->remote_name);
      fprintf(stdout,"I/O mode is %s\n",io_mode_name(console.io_mode));
      fflush(stdout);
      ui_setname(console.fifo->remote_name,console.fifo->remote_name);

      switch (console_loop(&console))
	{
	case CONSOLE_AGAIN:
	  break;
	case CONSOLE_QUIT:
	  console.reopen = 0;
	  break;
	default:
	  break;
	}

      fifo_delete(console.fifo);

      fprintf(stdout,"==== End of connexion ====\n");
      log_print(console.log,"==== End of connexion ====\n");
    } 
  while (console.reopen);

  log_close(console.log);
  ui_delete(console.ui);
  return 0;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     
