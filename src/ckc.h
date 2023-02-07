#ifndef CKC_H
#define CKC_H

#include "stddef.h"
/* included for size_t */

#include "stdbool.h"
/* included for bool */

#include "str.h"

typedef struct Ckc {
  bool valid;
  /* if false, an error was printed and you need to exit */

  bool help;
  /* show help (-h) */

  bool version;
  /* show version (-v) */

  bool pp_only;
  /* preprocessor only (-p) */

  bool com_only;
  /* compile only; do not assemble and link (-c) */

  CharV ofile;
  /* output file (-o) */

  CharVA idirs;
  /* include directories (-i) */

  CharVA ifiles;
  /* input files */
} Ckc;
/* compiler settings */

Ckc ckc_new_args(
  CharVV args /* argc and argv probably */
);
/* reads command-line arguments and parse settings.
returns compiler settings, where some memory was allocated.
you need to call ckc_delete when you are done */

void ckc_delete(
  Ckc ckc /* initialized compiler settings */
);
/* frees the memory that was allocated */

CharV ckc_help();
/* returns a char view of the help message */

CharV ckc_version();
/* returns a char view of the version information */

#endif /* CKC_H */
