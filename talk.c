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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> 
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "cr1.h"


void main(int argc, char *argv[])
{
  int s;
  int i;
  int x;
  int attempt;
  int port;
  struct hostent *hp;
  long ip = 0;
  struct sockaddr_in sin;
  char terminate_argument[66];
  unsigned char request_pack[200];

  if (argc <= 1)
  {
    printf("usage: talk ip [port]\n");
    exit(1);
  }

  if (argc >= 3)
    port = atoi(argv[2]);
  else port = DATA_PORT;

  strncpy(terminate_argument, argv[1], 65);


  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    fprintf(stderr,"error: socket(): %d\n", errno);
    exit(1);
  }

  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  hp = gethostbyname(terminate_argument);
  if (!hp)
  {
    ip = inet_addr(terminate_argument);
    if (ip == -1)
    {
      printf("can't figure out host ip address\n");
      exit(1);
    }
  }

  /* ip is set to 0 at the start of this function.  If the receive_arg is
     in inter.net.dot.format, then ip gets set.  Else the ip of the
     server is derived from the hp->h_addr value from gethostbyname */
  if (ip)
    memcpy(&sin.sin_addr, &ip, sizeof(ip));
  else
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);

  memset(request_pack, '\0', sizeof(request_pack));
  strcpy(request_pack, HEADER);  /* place stupid header message */
  i = sizeof(HEADER);  
  request_pack[i] = TEXT;
  strcpy(request_pack+i+1+4, "Hello, World\n");

  attempt = 0;
  while (attempt < 10)
    if (sendto(s, request_pack, 200, 0, (struct sockaddr*)(&sin), sizeof(sin)) < 0)
      printf("error: sendto: %d\n", errno);
    else break;
  close(s);

}

