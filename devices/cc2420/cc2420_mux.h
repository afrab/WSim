
/**
 *  \file   cc2420_mux.h
 *  \brief  CC2420 test output signals 
 *  \author Loic Lemaitre
 *  \date   2009
 **/

/*
 *  cc2420_mux.h
 *  
 *
 *  Created by Loic Lemaitre on 05/10/09.
 *  Copyright 2009 __WorldSens__. All rights reserved.
 *
 */

#ifndef _CC2420_MUX_H
#define _CC2420_MUX_H

void     cc2420_assert_ccamux  (struct _cc2420_t *cc2420, int event, int assert);
void     cc2420_assert_sfdmux  (struct _cc2420_t *cc2420, int event, int assert);
void     cc2420_update_mux     (struct _cc2420_t *cc2420, uint16_t val);

#endif
