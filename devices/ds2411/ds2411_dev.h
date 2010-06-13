/* Copyright (c) 2006 by ARES Inria.  All Rights Reserved */

/**
 *  \file   ds2411_dev.h
 *  \brief  Maxim DS2411 device
 *  \author Eric Fleury, Antoine Fraboulet
 *  \date   2006
 **/

/***
   NAME
     ds2411_dev
   PURPOSE
     
   NOTES
     
   HISTORY
     efleury - Jan 31, 2006: Created.
     $Log: ds2411_dev.h,v $
     Revision 1.6  2008-07-06 21:17:55  afraboul
     - doxygen header description
     - \brief description

     Revision 1.5  2006-07-02 18:35:47  afraboul
     - split 1wire and ds2411 automata
     - support for 1 wire protocol complete (except overdrive mode)
     - complete READROM command for ds2411

     Revision 1.4  2006/05/24 14:52:10  afraboul
     - réécriture des .h
     - début mise en forme du .c (uniformisation avec le reste)
     - passage au temps en nano
     - vérification de l'automate à faire avec temps en nano à faire/finir

     Revision 1.3  2006/03/29 19:53:18  afraboul
     - device.stop replaced by device.delete
     - devices should implement powerup and powerdown soon
     - device.get_size is now a scalar

     Revision 1.2  2006/03/29 11:45:26  afraboul
     - plumbing ds2411, not tested

     Revision 1.1  2006/03/28 21:55:40  efleury
     petit pbm de configure avec new version...

***/

#ifndef __DS2411_DEV
#define __DS2411_DEV

#define DS2411_D  1

int ds2411_device_size   (void);
int ds2411_device_create (int dev_num, char *serial);

#endif /* __DS2411_DEV */
