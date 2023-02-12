#include "str.h"

#include "stdio.h"
/* included to print to stderr */

#include "stdlib.h"
/* included for memory allocation */

#define UNIQ restrict

void mem_copy(char *UNIQ dst, const char *UNIQ src, size_t count) {
  while (count != 0) {
    *dst = *src;
    dst++;
    src++;
    count--;
  }
}

bool c_isspace(char character) {
  return character == '\n' || character == '\t' || character == '\0'
      || character == '\v' || character == '\r' || character == ' ';
}

bool c_isalpha(char character) {
  return ('A' <= character && character <= 'Z')
      || ('a' <= character && character <= 'z')
      || (unsigned char)character > 127;
}

bool c_isdigit(char character) {
  return '0' <= character && character <= '9';
}

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

CharV cv_rcut(CharV string, size_t count) {
  if (count > string.size) {
    return cv_mk(0, string.at);
  }
  return cv_mk(string.size - count, string.at);
}

CharV cv_get(CharV string, size_t count) {
  if (count > string.size) {
    return string;
  }
  return cv_mk(count, string.at);
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

const char *cv_chr(CharV string, char character) {
  for (size_t i = 0; i < string.size; i++) {
    if (string.at[i] == character) {
      return &string.at[i];
    }
  }
  return NULL;
}

const char *cv_rchr(CharV string, char character) {
  for (size_t i = 0, j = string.size - 1; i < string.size; i++, j--) {
    if (string.at[j] == character) {
      return &string.at[j];
    }
  }
  return NULL;
}

size_t cv_write(CharV string, FILE *stream) {
  return fwrite(string.at, sizeof(char), string.size, stream);
}

CharA ca_new(size_t size) {
  CharA array = (CharA) {
    .valid = 1,
    .size = size,
    .at = NULL
  };
  if (size == 0) {
    /* the empty array is valid */
    return array;
  }
  array.at = calloc(size, sizeof(char));
  if (array.at == NULL) {
    array.valid = 0;
    fprintf(stderr, "%s%s\n",
      __func__, ": error: fail to allocate memory");
  }
  return array;
}

CharA ca_new_cat(CharV string1, CharV string2) {
  CharA cat = ca_new(string1.size + string2.size + 1);
  if (!cat.valid) {
    return cat;
  }
  cat.size--;
  cat.at[cat.size] = '\0';
  mem_copy(cat.at, string1.at, string1.size);
  mem_copy(cat.at + string1.size, string2.at, string2.size);
  return cat;
}

CharV ca_view(CharA array) {
  if (!array.valid) {
    return cv_mk(0, NULL);
  }
  return cv_mk(array.size, array.at);
}

void ca_delete(CharA array) {
  if (array.valid) {
    free(array.at);
  }
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
    fprintf(stderr, "%s%s\n",
      __func__, ": error: fail to allocate memory");
  }
  return array;
}

void cva_delete(CharVA array) {
  if (array.valid) {
    free(array.at);
  }
}
