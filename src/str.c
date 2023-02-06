#include "str.h"

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

Str str_mk(size_t size, const char *pointer) {
  return (Str) {
    .size = size,
    .at = pointer
  };
}
