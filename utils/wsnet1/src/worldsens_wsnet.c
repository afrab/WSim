#include <public/simulation.h>

/* main function */

void 
app_exit_error()
{
  exit( EXIT_FAILURE );
}

int
main (int argc, char **argv)
{
  /* Start simulation */
  return simulation_start (argc, argv);
}
