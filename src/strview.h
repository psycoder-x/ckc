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

#include<stdlib.h>
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

/* String view array */
typedef struct Sva {
  /* Number of elements */
  size_t size;
  /* Maybe non-free, array of string views */
  Sv *data;
} Sva;

//****************************************************************
// Functions and function-macros

/* A null-terminated string to Sv (static) */
#define SV_SZ(SZ) { .data = SZ, .size = sizeof(SZ) - 1 }

/* Create a string view array and allocate memory for n elements */
Sva      sva_alc  (size_t n);
/* Free the memory used by string view array. Return empty array */
Sva      sva_free (Sva a);
/* Construct a string view array */
Sva      sva_con  (size_t n, Sv *a);
/* String view at index */
Sv       sva_at   (Sva a, size_t i);
/* Set string view at index */
void     sva_set  (Sva a, size_t i, Sv s);
/* Check if the array has errors */
bool     sva_err  (Sva a);
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
