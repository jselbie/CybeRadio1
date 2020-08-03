#if 0
    Cyber Radio 1 - Internet Radio Client/Server System - General header file
    Copyright (C) 1995  John Selbie

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    John Selbie
    1600 NW 4th Avenue
    #24
    Gainesville, FL 32603

    e-mail: jselbie@cis.ufl.edu

#endif


#ifndef _CR1_H
#define	_CR1_H


#define CONTROL_PORT 19222
#define DATA_PORT 9222
#define SERVER_SEND_PORT 29222

#define SERVER_NAME	"cyber.radio.org"

#define AUDIO_ULAW	1
#define TEXT		2
#define REQUEST		3
#define TERMINATE	4

#define HEADER	"Cyber Radio 1"
#define BUFSZ           1400

#define USE_CIRCULAR_BUFFER
/*

unfortunately ossfreee versin 3.8beta5 seems to be 
go into infinite loop when too much data is thrown at it. 
Also, all of the filters I have sound real crappy.
So, don't do this at this time.

#define DO_RATE_CHANGE
*/


#ifdef DO_RATE_CHANGE 
  #define USE_CIRCULAR_BUFFER
  #define EXTRABUFSZ           5000

#else /* DO_RATE_CHANGE */
  #define EXTRABUFSZ     BUFSZ
#endif/* DO_RATE_CHANGE */

#ifdef USE_CIRCULAR_BUFFER
#define CIRC_BUFF_SIZE 66000
#define PRELOAD_SIZE 30000
#endif /* USE_CIRCULAR_BUFFER */

/* corresponds to 20 seconds */
#define RETRANSMIT_INTERVAL	123
#define RETRANSMIT_TIME	20

#define PLAYSZ          1300

#define MEM_KEY 8930
#define MEM_SIZE 4000

#define SEM_KEY 8850


#define MAX_CONNECTIONS	10
#define MAX_ERRORS	7


#ifdef NEED_BCOPY

#define bzero(src, size)	memset((src), '\0', (size))
#define bcopy(src, dest, size)	memcpy((dest), (src), (size)) 

#endif

#endif
