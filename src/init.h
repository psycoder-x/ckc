/* init.h (C header)
 * Program initializing stage
 */

#ifndef INIT_H_INCLUDED
#define INIT_H_INCLUDED 1

//****************************************************************
// Dependencies

#include"./argstream.h"
#include"./strview.h"
#include<stdbool.h>

//****************************************************************
// Data types

/* Init context */
typedef struct InitCtx {
  /* If an output option has been triggered */
  bool output_option;
  /* The name of the file to store the program output */
  Sv output_filename;
  /* Listed files are the input of the program */
  Sva input_filenames;
} InitCtx;

//****************************************************************
// Functions and function-macros

/* Initialize the program. Members of the context will be overwritten */
void init(InitCtx *ctx, Args *a);
/* Free the memory used by init context */
void init_free(InitCtx *ctx);

#endif /* INIT_H_INCLUDED */
