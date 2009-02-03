/*
 *  ortho_interference.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _ORTHO_INTERFERENCE_H_
#define _ORTHO_INTERFERENCE_H_

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int ortho_interference_instantiate (FILE * config_fd) ;
double ortho_interference_correlation (double mW, int radio0, int radio1) ;
int ortho_interference_complete (void);


#endif
