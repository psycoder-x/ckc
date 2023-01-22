#ifndef CKC_H
#define CKC_H

#include"stdio.h"

#define VERSION "v0.0.0"

typedef struct State State;
struct State {
  /* arguments */
  char **args;
  int args_n;
  /* preprocessor only (-p) */
  char pp_only;
  /* compile only; do not assemble or link (-c) */
  char com_only;
  /* output file (-o) */
  char *ofile;
  /* include dirs (-i) */
  char **idirs;
  int idirs_n;
  /* input files */
  char **ifiles;
  int ifiles_n;
};

/* global error */
void gerror(char warn, const char *msg);
/* local error */
void lerror(char warn, const char *msg, const char *file, int line);
/* malloc that checks for errors */
void *emalloc(size_t bytes);
/* calloc that checks for errors */
void *ecalloc(size_t num, size_t size);
/* realloc that checks for errors */
void *erealloc(void *ptr, size_t bytes);

#endif /* CKC_H */
