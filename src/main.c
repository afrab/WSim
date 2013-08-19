
/**
 *  \file   main.c
 *  \brief  WSim simulator entry point, non gui version
 *  \author Antoine Fraboulet
 *  \date   2013
 **/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "src/mainworker.h"

/**
 * main : program entry point
 **/
int main(int argc, char* argv[])
{
  return simulationMain(argc, agrv);
}

