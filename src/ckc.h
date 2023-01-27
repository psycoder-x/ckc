#ifndef CKC_H
#define CKC_H

#include"stddef.h"
#include"stdbool.h"

#define VERSION "v0.0.0"

typedef struct State State;
struct State {
  /* arguments */
  char **args;
  int args_n;
  /* preprocessor only (-p) */
  bool pp_only;
  /* compile only; do not assemble or link (-c) */
  bool com_only;
  /* output file (-o) */
  char *ofile;
  /* include dirs (-i) */
  char **idirs;
  size_t idirs_n;
  /* input files */
  char **ifiles;
  size_t ifiles_n;
};

extern State state;

#endif /* CKC_H */
