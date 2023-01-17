/* preinit.h (C header)
 * 
 */

#ifndef PREINIT_H_INCLUDED
#define PREINIT_H_INCLUDED 1

//****************************************************************
// Dependencies

#include"./argstream.h"
#include<stdbool.h>

//****************************************************************
// Data types

/* Preinit context */
typedef struct PreinitCtx {
  /* Indicate if one of the information options appear.
   * If true other options and arguments should be ignored
   * and the program should exit successfully
   * without perform its normal functions.
   */
  bool info_mode;
} PreinitCtx;

//****************************************************************
// Functions and function-macros

/* Everything that needs to be done before initializing the program.
 * Some members of the preinit context will be overwritten
 */
void preinit(PreinitCtx *ctx, Args *a, size_t optc, const Opt *optv);

#endif /* PREINIT_H_INCLUDED */
