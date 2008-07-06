
/**
 *  \file   mgetopt.h
 *  \brief  WSim command line parsing
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef WSIM_MGETOPT_H
#define WSIM_MGETOPT_H

#define MAX_OPTIONS 30

enum option_type {
  no_argument        = 0,
  required_argument  = 1,
  optional_argument  = 2
};

enum option_package {
  mgetopt_base       = 0,
  mgetopt_optional   = 1
};

struct moption_t {
  char               *longname;
  enum option_type    type;
  char               *helpstring;
  char                isset;
  char               *value;
};

struct moption_list_t {
  int max;
  int init;
  struct moption_t *list[MAX_OPTIONS];
};

enum mgetopt_enum_t 
  {
    OPT_EOOPT    = 0,
    OPT_UNKNOWN  = 1,
    OPT_MISSING  = 2
  };
typedef enum mgetopt_enum_t mgetopt_t;

mgetopt_t mgetopt_long     (int *parseindex, int argc,  char* const argv[], 
		            struct moption_list_t *options, int *optindex);

void mgetopt_options_add   (struct moption_list_t *options, struct moption_t *o, enum option_package);
void mgetopt_options_print (FILE* out, struct moption_list_t *options);

#endif /* WSIM_MGETOPT_H */
