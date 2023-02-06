
/* dictionary:
----------------------------
nts | null-terminated string
str | string view
*/

#ifndef STR_H
#define STR_H

#include "stddef.h"
/* included for size_t */

typedef struct Str {
  size_t size;
  const char *at;
} Str;
/*
string view
*/

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

Str str_mk(
  size_t size, /* number of characters */
  const char *pointer /* pointer to characters */
);
/* returns a view of the array of characters */

#endif /* !STR_H */
