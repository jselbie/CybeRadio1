#if 0
    Cyber Radio 1 - Internet Radio Client/Server System - Server Send Program
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
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "cr1.h"
#include "shm.h"

unsigned char *shm;
char audio_file_arg[40] = "/dev/audio";
extern char *optarg;

void main(int argc, char *argv[])
{
  int bsock;
  struct sockaddr_in bsin, sin;
  struct hostent *hp;
  char hname[65];
  unsigned char audio_buffer[1500];
  int audio_file;
  unsigned char *byte_mask, *sock_start, *error_mask;
  int x, i;
  long l;
  int arg_c;  
  int use_stdin = 0;
  int total;
  

  while ((arg_c = getopt(argc, argv, "?a:i")) != EOF) {
    switch(arg_c) {
     case('a') :
       strcpy(audio_file_arg, optarg);
       break;
     case('i') :       /* -o indicates to use stdin for audio input */
	use_stdin = 1;
        break;
     case ('?') : 
     default :
       fprintf(stderr, "USAGE: server [-a audiofile | -o]\n");
       return;
    }
  }

  if (use_stdin)
    audio_file = fileno(stdin);
  else
    audio_file = open(audio_file_arg, O_RDONLY);
  if (audio_file == -1)
  {
    fprintf(stderr, "Error opening audio file: %d\n", errno);
    exit(1);
  }

  shm = get_shared_memory();

  if ((bsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    fprintf(stderr,"error: socket(): %d\n", errno);
    exit(1);
  }
 

  bsin.sin_family = AF_INET;
  bsin.sin_port = htons(SERVER_SEND_PORT);
  bsin.sin_addr.s_addr = INADDR_ANY;
 
  bind(bsock, &bsin, sizeof(bsin));

  byte_mask = shm + CONNECTION_MASK_START;
  error_mask = shm + ERROR_MASK_START;
  sock_start = shm + USER_RECORD_START;

  strcpy(audio_buffer, HEADER);
  i = sizeof(HEADER);
  audio_buffer[i] = AUDIO_ULAW;
  /* *((long *)(audio_buffer+i+1)) = htonl(PLAYSZ);  sun doesn't like this line */
  l = htonl(PLAYSZ);
  bcopy(&l, audio_buffer+i+1, sizeof(l));
    
  while (1)
  {
    total = 0;
    while (total < PLAYSZ)
      total += read(audio_file, audio_buffer+i+1+4+total, PLAYSZ-total);

    lock_memory();

    for (x=0; x<MAX_CONNECTIONS; x++)
    {
      if (byte_mask[x])
      {
        if (sendto(bsock, audio_buffer, PLAYSZ+i+4+1, 0,
                   sock_start+USER_RECORD_SIZE*x, sizeof(sin)) < 0)
        { 
          fprintf(stderr, "sendto() error: %d\n", errno);
          error_mask[x] = error_mask[x] + 1;
        }
      }
    }
    unlock_memory();
  }
}
 
