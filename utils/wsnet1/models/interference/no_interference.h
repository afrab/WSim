/*
 *  no_interference.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _NO_INTERFERENCE_H_
#define _NO_INTERFERENCE_H_

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int no_interference_instantiate (FILE * config_fd) ;
double no_interference_correlation (double mW, int radio0, int radio1) ;
int no_interference_complete (void);


#endif
