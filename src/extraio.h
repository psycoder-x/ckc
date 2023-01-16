/* extraio.h (C header)
 * Extended and improved C input/output system
 */

#ifndef EXTRAIO_H_INCLUDED
#define EXTRAIO_H_INCLUDED 1

//****************************************************************
// Dependencies

#include<stdio.h>
#include"./strview.h"

//****************************************************************
// Functions and function-macros

/* Print the string sv to file f and return true on success */
bool fputsv(Sv sv, FILE *f);

/* Print the string sv to file f and return true on success.
 * Add spaces on the right side to match boxlen exactly.
 * It creates left align, but only if the sv length less than boxlen
 */
bool fputsv_la(Sv sv, FILE *f, size_t boxlen);

/* Print the character c to file f, n times */
void fputc_x(char c, FILE *f, size_t n);

#endif /* EXTRAIO_H_INCLUDED */
