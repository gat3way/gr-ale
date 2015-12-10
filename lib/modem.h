/* -*- c -*-
 * 
 * Copyright (C) 2000 - 2001 
 *   Charles Brain (chbrain@dircon.co.uk)
 *   Ilkka Toivanen (pile@aimo.kareltek.fi)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * File:
 *   modem.h - Modem main file headers
 * 
 * Version:
 *   $Revision: 1.1.1.1 $
 * 
 * Date:
 *   $Date: 2001/05/23 20:19:50 $
 * 
 * Author:
 *   Charles Brain
 *   Ilkka Toivanen
 * 
 * History:
 *   $Log: modem.h,v $
 *   Revision 1.1.1.1  2001/05/23 20:19:50  pile
 *   Initial version for sourceforge.net
 *
 *   Revision 0.1  2001/05/20 19:43:57  pile
 *   Added text logging to file.
 *
 *   Revision 0.0.1.1  2001/05/17 07:10:57  pile
 *   LinuxALE
 *
 */

/*
  Modem signal processing routine
*/
#ifndef __MODEM_H__
#define __MODEM_H__

#include <stdio.h>

void modem_init(void);
char* modem(unsigned short *sample, int length, FILE *log_file);

#endif __MODEM_H__
