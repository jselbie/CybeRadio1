
/*
 * FILE:
 * rate.h
 *
 * FUNCTION:
 * Change the sampling rate of ulaw audio data.
 * 
 *
 * HISTORY:
 * Linas Vepstas February 1997
 */


#ifndef __RATE_H__
#define __RATE_H__

/* positive rates slow things down */

extern int ulaw_change_rate (unsigned char * ubuff, int len, float frate);

#endif /* __RATE_H__ */


/* =================== end of file ==================== */

