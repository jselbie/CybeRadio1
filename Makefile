#    Cyber Radio 1 - Internet Radio Client/Server System - Makefile
#    Copyright (C) 1995  John Selbie
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#    John Selbie
#    1600 NW 4th Avenue
#    #24
#    Gainesville, FL 32603
#
#    e-mail: jselbie@cis.ufl.edu

#use gcc whenever possible
CC=gcc

#this is a better compiler for solaris
#CC=/opt/SUNWspro/bin/cc

#the sgi compiler isn't bad either
#CC=cc

#libs will usually be blank
LIBS=
AUDIO_LIBS=

#Use this LIBS for Solaris
#LIBS= -lsocket -lnsl

#Use these LIBS for SGI
#AUDIO_LIBS= -laudio

#normal
#AUDIO_OBJ=

#sgi
#AUDIO_OBJ=tables.o audio_sgi.o


#normally, we won't need any DEFS
DEFS=

#Solaris is the exception
#DEFS= -DSOLARIS -DNEED_BCOPY

#uncomment this line if your system doesn't have bcopy
#DEFS= -DNEED_BCOPY


.c.o:
	$(CC) -c $*.c $(DEFS)


all: cr1 server request bconsole terminate talk

cr1: cr1.o $(AUDIO_OBJ)
	$(CC) cr1.o $(AUDIO_OBJ) $(LIBS) -o cr1 $(LIBS) $(AUDIO_LIBS)

server: server.o shm.o
	$(CC) server.o shm.o -o server $(LIBS)

request: request.o shm.o
	$(CC) request.o shm.o -o request $(LIBS)

bconsole: bconsole.o shm.o
	$(CC) bconsole.o shm.o -o bconsole $(LIBS)

terminate: terminate.o
	$(CC) terminate.o $(LIBS) -o terminate $(LIBS)

talk: talk.o
	$(CC) talk.o $(LIBS) -o talk $(LIBS)

tarup:
	tar cvf CyberRadio1.tar audio_sgi.c audio_sgi.h bconsole.c cr1.c cr1.h cr1.py packet.format protocol request.c server.c shm.c shm.h tables.c tables.h talk.c terminate.c wwwdemo.cgi wwwdemo.c Makefile README COPYING INSTALL MISC RUNNING HACKING
	gzip CyberRadio1.tar

clean:
	rm cr1 server request bconsole terminate talk *.o

