#if 0
    Cyber Radio 1 - Internet Radio Client/Server System - Broadcast console
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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "cr1.h"
#include "shm.h"


#define LITE 1
#define ALL 2

unsigned char *shared_memory;

void disconnect_client(int channel)
{
  int x;
  int actual_channel;
  actual_channel = channel -1;
  if ((channel > 0) && (channel <= MAX_CONNECTIONS))
  {
    lock_memory();
    if (shared_memory[2+actual_channel])
      shared_memory[2+actual_channel] = 0;
    unlock_memory();
  }

}

void display_stat(int channel, int amt)
{
  unsigned char user_record[USER_RECORD_SIZE];
  int actual_channel;
  struct sockaddr_in sock;
  int port;
  char address_string[20];
  int connect, last;
  char user_name[30], host_name[30], domain_name[30];
 
  if ((channel < 1) || (channel > MAX_CONNECTIONS))
    return;

  actual_channel = channel - 1;
  if (shared_memory[2+actual_channel] == 0)
  {
    printf("channel: %d\n", channel); 
    return;
  }
  
  lock_memory();
    memcpy(user_record, shared_memory + USER_RECORD_START + actual_channel * USER_RECORD_SIZE, USER_RECORD_SIZE);
  unlock_memory();

  memcpy(&sock, user_record + SOCK_OFFSET, sizeof(sock));
  port = ntohs(sock.sin_port);
  strcpy(address_string, inet_ntoa(sock.sin_addr));
  
  memcpy(&connect, user_record + CONNECT_OFFSET, sizeof(connect));
  memcpy(&last, user_record + LAST_OFFSET, sizeof(last));

  memcpy(user_name, user_record + USER_NAME_OFFSET, 20);
  user_name[19] = '\0';
  memcpy(host_name, user_record + HOST_NAME_OFFSET, 30);
  host_name[29] = '\0';
  memcpy(domain_name, user_record + DOMAIN_NAME_OFFSET, 30);
  domain_name[29] = '\0';

  printf("channel: %d  ip: %s  port: %u connect: %d  %s@%s",
          channel, address_string, port, connect, user_name, host_name, domain_name);
  if (domain_name[0] != '\0')
    printf(".%s", domain_name);
  printf("\n");
    
}

void display_all(void)
{
  int x;
  for (x = 1; x <= MAX_CONNECTIONS; x++)
    display_stat(x, ALL); 
}
  


void main(void)
{
  int x;
  char com;
  int who;
  char command_line[80];
  shared_memory = get_shared_memory();
  
  display_all(); 

  printf("\nCYBER RADIO 1 BROADCAST CONSOLE\n");
  while (1)
  {
    printf("CR1: ");
    gets(command_line);
    who = 0;
    sscanf(command_line, " %c %d", &com, &who);
    
    switch (com)
    {
      case 'l': if ((who <= MAX_CONNECTIONS) && (who > 0)) display_stat(who, ALL);
                else display_all();
              break;
      case 'd': if ((who <= MAX_CONNECTIONS) && (who > 0)) disconnect_client(who);
              break;
      case 'q': return;
    }               
  }

}

