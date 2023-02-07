#include "str.h"

#include "stdio.h"
/* included to print to stderr */

#include "stdlib.h"
/* included for memory allocation */

size_t nts_chr(const char *string, char character) {
  const char *begin = string;
  while (*string != '\0' && *string != character) {
    string++;
  }
  return string - begin;
}

size_t nts_len(const char *string) {
  const char *begin = string;
  while (*string != '\0') {
    string++;
  }
  return string - begin;
}

CharV cv_mk(size_t size, const char *pointer) {
  return (CharV) {
    .size = size,
    .at = pointer
  };
}

CharV cv_cut(CharV string, size_t count) {
  if (count > string.size) {
    return cv_mk(0, string.at + string.size);
  }
  return cv_mk(string.size - count, string.at + count);
}

bool cv_eq(CharV string, CharV another) {
  if (string.size != another.size) {
    return false;
  }
  for (size_t i = 0; i < string.size; i++) {
    if (string.at[i] != another.at[i]) {
      return false;
    }
  }
  return true;
}

size_t cv_write(CharV string, FILE *stream) {
  return fwrite(string.at, sizeof(char), string.size, stream);
}

CharVV cvv_mk(size_t size, const CharV *pointer) {
  return (CharVV) {
    .size = size,
    .at = pointer
  };
}

CharVV cvv_cva(CharVA array) {
  if (!array.valid) {
    return cvv_mk(0, NULL);
  }
  return cvv_mk(array.size, array.at);
}

CharVA cva_new(size_t size) {
  CharVA array = (CharVA) {
    .valid = 1,
    .size = size,
    .at = NULL
  };
  if (size == 0) {
    /* the empty array is valid */
    return array;
  }
  array.at = calloc(size, sizeof(CharV));
  if (array.at == NULL) {
    array.valid = 0;
    fprintf(stderr, "%s%s\n", __func__,
      ": error: fail to allocate memory");
  }
  return array;
}

void cva_delete(CharVA array) {
  if (!array.valid) {
    return;
  }
  free(array.at);
}
