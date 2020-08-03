#if 0
    Cyber Radio 1 - Internet Radio Client/Server System - IPC services
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



#ifndef SHM_H
#define SHM_H

/* we need the next line to get the MAX_CONNECTIONS setting */
#include "cr1.h"

/* Completely brain-dead, but useful routines for processes that
   will be sharing shared memory */

unsigned char * establish_shared_memory(void);

unsigned char * get_shared_memory(void);

int initialize_shared_memory(unsigned char *mem);

int lock_memory(void);

int unlock_memory(void);



/*
Ok, now we must decide on the layout scheme for our shared memory.
We need the memory to hold the following info:

  10 "rows" for the updating and viewing of who is requesting data.
  Each row should include the following:
        sockaddr_in structure for server to use
	connect time
	last heard from (terminate connections from people who
	                 we haven't heard from after several minutes)
	hostname
	domainname
	user name (login or actual)
	IP address
	sendto port (probably will be 9222 for first release)

  Finally, we need a "lock" system so that only one process will have
  access updating the memory at a time.  Probably, semaphores should
  be used here.

So, let's propose the following for our structure:

byte 0: lock byte (0=unlocked)  probably replaced by semaphores
byte 1: unused: was the number of connections
bytes 2-11: array mask to indicate whether or not we have a connection 
bytes 12-21: will be an a byte mask to hold the number of errors for
             each connection.  When the server sees an ammount that has
             risen too fast too soon, he can cut the connection.

bytes 22 - 1000: reserved for future

(now let's plan for each user row, starting at 1001)
20 bytes for sockaddr_in structure (really need 16, but you never know..)
4 bytes for connect time
4 bytes for the "last heard from time"
1 byte for type of data wanted (ignored for now, since all we are sending is 8uLaw)
4 bytes for IP address (network byte order)
2 bytes for port (network byte order, will probably be 9222)
20 bytes for user name
30 bytes for host name
30 bytes for domain name

This means that 10 user records will exist between bytes 1001-2150
Cool.
*/

#define CONNECTION_MASK_START	2
#define ERROR_MASK_START	2+MAX_CONNECTIONS

#define USER_RECORD_START	1001
#define SOCK_OFFSET		0
#define CONNECT_OFFSET		20
#define LAST_OFFSET		24
#define TYPE_OFFSET		28
#define IP_OFFSET		29
#define PORT_OFFSET		33
#define USER_NAME_OFFSET	35
#define HOST_NAME_OFFSET	55
#define DOMAIN_NAME_OFFSET	85

#define USER_RECORD_SIZE	115

#endif

