
/*
 * FILE:
 * rate.c
 *
 * FUNCTION:
 * Change the rate
end points are not well handled.
 *
 * HISTORY:
 * Linas Vepstas February 1997
 */

#define LEN 50000

#include <math.h>

#include "libst.h"
#include "rate.h"

/* positive rates slow things down */
int change_rate_linear (int * obuff, int * ibuff, 
                        int ilen, float frate) 
{

   int rate = frate * 16384.0;

   int alpha = 16384;
   int beta = 0;
   int iin = 0;
   int iout = 0;

   /* change sampling rate */
   while (ilen-1 > iin) {
      obuff[iout] = alpha * ibuff[iin] + beta * ibuff[iin+1];
      obuff[iout] >>= 14;

      beta += rate;
      alpha -= rate;
      if (0 > alpha) {
         alpha += 16384;
         beta -= 16384;
         iin --;
      }

      if (0 > beta) {
         beta += 16384;
         alpha -= 16384;
         iin ++;
      }

      iin ++;
      iout ++;
   }

   return iout;
}

int change_rate_cubic (int * obuff, int * ibuff, 
                        int ilen, float frate) 
{
   int rate = frate * 32.0;

   short xmm = -64;
   short xm = -32;
   short x = 0;
   short xp = 32;

   int iin = 1;
   int iout = 1;
   int tmpc, tmpo;

   /* first point is just a copy */
   obuff[0] = ibuff[0];

   /* change sampling rate */
   while (ilen-2 > iin) {
      
      tmpc = xm * ibuff[iin] - x * ibuff[iin+1];
      tmpc *= xp*xmm;
      tmpc /= 2;

      tmpo = xp * ibuff[iin+2] - xmm * ibuff[iin-1];
      tmpo *= x*xm;
      tmpo /= 6;

      obuff[iout] = tmpc+tmpo;
      obuff[iout] >>= 15;

      xmm += rate;
      xm += rate;
      x += rate;
      xp += rate;

      if (32 < x) {
         xmm -= 32;
         xm -= 32;
         x -= 32;
         xp -= 32;
         iin --;
      }

      if (0 > x) {
         xmm += 32;
         xm += 32;
         x += 32;
         xp += 32;
         iin ++;
      }

      iin ++;
      iout ++;
   }

   /* last point is just a copy */

   return iout;
}

int change_rate_bspline (int * obuff, int * ibuff, 
                        int ilen, float frate) 
{

   int rate = frate * 32.0;

   int alpha = 32;
   int beta = 0;
   int iin = 1;
   int iout = 1;
   int tmpc, tmpo;

   /* first point is just a copy */
   obuff[0] = ibuff[0];

   /* change sampling rate */
   while (ilen-2 > iin) {
      
      tmpo = alpha*alpha*alpha * ibuff[iin-1];
      tmpo += beta*beta*beta * ibuff[iin+2];

      tmpc = 3*(1024 + alpha*beta); 
      tmpo += (32 + alpha * tmpc) * ibuff[iin];
      tmpo += (32 + beta * tmpc) * ibuff[iin+1];
      tmpo /= 6;

      obuff[iout] = tmpo;
      obuff[iout] >>= 15;

      beta += rate;
      alpha -= rate;
      if (0 > alpha) {
         alpha += 32;
         beta -= 32;
         iin --;
      }

      if (0 > beta) {
         beta += 32;
         alpha -= 32;
         iin ++;
      }

      iin ++;
      iout ++;
   }

   return iout;
}

int change_rate_bezier (int * obuff, int * ibuff, 
                        int ilen, float frate) 
{

   int rate = frate * 32.0;

   int alpha = 32;
   int beta = 0;
   int iin = 1;
   int iout = 1;
   int tmpc, tmpo;

   /* first point is just a copy */
   obuff[0] = ibuff[0];

   /* change sampling rate */
   while (ilen-2 > iin) {
      
      tmpo = alpha * ibuff[iin-1];
      tmpo += beta * ibuff[iin+2];
      tmpo *= 3 * alpha * beta;

      tmpo += alpha* alpha*alpha* ibuff[iin];
      tmpo += beta * beta * beta * ibuff[iin+1];

      obuff[iout] = tmpo;
      obuff[iout] >>= 15;

      beta += rate;
      alpha -= rate;
      if (0 > alpha) {
         alpha += 32;
         beta -= 32;
         iin --;
      }

      if (0 > beta) {
         beta += 32;
         alpha -= 32;
         iin ++;
      }

      iin ++;
      iout ++;
   }

   return iout;
}

int change_rate_psinc (int * obuff, int * ibuff, 
                       int ilen, float frate) 
{
   int rate = frate * 16.0;

   short xmm = -32;
   short xm = -16;
   short x = 0;
   short xp = 16;

   int ymm, ym, y, yp;

   int iin = 1;
   int iout = 1;

   /* first point is just a copy */
   obuff[0] = ibuff[0];

   /* change sampling rate */
   while (ilen-2 > iin) {
      
      ymm=xmm*xmm; ymm=262144-1280*ymm+ymm*ymm; ymm >>= 5;
      ym=xm*xm; ym=262144-1280*ym+ym*ym; ym >>= 5;
      y=x*x; y=262144-1280*y+y*y; y >>= 5;
      yp=xp*xp; yp=262144-1280*yp+yp*yp; yp >>= 5;
      

      obuff[iout] = yp*ibuff[iin-1] + y*ibuff[iin] + ym*ibuff[iin+1] + ymm*ibuff[iin+2];

      obuff[iout] >>= 13;

      xmm += rate;
      xm += rate;
      x += rate;
      xp += rate;

      if (16 < x) {
         xmm -= 16;
         xm -= 16;
         x -= 16;
         xp -= 16;
         iin --;
      }

      if (0 > x) {
         xmm += 16;
         xm += 16;
         x += 16;
         xp += 16;
         iin ++;
      }

      iin ++;
      iout ++;
   }

   /* last point is just a copy */

   return iout;
}

/* instead of using the low performance system sin() routine,
   the high-performance spherical bessel recursion relations should
   be used. */
double sinc (double x) {
   if ((1.0e-5 > x) && (1.0e-5 > -x)) return (1.0-x*x/6.0);
   return (sin(x) / x);
}

/*
 * Do some filtering while we are at it: 
 * Cutoff frequency is 0.85 of 8 KHz
 */
#define CUTOFF_FREQ 0.85 * M_PI

int change_rate_sinc (int * obuff, int * ibuff, 
                        int ilen, float frate) 
{

   double x = 0.0;
   double ymm, ym, y, yp;
   double smm, sm, s, sp;


   int iin = 1;
   int iout = 1;

   /* first point is just a copy */
   obuff[0] = ibuff[0];

   /* change sampling rate */
   while (ilen-2 > iin) {
      
      ymm = (x-2.0) * CUTOFF_FREQ; ymm = sinc (ymm);
      ym = (x-1.0) * CUTOFF_FREQ; ym = sinc (ym);
      y = x * CUTOFF_FREQ; y = sinc (y);
      yp = (x+1.0) * CUTOFF_FREQ; yp = sinc (yp);
      
      sp = ibuff[iin-1];
      s = ibuff[iin];
      sm = ibuff[iin+1];
      smm = ibuff[iin+2];
      obuff[iout] = yp*sp + y*s + ym*sm + ymm*smm;

      x += frate;

      if (1.0 < x) {
         x -= 1.0;
         iin --;
      }

      if (0.0 > x) {
         x += 1.0;
         iin ++;
      }

      iin ++;
      iout ++;
   }

   /* last point is just a copy */

   return iout;
}

int ulaw_change_rate (unsigned char * ubuff, int len, float frate) {

   int ibuff[LEN];
   int obuff[LEN];
   int i;
   if (0.0 == frate) return len;

   /* convert from ulaw to linear */
   for (i=0; i< len; i++) {
      ibuff[i] = st_ulaw_to_linear_slow (ubuff[i]);
   }

/*
   len = change_rate_linear (obuff, ibuff, len, frate);
   len = change_rate_cubic (obuff, ibuff, len, frate);
   len = change_rate_bspline (obuff, ibuff, len, frate);
   len = change_rate_bezier (obuff, ibuff, len, frate);
   len = change_rate_psinc (obuff, ibuff, len, frate);
*/
   len = change_rate_sinc (obuff, ibuff, len, frate);

   /* convert from linear back to ulaw */
   for (i=0; i< len; i++) {
      ubuff[i] = st_linear_to_ulaw (obuff[i]);
   }

   return len;
}

/* =================== end of file ==================== */

