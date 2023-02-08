/* str.h is a library to work with a text */

/* dictionary:
----------------------------
nts  | null-terminated string
*/

#ifndef STR_H
#define STR_H

#include "stddef.h"
/* included for size_t */

#include "stdbool.h"
/* included for bool */

#include "stdio.h"
/* included for io */

typedef struct CharV {
  size_t size;
  const char *at;
} CharV;
/* view of chars */

typedef struct CharVV {
  size_t size;
  const CharV *at;
} CharVV;
/* view of view of chars */

typedef struct CharVA {
  bool valid; /* if false, an error was printed and you need to exit */
  size_t size;
  CharV *at;
} CharVA;
/* array of view of chars */

size_t nts_chr(
  const char *string, /* null-terminated string */
  char character /* character to be located */
);
/* returns an index of the first occurrence of <character> in <string>.
returns the length of <string> if <character> is not found */

size_t nts_len(
  const char *string /* null-terminated string */
);
/* returns the length of <string> */

CharV cv_mk(
  size_t size, /* number of characters */
  const char *pointer /* pointer to characters */
);
/* returns a view of characters */

#define CV_NTS(NTS) { .size = (sizeof(NTS) - 1), .at = NTS }
/* (macro-function) returns a char view of null-terminated string */

CharV cv_cut(
  CharV string, /* char view to be cutted */
  size_t count /* number of characters to remove */
);
/* removes the first <count> characters from <string>.
if <count> is greater than <string.size>, returns the empty CharV */

CharV cv_rcut(
  CharV string, /* char view to be cutted */
  size_t count /* number of characters to remove */
);
/* removes the last <count> characters from <string>.
if <count> is greater than <string.size>, returns the empty CharV */

CharV cv_get(
  CharV string, /* char view to be cutted */
  size_t count /* number of resulting characters */
);
/* returns the first <count> characters from <string>.
if <count> is greater than <string.size>, returns <string> */

bool cv_eq(
  CharV string, /* char view */
  CharV another /* another char view */
);
/* returns true if two strings are equal */

size_t cv_write(
  CharV string, /* char view */
  FILE *stream /* stream with write access */
);
/* writes characters from <string> to <stream>.
returns the number of characters written successfully */

CharVV cvv_mk(
  size_t size, /* number of char views */
  const CharV *pointer /* pointer to char views */
);
/* returns a view of char views */

CharVV cvv_cva(
  CharVA array /* array to view */
);
/* returns a view of the array of char views */

CharVA cva_new(
  size_t size /* number of characters to allocate */
);
/* returns an array of char views,
not allocates memory when <size> is 0 */

void cva_delete(
  CharVA array /* an array returned from a "new" function */
);
/* frees the memory used by <array> */

#endif /* !STR_H */
