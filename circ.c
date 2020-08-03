
/* 
 * FILE:
 * circ.c
 *
 * FUNCTION:
 * Quick and dirty implementation of circular buffer (fifo).
 * Coded in OO style (should be easy to convert to C++).
 * See header file for additional documentation.
 *
 * HISTORY:
 * Linas Vepstas February 1997
 */

#include "circ.h"


struct circular_buffer * new_circular_buffer (size_t fifo_len) {
   struct circular_buffer * self = 
         (struct circular_buffer *) malloc (sizeof (struct circular_buffer));

   self -> size = fifo_len;
   self -> bottom = malloc (fifo_len);
   self -> top = self -> bottom + fifo_len;
   self -> head = self -> bottom;
   self -> tail = self -> bottom;
   self -> content_length = 0;
}

void circular_buffer_add_to (struct circular_buffer *self, size_t len, char *buff) 
{
   /* see if there is room, if no room, then do nothing */
   if (self->size - self->content_length < len) {
      perror ("no room in circular buffer \n");
      return;
   }
   
   /* if there is room to add without wrapping, add */
   if (self->head + len < self->top) { 

      memcpy (self->head, buff, len);
      self -> head += len;
   } 
   /* else have to wrap around */
   else {
      size_t at_top = self-> top - self->head;
      memcpy (self->head, buff, at_top);
      buff += at_top;
      memcpy (self->bottom, buff, len-at_top);
      self -> head = self->bottom + len-at_top;
   }
      
   self -> content_length += len;
/*
   printf ("added; circ buff now contains %d bytes, and %d bytes are free \n", 
      self -> content_length, self -> size - self->content_length);
*/
}


size_t circular_buffer_content_length (struct circular_buffer *self) {
   return (self -> content_length);
}

size_t circular_buffer_get_chunk (struct circular_buffer *self, 
                                  char ** pbuffptr, size_t len) 
{
   size_t bytes_to_top = self->top - self->tail;

   /* see if we have data to return */
   if (self->content_length < len) {
      len = self->content_length;
   }

   /* see if we have to wrap */
   if (bytes_to_top < len) { 
      len = bytes_to_top;
   }

   *pbuffptr = self -> tail;
/*
   printf ("got: circ buff tail 0x%x \n", self->tail); 
*/
   return len;
}

void circular_buffer_commit_get (struct circular_buffer *self, size_t len) 
{
   size_t bytes_to_top = self->top - self -> tail;

   self -> content_length -= len;

   /* see if we've drained the buffer */
   if (self->content_length < 0) {
      self -> content_length = 0;
   }

   /* see if we have to wrap */
   if (bytes_to_top > len) { 
      self->tail += len;
   } else {
      self->tail = self -> bottom + len - bytes_to_top;
   }

/*
   printf ("removed; circ buff now contains %d bytes, and %d bytes are free \n", 
      self -> content_length, self -> size - self->content_length);
*/
}

/* ================ end of file ============ */
