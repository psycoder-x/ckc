/* strview.h (C header)
 * Pure and safe implementation of string view to avoid:
 *  - memory allocation;
 *  - memory leak;
 *  - null-terminated strings;
 *  - index out of bounds;
 *  - jump instructions.
 * Warning: functions do not check for NULL
 */

#ifndef STRVIEW_H_INCLUDED
#define STRVIEW_H_INCLUDED 1

//****************************************************************
// Dependencies

#include<string.h>
#include<stdbool.h>
#include<limits.h>
#include<stdint.h>

//****************************************************************
// Data types

/* String view */
typedef struct Sv {
  /* Read-only, non-free, non-null-terminated string */
  const char *data;
  /* The number of characters at the pointer .data */
  size_t size;
} Sv;

//****************************************************************
// Functions and function-macros

/* A null-terminated string to Sv (static) */
#define SV_SZ(SZ) { .data = SZ, .size = sizeof(SZ) - 1 }

/* An array to Sv */
Sv       sv_arr   (const char *a, size_t n);
/* A null-terminated string to Sv */
Sv       sv_sz    (const char *sz);
/* Get length */
size_t   sv_len   (Sv s);
/* Check if string is empty */
bool     sv_mty   (Sv s);
/* Check if two strings are equal */
bool     sv_equ   (Sv s1, Sv s2);
/* Remove the first n characters */
Sv       sv_cut   (Sv s, size_t n);
/* Get the first n characters */
Sv       sv_get   (Sv s, size_t n);
/* Character at index */
char     sv_at    (Sv s, size_t i);

//****************************************************************
// Variables and const-macros

/* An empty string */
const Sv SV_NULL;

#endif /* STRVIEW_H_INCLUDED */
