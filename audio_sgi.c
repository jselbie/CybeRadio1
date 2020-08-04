#if 0
    Cyber Radio 1 - Internet Radio Client/Server System - SGI Audio code
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


#include "tables.h" /* 16->8 bit audio conversion stuff */
#include <audio.h>
#include <stdio.h>
#include "audio_sgi.h"


/* We need a big audio buffer to get the 16->8 bit conversions to work */
static short tmpShortBuffer[0x64000];

AUDIO_T AUDIO_open() 
{
 ALconfig audio_port_config;
 long pvbuf[2];
 long buflen;
 ALport *audio_port=(ALport *)malloc(sizeof(ALport)*2);

 /*
  * set output sample rate
  */
 pvbuf[0] = AL_OUTPUT_RATE;
 pvbuf[1] = AL_RATE_8000;
 buflen   = 2;
 ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);

 /*
  * set input sample rate
  */
 pvbuf[0] = AL_INPUT_RATE;
 pvbuf[1] = AL_RATE_8000;
 buflen   = 2;
 ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);

 /*
  * configure and open the audio port 
  */
 audio_port_config=ALnewconfig();
 ALsetwidth(audio_port_config,AL_SAMPLE_16);
 ALsetchannels(audio_port_config,AL_MONO);
 ALsetqueuesize(audio_port_config, /* guess */ 2400);
 audio_port[0]= ALopenport("ms", "r", audio_port_config);

 /* we end up opening it twice, since we need both input and output */
 audio_port_config=ALnewconfig();
 ALsetwidth(audio_port_config,AL_SAMPLE_16);
 ALsetchannels(audio_port_config,AL_MONO);
 ALsetqueuesize(audio_port_config, /* guess */ 2400);
 audio_port[1]= ALopenport("ms", "w", audio_port_config);
 return audio_port;
}

void AUDIO_close(AUDIO_T ap)
{
  ALcloseport(ap[0]);
  ALcloseport(ap[1]);
  free(*ap);
  *ap=NULL;
}

int AUDIO_play(AUDIO_T ap,unsigned char *buffer,unsigned int length)
{
  int i;
  if (length>0x64000) {
    fprintf(stderr,"OS-INTERFACE: internal buffer overflow in AUDIO_play!\n\tlength must be less than %d\n",length);
    /* PROCESS_exit(1); */
  }
  AUDIO_8to16bit(buffer,tmpShortBuffer,length);
  ALwritesamps(ap[1],tmpShortBuffer,(long)length);
  return 0;
}

int AUDIO_record(AUDIO_T ap,unsigned char *buffer,unsigned int length)
{

  if (length>0x64000) {
    fprintf(stderr,"OS-INTERFACE: internal buffer overflow in AUDIO_play!\n\tlength must be less than %d\n",length);
    /* PROCESS_exit(1); */
  }
  ALreadsamps(ap[0],tmpShortBuffer, (long)length);
  AUDIO_16to8bit(tmpShortBuffer,buffer,length);
  return 0;
}


int AUDIO_valid(AUDIO_T ah)
{
  if (ah) return 1;
  return 0;
}

void AUDIO_8to16bit(unsigned char *mulawBuf,short *shortBuf,
                    unsigned int length)
{
  int i;
  /* need a conversion to 16 bit samps */
  for (i=0; i<length; ++i) {
    shortBuf[i]=ulawValues[(int)mulawBuf[i]];
  }
}

/* dummy place holder for compile until we can get this fixed */
void AUDIO_16to8bit(short *s,unsigned char *c,unsigned int i)
{;}

