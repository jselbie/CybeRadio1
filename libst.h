/* 
 * libst.h - portable sound tools library
**
** Craig Reese: IDA/Supercomputing Research Center
** Joe Campbell: Department of Defense
** 29 September 1989
*/

extern unsigned char st_linear_to_ulaw (int sample);
extern int st_ulaw_to_linear_slow (unsigned char ulawbyte);

/* ===================== end of file ================  */
