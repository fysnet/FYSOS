/*
 *                             Copyright (c) 1984-2020
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  strings.cpp
 *
 *  Last updated: 17 July 2020
 */

#include <string.h>
#include <memory.h>

#include "gui.h"

/*  string()
 *    string = pointer to the string "object"
 *
 *  clears the string object
 *   
 */
void string(struct STRING *string) {
  memset(string, 0, sizeof(struct STRING));
}

/*  string_free()
 *    string = pointer to the string "object"
 *
 *  frees the allocated string and clears the string object
 *   
 */
void string_free(struct STRING *string) {
  if (string->str && string->allocated)
    free((void *) string->str);
  
  memset(string, 0, sizeof(struct STRING));
}

/*  string_set()
 *      string = pointer to the string "object"
 *         str = pointer to a text string to set the string object to
 *         len = len of text (or -1 for us to figure it out)
 *   allocated = set if the text is allocated
 *
 *  sets a string object to the text given
 *
 *  len = length of buffer allocated, or length of fixed buffer sent
 *  if len == -1, get length from string (is not allocated)   
 *
 */
void string_set(struct STRING *string, const char *str, int len, const bool allocated) {
  // if this string obj is already allocated and we are setting it
  //  to another string (allocated or not), need to free the orginal
  //  string buffer first.
  if (string->str && string->allocated && (string->str != str))
    free((void *) string->str);
  
  // if len == -1, buffer is not allocated, it is a fixed "in code/data segment"
  //  block of memory.  Get the size from the string.
  // if len > -1, it is an allocated block of memory.  Len = the size of the allocated
  //  block.
  if (str && (len == -1))
    len = strlen(str) + 1;
  else
    len = 0;
  
  // point our object to the buffer and set the length
  string->str = (char *) str;
  string->len = len;
  string->allocated = allocated;
}

/*  string_copy()
 *      string = pointer to the string "object"
 *         str = pointer to a text string to set the string object to
 *
 *  copies a string to a given textual obj.
 *  if the target textual object (str) is large enough to hold the
 *   new str (allocated or not), just copy it over.  If it is not
 *   large enough and is allocated, enlarge the memory buffer.
 *   if not allocated, allocate a new buffer.
 *
 */
void string_copy(struct STRING *string, const char *str) {
  int len = strlen(str) + 1;
  
  if (len > string->len) {
    len = (len + 511) & ~511; // round up to 512 byte chunks
    if (string->allocated)
      string->str = (char *) realloc(string->str, len);
    else {
      string->str = (char *) calloc(len, 1);
      string->allocated = TRUE;
    }
    string->len = len;
  }
  
  strcpy(string->str, str);
}

/*  string_insert()
 *      string = pointer to the string "object"
 *         loc = index into str to insert char
 *          ch = char to insert
 *
 * inserts a char (ch) at location (loc)
 * if string->free is non zero, will realloc() the string (if needed)
 *  else it is on the programmer to make sure buffer overrun is accounted for.
 *
 */
void string_insert(struct STRING *string, const int loc, const char ch) {
  int len = strlen(string->str);
  
  if (string->allocated && ((len + 2) >= string->len)) {
    string->len += 4096;
    string->str = (char *) realloc(string->str, string->len);
  }
  
  memmove((void *) (string->str + loc + 1), (void *) (string->str + loc), ((len + 1) - loc));
  
  // insert the char
  string->str[loc] = ch;
  
  // make sure we are null terminated
  string->str[len+1] = 0;
}

/*  string_remove()
 *      string = pointer to the string "object"
 *         loc = index into str to remove char
 *
 * removes the char at location (loc)
 *
 */
void string_remove(struct STRING *string, const int loc) {
  int len = strlen(string->str);
  
  if (loc >= len)
    return;
  
  memmove((void *) (string->str + loc), (void *) (string->str + loc + 1), (len - loc));
  
  // make sure we are null terminated
  if (len > 0)
    string->str[len-1] = 0;
}

/*  string_text()
 *      string = pointer to the string "object"
 *         len = pointer to hold length of returned string
 *
 * returns the length of pointer to the string in string object
 *
 */
const char *string_text(const struct STRING *string, int *len) {
  if (len && string->str)
    *len = strlen(string->str);
  
  return string->str;
}
