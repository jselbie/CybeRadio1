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


#ifdef sgi
#define SGI_IRIX
#endif

#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h> 
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include "cr1.h"

#ifdef SGI_IRIX
#include "audio_sgi.h"
#include "tables.h"
#endif

/* 
 * The player will behave a lot better if a good bit of sound 
 * is saved before playing starts.  There will be less drop-out, on
 * average, for very jittery networks.  That is, as packets arrive with
 * varying latency, we store them up, and only then dump them to the audio
 * device.
 */
#ifdef USE_CIRCULAR_BUFFER
#include "circ.h"
#endif /* USE_CIRCULAR_BUFFER */

/* 
 * rate hacks
 */

#ifdef DO_RATE_CHANGE
#include "rate.h"
#endif /* DO_RATE_CHANGE */

#define VERSION 1
#define RCS_VERSION "$Id: cr1.c,v 3.9 1995/10/09 22:02:57 jselbie Exp $"

int sock1;   /* to send request messages through */
int sock2;   /* to receive data */
char audio_file_arg[40] = "/dev/audio";
char receive_arg[40] = SERVER_NAME;
int port_arg = DATA_PORT;
int remote_port_arg = CONTROL_PORT;

int use_stdout = 0;
int use_alt_device = 0;
int audio_file;         /* /dev/audio */

int header_size;

unsigned char request_pack[200];
struct sockaddr_in sendto_sin;  /* contains address of host */


#ifdef SGI_IRIX
AUDIO_T sgi_at;
#endif

/* forward declaration */
int send_request_pack(int s, int type);

int issue_socket(void)
{
  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("issue_socket():");
    exit(1);
  }
  return (s);
}


void terminate_connection(void)
{
  send_request_pack(sock1, TERMINATE);
}


void signal_handler(int x)
{
  terminate_connection();
#ifdef SGI_IRIX
  if ((!use_stdout) && (!use_alt_device))
    AUDIO_close(sgi_at);
  else
#endif
    if (!use_stdout) close(audio_file);

  close(sock1);
  close(sock2);
  fprintf(stderr,"\nThank you for listening to Cyber Radio 1\n");
  fprintf(stderr,"Cyber Radio 1 Software by John Selbie (jselbie@cis.ufl.edu)\n");
  exit(0);
}


void build_request_pack(int port)
{

  int i;
  uid_t uid;
  short p;
  unsigned char hn[31];
  unsigned char dn[31];
  char user_name[21];
  struct passwd *pw;
  struct hostent *hp;
  int ip = 0;

  memset(request_pack, '\0', sizeof(request_pack));
 
  strcpy(request_pack, HEADER);  /* place stupid header message */
 
  /* place port number within the 2 bytes */
  header_size = i = sizeof(HEADER);
  p = htons(port);
  memcpy(request_pack + i, &p, 2);
 
  /* put the user name on the data structure */
  uid = geteuid();
  pw = getpwuid(uid);
  strncpy(request_pack+i+2, pw->pw_name, 20);
 
  /* place hostname and domain-name on data structure */
  gethostname(hn, 30);   /* ignore error */
  getdomainname(dn, 30); /* ignore error */
  strncpy(request_pack+i+2+20, hn, 30);
  strncpy(request_pack+i+2+20+30, dn, 30);
 
  /* for this version, we are only going to want ULAW DATA */

  request_pack[i+2+20+30+30] = REQUEST;  /* could be REQUEST or TERMINATE */

  /* prep the socket address structure */
  sendto_sin.sin_family = AF_INET;
  sendto_sin.sin_port = htons(remote_port_arg);

  hp = gethostbyname(receive_arg);
  if (!hp)
  {
    ip = inet_addr(receive_arg);
    if (ip == -1)
    {
      fprintf(stderr, "can't figure out host ip address\n");
      exit(1);
    }
  }
 
  /* ip is set to 0 at the start of this function.  If the receive_arg is
     in inter.net.dot.format, then ip gets set.  Else the ip of the
     server is derived from the hp->h_addr value from gethostbyname */
  if (ip)
    bcopy(&ip, &sendto_sin.sin_addr, sizeof(ip));
  else
    bcopy(hp->h_addr, &sendto_sin.sin_addr, hp->h_length);
}




int send_request_pack(int s, int type)
{
  int attempt = 0;

  request_pack[header_size+2+20+30+30] = type;

  while ((sendto(s, request_pack, 200, 0, (struct sockaddr *) &sendto_sin, sizeof(sendto_sin)) < 0)
          && attempt < 10) {
    attempt++;
    perror("sendto:");
  }
  if (attempt == 10)
    fprintf(stderr,"sendto error: unable to send request packet\n");
}




void main(int argc, char *argv[])
{
  char buffer[EXTRABUFSZ];     /* ethernet packet data */
  struct sockaddr_in sin; /* socket address structure to send on */
  struct hostent *hp;     /* host structure to send with */
  char hname[65];
  int i;                 /* sizeof the standard header */
  int ret;               /* used to compare HEADER string */
  long x;                /* length of data in network byte order */
  int len;               /* length of the sin struct before recvfrom call */
  int n;                 /* return value for recvfrom */
  long data_length;      /* x converted from network type */
  char c;
  int count;
  int arg_c;
  int bind_success;
  pid_t pp;
  struct timeval tv;     /* for select call */ 
  fd_set fds;            /* for select call */
  int renotify_count;    /* for select call */
  int total;             /* for writing all of the buffer to file */
#ifdef USE_CIRCULAR_BUFFER
  int preload = 12;       /* preroll */
  struct circular_buffer *cb;   /* buffer */
#endif /* USE_CIRCULAR_BUFFER */


/* Process command line arguments and switches ----------------------- */


  while ((arg_c = getopt(argc, argv, "?ha:p:oi:vr:")) != EOF) {
    switch(arg_c) {
     case('a') :
       strcpy(audio_file_arg, optarg);
       use_alt_device = 1;
       break;
     case('o') :       /* -o indicates to use stdout for audio output */
        use_stdout = 1;
        use_alt_device = 1;
        break;
     case('i') :
      strcpy(receive_arg, optarg);
      break;
     case('p') :
       port_arg = atoi(optarg);
       if ((port_arg < 1025) || (port_arg > 65535))
       {
         fprintf(stderr, "Invalid port number\n");
         exit(1);
       }
       break;
     case('r') :
       remote_port_arg = atoi(optarg);
       break;
     case('v') :
       fprintf(stderr, "CR-1 client: RELEASE %d  INTERNAL: %s\n", VERSION, RCS_VERSION);
       return;
     case ('?') : 
     case ('h') : 
     default :
       fprintf(stderr, "USAGE: %s [-a audiofile | -o] [-p receiveport] [-r serverport] [-i server_ip_address]\n", argv[0]);
       return;
    }
  }
   



/* Open Audio Device ----------------------------------------------- */

#ifdef SGI_IRIX
  /* open sgi audio_device */
  if ((use_stdout) || (use_alt_device))
  {
    if (use_stdout)
      audio_file = fileno(stdout);
    else audio_file = open(audio_file_arg, O_WRONLY|O_NDELAY);
    if (audio_file == -1)
    {
      perror("Could not open audio device");
      exit(1);
    } 
  }  
  else
  {
     sgi_at = (AUDIO_T)AUDIO_open();
     if (!sgi_at) 
     {
       fprintf(stderr, "AUDIO_open returned null\n");
       exit(1);
     }
  }

#else /* anything but SGI */
  /* open audio device */
  if (use_stdout)
    audio_file = fileno(stdout);
  else audio_file = open(audio_file_arg, O_WRONLY|O_NDELAY);
  if (audio_file == -1)
  {
    perror("Could not open Audio Device");
    exit(1);
  }
#endif



/* Socket initialization ------------------------------------------- */

/* open sockets for sending and receiving packets */
  sock1 = issue_socket();  /* send messages on this socket */
  sock2 = issue_socket();  /* receive data on this socket */

  memset(&sin, '\0', sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = INADDR_ANY;
  bind_success = bind (sock1, (struct sockaddr *) &sin, sizeof(sin));
  if (bind_success) perror("bind - socket 1 port");

  build_request_pack(port_arg);
  send_request_pack(sock1, REQUEST);

  /* set socket address structure to receive data from the host */
  memset(&sin, '\0', sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port_arg);
  bind_success = bind (sock2, (struct sockaddr *) &sin, sizeof(sin));
  if (bind_success) perror("bind - socket 2 port");

 
/* Install Signal handlers --------------------------------------- */

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGTSTP, signal_handler);



/* Connect to server ----------------------------------------------- */

  renotify_count = 0;
  tv.tv_sec = 1; tv.tv_usec = 0;
  while (renotify_count < 6) {
    FD_ZERO(&fds);
    FD_SET(sock2, &fds);
    select(sock2+1, &fds, NULL, NULL, &tv);
    if (FD_ISSET(sock2, &fds))
      break;

    /* else */
    renotify_count++;
    send_request_pack(sock1, REQUEST);
  }

  if (!FD_ISSET(sock2, &fds)) {
    fprintf(stderr, "\n     CR1: No response from server.\n");
    signal_handler(1);
    exit(1);
  }


/* set up circular_buffer -------------------------------------------- */
#ifdef USE_CIRCULAR_BUFFER
   cb = new_circular_buffer (CIRC_BUFF_SIZE);
/*
   let the receives timeout, so that we can do some work on the side
   tv.tv_sec = 0;
   tv.tv_usec = 1000;
   setsockopt (sock2, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (struct timeval));
*/
#endif /* USE_CIRCULAR_BUFFER */


/* This is main loop ------------------------------------------------- */

  count = 0;
  while(1)
  {

    /* check to make sure that the shell that lauched the program is still
       around.  A parent process id of 1 could mean that the listener has
       logged off */

    pp = getppid();
    if (pp == 1)
      signal_handler(1);  /* this will exit */


    /* check to see if the server needs to be re-notified of our existance */
    if (count >= RETRANSMIT_INTERVAL)
    {
      count = 0;
      send_request_pack(sock1, REQUEST);
    }
    count++;


    len = sizeof(sin);
    n = recvfrom(sock2, buffer, BUFSZ, 0, (struct sockaddr *) &sin, &len);
    if (n < 0)
    {
      int norr = errno;
      fprintf (stderr, "recvfrom error: %d %s", norr, strerror (norr));
      exit(1);
    }

    i = sizeof(HEADER);
    /* authentication */
    ret = strncmp(HEADER, buffer, i);
    if (ret)
    {
      fprintf(stderr,"Packet with invalid header\n");
      continue;
    }

    if (buffer[i] == TEXT)            
    {
      bcopy(buffer+i+1, &x, sizeof(x));
      data_length = ntohl(x);  /* ignore for now, we'll use strings in broadcast */
      fprintf(stderr,"CR1 TEXT MESSAGE: ");
      fprintf(stderr, "%s\n", buffer+i+1+4);    

      continue;
    }

    if (buffer[i] == AUDIO_ULAW)
    {
      bcopy(buffer+i+1, &x, sizeof(x));
      data_length = ntohl(x);

#ifdef DO_RATE_CHANGE
printf ("orig data length = %d ", data_length);
      data_length = ulaw_change_rate (buffer+i+1+4, data_length, 0.08);
printf ("new data length = %d \n", data_length);
#endif /* DO_RATE_CHANGE */
#ifdef USE_CIRCULAR_BUFFER
      /* copy data into circular buffer */
      circular_buffer_add_to (cb, data_length, buffer+i+1+4);

      /* see if we need to queue up more data before playing */
      if (PRELOAD_SIZE > circular_buffer_content_length (cb)) {
         if (10 < preload) continue;
      } else {
         preload = 0;
      }

      /* if we got to here, we have data to feed to audio device */
      {
         size_t len = 0;
         char * buffptr = 0x0;
         int retval = 0;
         
         /* try to fill up the audio device */
         while (1) {
            len = circular_buffer_get_chunk (cb, &buffptr, 11000);
            if (0 >= len) {
               preload ++;
               break;
            }
#ifdef SGI_IRIX
           if (!use_alt_device) 
              AUDIO_play(sgi_at, buffptr, len);
            else 
#endif /* SGI_IRIX */
      
            /* !! don't commit the circ buff read until we are 
             * sure the audio write succeeded !! */
            retval = write (audio_file, buffptr, len);
            if (-1 == retval && EAGAIN == errno) break;
            circular_buffer_commit_get (cb, len);
         }
      }
      continue;
      

#else /* USE_CIRCULAR_BUFFER */
#ifdef SGI_IRIX
      if (!use_alt_device)
        AUDIO_play(sgi_at, buffer+i+1+4, data_length);
      else
      {
        total = 0;
        while (total < data_length)
          total += write(audio_file, buffer+i+1+4+total, data_length-total);
      }


#else /* anything but sgi */
      total = 0;
      while (total < data_length)
        total += write(audio_file, buffer+i+1+4+total, data_length-total);
#endif
#endif /* USE_CIRCULAR_BUFFER */

      continue;
    }

    if (buffer[i] == TERMINATE)
    {
      fprintf(stderr, "Broadcaster has terminated connection\n");
#ifdef SGI_IRIX
  if ((!use_alt_device) && (!use_stdout))
    AUDIO_close(sgi_at);
  else close(audio_file);
#else 
      close(audio_file);
#endif
      close(sock1);
      close(sock2);
      exit(1);
    }
  } /* end WHILE */

  return;
}



