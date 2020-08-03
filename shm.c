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


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "cr1.h"
#include "shm.h"

/* Completely brain-dead, but useful routines for processes that
   will be sharing shared memory */

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
bytest 2- 11: connection array mask

bytes 12 - 1000: reserved for future

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

#ifdef SOLARIS

union semun {
  int val;
  struct semid_ds *buf;
  ushort *array
} arg;

#endif





static struct sembuf op_lock[2] = {
  0, 0, 0,
  0, 1, 0
};

static struct sembuf op_unlock[1] = {
  0, -1, 0
};

int semid = -1;



unsigned char * establish_shared_memory(void)
{
  int shmid;
  unsigned char *mem;


  if ((shmid = shmget(MEM_KEY, MEM_SIZE, IPC_CREAT | 0666)) < 0)
  {
    fprintf(stderr, "shmget failed: %d\n", errno);
    exit(1);
  }

    
  if ((mem = shmat(shmid, NULL, 0)) == (char *) -1)
  {
    fprintf(stderr, "shmat failed %d\n", errno);
    exit(1);
  }

  if ((semid = semget(SEM_KEY, 1, IPC_CREAT|0666)) < 0)
    printf("establish_shared_memory: semaphore creation problem\n");


  return ((unsigned char *)mem);
}

unsigned char * get_shared_memory(void)
{
  int shmid;
  unsigned char *mem;

  if ((shmid = shmget(MEM_KEY, MEM_SIZE, 0666)) < 0)
  {
    fprintf(stderr, "shmget failed: %d\n", errno);
    exit(1);
  }


  if ((mem = shmat(shmid, NULL, 0)) == (char *) -1)
  {
    fprintf(stderr, "shmat failed %d\n", errno);
    exit(1);
  }

  if ((semid = semget(SEM_KEY, 1, 0666)) < 0)
    printf("establish_shared_memory: semaphore creation problem\n");

  return ((unsigned char *)mem);
}
 



int initialize_shared_memory(unsigned char *mem)
{
  union semun s;

  s.val = 0;
  semctl(semid, 0, SETVAL, s);

  lock_memory();
  memset(mem, '\0', MEM_SIZE);
  unlock_memory();
  return (1);
}

int lock_memory(void)
{
  if (semop(semid, op_lock, 2) < 0)
    printf("error locking shared memory: %d\n,", errno);
  return 0;
}

int unlock_memory(void)
{

  if (semop(semid, op_unlock, 1) < 0)
    printf("error unlocking memory: %d\n", errno);

}

