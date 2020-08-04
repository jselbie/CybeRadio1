#if 0
    Cyber Radio 1 - Internet Radio Client/Server System
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

#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <time.h>
#include <syslog.h>
#include <signal.h>


#include "cr1.h"
#include "shm.h"

static unsigned char *shm; 
time_t start;
extern h_errno;

int child_pid = 0;

void request_signal_handler(void)
{
  if (child_pid)
    kill (child_pid, SIGKILL);
  printf("Thank you for listening to Cyber Radio 1\n");
  exit(0);
}


static int issue_socket(void)
{
  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    fprintf(stderr,"error: socket(): %d\n", errno);
    exit(1);
  }
  return (s);
}

/* we need a function that will take the data from a request packet
   and return a valid sockaddr_in */

struct sockaddr_in build_sock(struct sockaddr_in sockin, unsigned char *packet)
{
  struct sockaddr_in sock;
  short port;

  bcopy(packet+sizeof(HEADER), &port, 2);

  sock.sin_family = AF_INET;
  sock.sin_port = port;  /* already in network byte order */
  sock.sin_addr = sockin.sin_addr;

  return sock;
}




int update_list(struct sockaddr_in sin, unsigned char *info)
{

  unsigned long ip;
  unsigned char c;
  unsigned char *slot;
  int update = 0;
  int x, found;
  int empty_slot;
  short int port = 9110;
  time_t t;
  struct sockaddr_in new_sock;
  char user_host[81];


  printf("update_list is going\n");


  lock_memory();
  t = time(NULL) - start;

  /* search to see if the member exists already */
  /* update the last heard from field if found */
  ip = sin.sin_addr.s_addr;  
  for (x = 0; x < MAX_CONNECTIONS; x++)
  {
    if ((shm[2+x]) == 1)
    {
      if ( memcmp(shm+USER_RECORD_START+USER_RECORD_SIZE*x+IP_OFFSET, 
                  &ip, sizeof(long)) == 0)
      {
        memcpy( (shm+LAST_OFFSET + USER_RECORD_START + x*USER_RECORD_SIZE),
                &t, sizeof(time_t) );
        unlock_memory();
        printf("client list updated\n");
        return(0);
      }
    }
  }


  /* find an empty slot */
  c = 0;
  while (c < MAX_CONNECTIONS)
  {
    if (shm[2+c] == 0)
      break; 
    else c++;
  }

  if (c >= MAX_CONNECTIONS)   /* too many participants, get out of here */
  {
    unlock_memory();
    printf("too many clients\n");
    return(1);
  }
                 /* somehow or another, we need to do some serious */
                 /* interprocess communication, to let the broadcast */
                 /* program know to send a message to the requester */
                 /* who is waiting for data */
                 /* for now, we are gonna let him hang */

  shm[2+c] = 1;  /* fill array mask to indicate this slot is being taken */

  new_sock = build_sock(sin, info);
  slot = shm + USER_RECORD_START + c*USER_RECORD_SIZE;
  memcpy(slot+SOCK_OFFSET, &new_sock, sizeof(new_sock));
  memcpy(slot+CONNECT_OFFSET, &t, sizeof(time_t));
  memcpy(slot+LAST_OFFSET, &t, sizeof(time_t));
  slot[TYPE_OFFSET] = AUDIO_ULAW; /* this is hardwired */
  memcpy(slot+IP_OFFSET, &ip, 4); /* network byte order !*/
  port = htons(port);
  memcpy(slot+PORT_OFFSET, &port, sizeof(short int));
  memcpy(slot+USER_NAME_OFFSET, info+sizeof(HEADER)+2, 20);
  memcpy(slot+HOST_NAME_OFFSET, info+sizeof(HEADER)+2+20, 30);
  memcpy(slot+DOMAIN_NAME_OFFSET, info+sizeof(HEADER)+2+20+30, 30);

  printf("new client added\n");
  strncpy(user_host, slot+USER_NAME_OFFSET, 20);
  strncat(user_host, "@", 1);
  strncat(user_host, slot+HOST_NAME_OFFSET, 30);
  if (slot[DOMAIN_NAME_OFFSET] != '\0') {
    strncat(user_host, ".", 1);
    strncat(user_host, slot+DOMAIN_NAME_OFFSET, 30);
  }
  syslog(LOG_INFO|LOG_DAEMON, "CYBER-RADIO-1: connection added from %s (%s)\n",
         user_host, inet_ntoa(new_sock.sin_addr));
  unlock_memory();
  return(0);

}    



int remove_list(struct sockaddr_in sin, unsigned char *info)
{
  unsigned long ip;
  int x;
  char user_host[81];
  unsigned char *slot;

  lock_memory();
  /* search to see if the member exists already */
  /* find what row he is in, and null out the array mask */
  ip = sin.sin_addr.s_addr;  
  for (x = 0; x < MAX_CONNECTIONS; x++)
  {
    if ((shm[2+x]) == 1)
    {
      if ( memcmp(shm+USER_RECORD_START+USER_RECORD_SIZE*x+IP_OFFSET, 
                  &ip, sizeof(long)) == 0)
      {
        shm[2+x] = 0;
        printf("client removed from list\n");

        slot = shm + USER_RECORD_START + x*USER_RECORD_SIZE;
        strncpy(user_host, slot+USER_NAME_OFFSET, 20);
        strncat(user_host, "@", 1);
        strncat(user_host, slot+HOST_NAME_OFFSET, 30);
        if (slot[DOMAIN_NAME_OFFSET] != '\0') {
          strncat(user_host, ".", 1);
          strncat(user_host, slot+DOMAIN_NAME_OFFSET, 30);
        }
        syslog(LOG_INFO|LOG_DAEMON, "CYBER-RADIO-1: self-disconnect from %s\n",user_host);

        unlock_memory();
        return(0);
      }
    }
  }

  unlock_memory();
  return (2); /* error: client not found */
}


/* the child process is going to scan the shared memory list every minute
   and look for people who have not been heard from in the past 61 seconds.
   Connections that appear will be lost get cut off */

void child_process_function(void)
{
  time_t t, ct;  /* t = last time, ct = current_time */
  int x;


  while (1)
  {
    sleep(3*RETRANSMIT_TIME);

    lock_memory();
    ct = time(NULL) - start;

    for (x = 0; x < MAX_CONNECTIONS; x++)
      if ((shm[2+x]) == 1)
      {
        bcopy(shm+USER_RECORD_START+x*USER_RECORD_SIZE+LAST_OFFSET, &t, sizeof(t));
        if ((ct - t) > 3*RETRANSMIT_TIME+1)
        { 
          printf("ct = %d    t = %d\n", ct, t);
          shm[2+x] = 0;  /* remove client from list */
          printf("auto-disconnect of client\n");
        }
      }
    unlock_memory();
  }

}
    


void main(void)
{
  struct sockaddr_in sin, sin2;
  int s, n, len, i, ret;
  char hn[65];
  unsigned char msg[1500];
  
  struct hostent *hp;

  shm = establish_shared_memory();
  initialize_shared_memory(shm);

  s = issue_socket();
  sin.sin_family = AF_INET;
  sin.sin_port = htons(CONTROL_PORT);
  gethostname(hn, 65);

	/*  hp = gethostbyname(hn);
	  if (hp == NULL)
	  {
	    fprintf(stderr, "error with gethostbyname: %d\n", h_errno);
	    exit(1);
	  }
	  bcopy(hp->h_addr, &sin.sin_addr, hp->h_length); */
  sin.sin_addr.s_addr = INADDR_ANY;
  bind(s, (struct sockaddr *)&sin, sizeof(sin));

  start = time(NULL);

  /* fork off the child process to keep scanning for lost connections */
  child_pid = fork();
  if (child_pid == 0)
  {
    child_process_function();
    exit(1);
  }
    

  signal(SIGTERM, request_signal_handler);
  signal(SIGINT, request_signal_handler);
  signal(SIGQUIT, request_signal_handler);
  signal(SIGHUP, request_signal_handler);

 
  while (1)
  {
    len = sizeof(sin2);
    n = recvfrom(s, msg, BUFSZ, 0, (struct sockaddr *)&sin2, &len);
    if (n < 0)
    {
      fprintf(stderr, "recvfrom error\n");
      exit(1);
    }

    i = sizeof(HEADER);
    /* authentication */
    ret = strncmp(HEADER, msg, i);

    if ((!ret) && (msg[i+2+20+30+30] == REQUEST))
      update_list(sin2, msg);
    else if ((!ret) && (msg[i+2+20+30+30] == TERMINATE))
     remove_list(sin2, msg);

  }
}

