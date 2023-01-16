/* extraio.c (C source)
 * Implementation for extraio.h
 */

//****************************************************************
// Dependencies

#include"./extraio.h"

//****************************************************************
// Functions and function-macros

bool fputsv(Sv sv, FILE *f) {
  return fwrite(sv.data, sv.size, 1, f) == sv.size;
}

bool fputsv_la(Sv sv, FILE *f, size_t boxlen) {
  size_t len = sv_len(sv);
  if (len >= boxlen) {
    return fputsv(sv, f);
  }
  bool success = fputsv(sv, f);
  fputc_x(' ', f, boxlen - len);
  return success;
}

void fputc_x(char c, FILE *f, size_t n) {
  for (size_t i = 0; i < n; i++) {
    fputc(c, f);
  }
}
