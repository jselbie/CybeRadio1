
/* 
 * FILE:
 * circ.h
 *
 * FUNCTION:
 * Quick and dirty implementation of a circular buffer (fifo).
 * Coded in OO style (should be easy to convert to C++).
 *
 * METHODS:
 * new_circular_buffer():  create new buffer. The argument is the size
 *    of the buffer to be created.
 * 
 * circular_buffer_add_to():  add data to buffer.  This method copies
 *    the indicated number of bytes into the circular buffer.
 * 
 * circular_buffer_content_length(): returns number of bytes stored
 *    in the buffer.
 * 
 * circular_buffer_get_chunk(): return pointer to at most "len" bytes.
 *   The returned value is the number of actual bytes returned.
 *   This method does NOT do any copying: it simply returns a pointer
 *   that points into the cirular queue. Over-write stuff at your own
 *   risk!  This method does NOT update the fifo size or any internal
 *   pointers; repeated calls to this method will return the same
 *   values.  Use the commit_get() method to confirm that data can be
 *   freed.
 * 
 * circular_buffer_commit_get(): this method will confirm that data 
 *    has been read. That, is, it will update internal structures to 
 *    show that space has been freed.  This allows a two-phase
 *    read-commit algorithm to be used when reading from the buffer.
 *
 * HISTORY:
 * created by Linas Veptas February 1997
 */

#ifndef __CIRC_BUFF_H__
#define __CIRC_BUFF_H__

#include <stdlib.h>

struct circular_buffer {
   /* private members */
   char * bottom;   
   char * top;
   char * head;
   char * tail;
   size_t size;
   size_t content_length;
};

/* methods */
extern struct circular_buffer * new_circular_buffer (size_t);
extern void circular_buffer_add_to (struct circular_buffer *, size_t, char *);
extern size_t circular_buffer_content_length (struct circular_buffer *);
extern size_t circular_buffer_get_chunk (struct circular_buffer *, char **, size_t);
extern void circular_buffer_commit_get (struct circular_buffer *, size_t);

#endif /* __CIRC_BUFF_H__ */
