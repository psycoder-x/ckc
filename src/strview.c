/* strview.c (C source)
 * Implementation for strview.h
 */

//****************************************************************
// Dependencies

#include"./strview.h"

//****************************************************************
// Functions and function-macros

Sv sv_arr(const char *a, size_t n) {
  return (Sv) { .data = a, .size = n };
}

Sv sv_sz(const char *sz) {
  return (Sv) { .data = sz, .size = strlen(sz) };
}

size_t sv_len(Sv s) {
  return s.size;
}

bool sv_mty(Sv s) {
  return s.size == 0;
}

bool sv_equ(Sv s1, Sv s2) {
  if (s1.size != s2.size) {
    return false;
  }
  return memcmp(s1.data, s2.data, s1.size) == 0;
}

Sv sv_cut(Sv s, size_t n) {
  if (s.data == NULL) {
    return s;
  }
  return sv_arr(s.data + n, (n < s.size) ? (s.size - n) : 0);
}

Sv sv_get(Sv s, size_t n) {
  return sv_arr(s.data, (n < s.size) ? n : s.size);
}

char sv_at(Sv s, size_t i) {
  return (i < s.size) ? s.data[i] : '\0';
}

//****************************************************************
// Variables and const-macros

const Sv SV_NULL = SV_SZ("");
