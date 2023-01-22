#include"ckc.h"
#include"stdio.h"
#include"stdlib.h"
#include"stdbool.h"
#include"string.h"

static void exit_event();

static const char version[] =
  "CK Compiler "VERSION"\n"
  "Copyright (C) 2023 psycoder-x\n"
  "License: MIT <https://spdx.org/licenses/MIT.html>\n"
  ;

static const char help[] =
  "Usage: ckc [options...] [-o outfile] [-c|-p] file(s)...\n"
  "  -c                 Compile only; do not assemble or link\n"
  "  -h --help          Display this information\n"
  "  -i --include <dir> Add include path <dir>\n"
  "  -o --output <file> Place the output into <file>\n"
  "  -p                 Preprocessor only\n"
  "  -v --version       Display compiler version information\n"
  ;

State state;

int main(int argc, char **argv) {
  if (atexit(exit_event)) {
    gerror(0, "atexit() is not supported");
  }
  state.args = argv;
  state.args_n = argc;
  /*
   * exit options
   */
  for (int i = 1; i < state.args_n; i++) {
    if (strcmp(state.args[i], "--help") == 0
      || strcmp(state.args[i], "-h") == 0) {
      fputs(help, stdout);
      exit(0);
    }
    if (strcmp(state.args[i], "--version") == 0
      || strcmp(state.args[i], "-v") == 0) {
      fputs(version, stdout);
      exit(0);
    }
  }
  /*
   * immediate options
   */
  for (int i = 1; i < state.args_n; i++) {
    /**/ if (strcmp(state.args[i], "--include") == 0
          || strcmp(state.args[i], "-i") == 0) {
      i++;
      state.idirs_n++;
    }
    else if (strcmp(state.args[i], "--output") == 0
          || strcmp(state.args[i], "-o") == 0) {
      i++;
      state.ofile = state.args[i];
    }
    else if (strcmp(state.args[i], "-c") == 0) {
      state.com_only = 1;
    }
    else if (strcmp(state.args[i], "-p") == 0) {
      state.pp_only = 1;
    }
    /* input file */
    else {
      state.ifiles_n++;
    }
  }
  if (state.ifiles_n == 0) {
    gerror(0, "no input files");
  }
  /*
   * options
   */
  state.ifiles = ecalloc(state.ifiles_n, sizeof(char *));
  state.idirs = ecalloc(state.idirs_n, sizeof(char *));
  int ifile_i = 0;
  int idir_i = 0;
  for (int i = 1; i < state.args_n; i++) {
    if (strcmp(state.args[i], "-c") == 0
      || strcmp(state.args[i], "-p") == 0) {
      continue;
    }
    if (strcmp(state.args[i], "--output") == 0
      || strcmp(state.args[i], "-o") == 0) {
      i++;
      continue;
    }
    if (strcmp(state.args[i], "--include") == 0
      || strcmp(state.args[i], "-i") == 0) {
      i++;
      state.idirs[idir_i] = state.args[i];
      idir_i++;
    }
    /* infile */
    else {
      state.ifiles[ifile_i] = state.args[i];
      ifile_i++;
    }
  }
  /*
   * file by file
   */
  for (int i = 0; i < state.ifiles_n; i++) {
    /* for example print path */
    printf("%s\n", state.ifiles[i]);
  }
  exit(0);
}

void exit_event() {
  free(state.idirs);
  free(state.ifiles);
}

void gerror(char warn, const char *msg) {
  const char *type = warn ? "warning" : "error";
  fprintf(stderr, "%s: %s\n", type, msg);
  if (!warn) {
    exit(1);
  }
}

void lerror(char warn, const char *msg, const char *file, int line) {
  const char *type = warn ? "warning" : "error";
  fprintf(stderr, "%s:%i: %s: %s\n", file, line, type, msg);
  if (!warn) {
    exit(1);
  }
}

void *emalloc(size_t bytes) {
  void *new_ptr = malloc(bytes);
  if (new_ptr == NULL) {
    gerror(0, "fail to allocate memory");
  }
  return new_ptr;
}

void *ecalloc(size_t num, size_t size) {
  void *new_ptr = calloc(num, size);
  if (new_ptr == NULL) {
    gerror(0, "fail to allocate memory");
  }
  return new_ptr;
}

void *erealloc(void *ptr, size_t bytes) {
  void *new_ptr = realloc(ptr, bytes);
  if (new_ptr == NULL) {
    gerror(0, "fail to reallocate memory");
  }
  return new_ptr;
}
