
/**
 *  \file   mgetopt.c
 *  \brief  WSim command line parsing
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdio.h>
#include <string.h>

#include "arch/common/debug.h"
#include "mgetopt.h"

#undef DEBUG
#ifdef DEBUG
#  define DMSG(x...)  fprintf(stderr,x)
#else
#  define DMSG(x...)  do { } while(0)
#endif

#define ERROR(x...)  fprintf(stderr,x)
#define WARNING(x...)  fprintf(stderr,x)


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
 * find the option index in the options array definition (if any)
 * return -1 on failure
 */

static int mgetopt_find_option(char *opt, const struct moption_list_t *options, int longopt)
{
  char *eq = NULL;
  int    i = 0;

  if ((longopt == 1) && ((eq = strchr(opt,'=')) != NULL))
    *eq = '\0';
      
  while (options->list[i] && options->list[i]->longname != NULL)
    {
      if ((longopt == 1) && (strcmp(opt,options->list[i]->longname) == 0))
	{
	  if (eq != NULL)
	    *eq = '=';
	  return i;
	}
      i++;
    }


  return -1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void  mgetopt_options_add(struct moption_list_t *options, struct moption_t *o, enum option_package op)
{
  int i;
  for (i=0; i<options->max; i++)
    {
      if (options->list[i]->longname == o->longname)
	{
	  ERROR("** options : long option %s is defined twice\n",o->longname);
	}
    }

  options->list[options->max] = o;
  options->list[options->max]->isset = 0;
  options->list[options->max]->value = 0;

  DMSG("Adding option %d:%s\n",options->max,
       options->list[options->max]->longname);

  if (op == mgetopt_base)
    options->init ++;
  options->max ++;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define DEFSTRING 20

static char* mgetopt_string(int size)
{
  static char fill_string[DEFSTRING + 1];
  memset(fill_string,' ',DEFSTRING);
  fill_string[size] = '\0';
  return fill_string;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mgetopt_options_print(FILE* out, struct moption_list_t *options)
{
  int i;
  for(i=0; i < options->max; i++)
    {
      switch (options->list[i]->type)
	{
	case no_argument:
	  fprintf(out,"   --%s%s : %s\n",
		  options->list[i]->longname,
		  mgetopt_string(DEFSTRING - strlen(options->list[i]->longname)),
		  options->list[i]->helpstring);
	  break;
	case required_argument:
	  fprintf(out,"   --%s=arg%s : %s\n",
		  options->list[i]->longname,
		  mgetopt_string(DEFSTRING - strlen(options->list[i]->longname) - 4),
		  options->list[i]->helpstring);
	  break;
	case optional_argument:
	  fprintf(out,"   --%s[=arg]%s : %s\n",
		  options->list[i]->longname,
		  mgetopt_string(DEFSTRING - strlen(options->list[i]->longname) - 6),
		  options->list[i]->helpstring);
	  break;
	}
      /* separating base options from platform specific options */
      if (i == (options->init - 1))
	{
	  fprintf(out,"\n");
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
 * identify the option and sets *optarg and *optindex accordingly
 * returns UNKNOWN | MISSING | END
 */

static mgetopt_t
mgetopt_parse(int *parseindex, int UNUSED argc, char* const argv[], struct moption_list_t *options, int *optindex)
{
  int    i = -1;
  char *eq = NULL;

  *optindex = 0;

  if ((argv[*parseindex] != NULL) && (strstr(argv[*parseindex],"--") == argv[*parseindex]))
    {
      if ((i = mgetopt_find_option(argv[*parseindex] + 2, options, 1)) == -1)
	  return OPT_UNKNOWN;

      *optindex = i;
      options->list[*optindex]->isset = 1;

      DMSG("   long opt index = %d (%s)\n",*optindex,options->list[*optindex]->longname);

      switch (options->list[*optindex]->type)
	{
	case no_argument:
	  *parseindex = *parseindex + 1;
	  break;
	case required_argument:
	  if (((eq = strchr(argv[*parseindex],'=')) == NULL) || (eq[1] == '\0'))
	    {
	      return OPT_MISSING;
	    }
	  options->list[*optindex]->value = eq + 1;
	  DMSG("      required argument %s\n",options->list[*optindex]->value);
	  *parseindex = *parseindex + 1;
	  break;
	case optional_argument:
	  if ((eq = strchr(argv[*parseindex],'=')) != NULL)
	    {
	      options->list[*optindex]->value = eq + 1;
	      DMSG("      optional argument %s\n",options->list[*optindex]->value);
	    }
	  *parseindex = *parseindex + 1;
	  break;
	}
    }

  return OPT_EOOPT;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

mgetopt_t
mgetopt_long(int *parseindex, int argc, char* const argv[], struct moption_list_t *options, int  *optindex)
{
  DMSG("getopt : start parse\n");

  while (*parseindex < argc)
    {
      if (argv[*parseindex][0] == '\0')
	{
	  return OPT_EOOPT;
	}

      if (argv[*parseindex][0] != '-')
	{
	  return OPT_EOOPT;
	}

      if (argv[*parseindex][1] != '-')
	{
	  return OPT_UNKNOWN;
	}

      if (strcmp(argv[*parseindex],"--") == 0)
	{
	  *parseindex = *parseindex + 1;
	  return OPT_EOOPT;
	}

      switch (mgetopt_parse(parseindex, argc, argv, options, optindex))
	{
	case OPT_UNKNOWN: /* unknown option   */
	  return OPT_UNKNOWN;
	case OPT_MISSING: /* missing argument */
	  return OPT_MISSING;
	default:          /* fill next option */
	  break;
	}
    }
  return OPT_EOOPT;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
