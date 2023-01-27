#include"ckc.h"
#include"ckclib.h"
#include"lexer.h"
#include"stdio.h"
#include"stdlib.h"
#include"stdbool.h"
#include"string.h"

State state;

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

int main(int argc, char **argv) {
  if (atexit(exit_event)) {
    gerr("atexit() is not supported");
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
    gerr("no input files");
  }
  /*
   * options
   */
  state.ifiles = scalloc(state.ifiles_n, sizeof(char *));
  state.idirs = scalloc(state.idirs_n, sizeof(char *));
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
  for (size_t i = 0; i < state.ifiles_n; i++) {
    /* for example print path and tokens */
    const char *filename = state.ifiles[i];
    printf("FILE: %s\n", filename);
    file_data file = file_read_by_name(filename);
    token_list tl = lex_file(file);
    token_view tv = tl_view(tl);
    size_t len = tv_len(tv);
    /* token by token */
    for (size_t i = 0; i < len; i++) {
      token tok = tv_get(tv, i);
      const char *fmt = "%i:%i\t%s\t%s\n";
      const char *ttag = token_type_strings[tok.type];
      printf(fmt, tok.loc.line, tok.loc.column, ttag, tok.string);
    }
    tl_del(tl);
    file_del(file);
  }
  exit(0);
}

void exit_event() {
  free(state.idirs);
  free(state.ifiles);
}
