/* init.c (C source)
 * Implementation for init.h
 */

//****************************************************************
// Dependencies

#include"./init.h"

//****************************************************************
// Static functions and function-macros

/* Write default values to the init context */
static void init_default(InitCtx *ctx);

//****************************************************************
// Functions and function-macros

void init(InitCtx *ctx, Args *a) {
  init_default(ctx);
  // read options
  size_t n = 0;
  arewind(a);
  while (!aeos(a) && !aerror(a)) {
    const Arg *arg = aget(a);
    if (arg->alpha == 'o') {
      ctx->output_option = true;
      ctx->output_filename = arg->value;
    } else if (arg->alpha == '\0') {
      n++;
    }
  }
  // read input filenames
  ctx->input_filenames = sva_alc(n);
  if (sva_err(ctx->input_filenames)) {
    // TODO: print the error message and exit the progam
    return;
  }
  size_t i = 0;
  arewind(a);
  while (!aeos(a) && !aerror(a)) {
    const Arg *arg = aget(a);
    if (arg->alpha == '\0') {
      sva_set(ctx->input_filenames, i, arg->value);
      i++;
    }
  }
}

void init_free(InitCtx *ctx) {
  sva_free(ctx->input_filenames);
}

void init_default(InitCtx *ctx) {
  ctx->output_option = false;
  ctx->output_filename = (Sv) SV_SZ("./a.out");
}
