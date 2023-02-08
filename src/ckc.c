#include "ckc.h"

#include "stdio.h"
/* included to print errors */

#define CKC_VERSION "v0.0.0"

typedef enum ArgumentType {
  A_VERSION,
  A_HELP,
  A_PP_ONLY,
  A_COM_ONLY,
  A_OUT_FILE,
  A_INPUT_DIR,
  A_INPUT_FILE,
  A_COUNT
} ArgumentType;

typedef struct Argument {
  ArgumentType type;
  /* meaning of the argument */

  CharV patern;
  /* possible sequence of characters */

  bool param;
  /* the argument does require a parameter */
} Argument;

typedef void(*ArgumentHandler)(
  Ckc *ckc,
  ArgumentType type,
  CharV param
);

static void foreach_argument(
  Ckc *ckc,
  CharVV args,
  ArgumentHandler func
);

static void count_arg(
  Ckc *ckc,
  ArgumentType type,
  CharV param
);

static void save_arg(
  Ckc *ckc,
  ArgumentType type,
  CharV param
);

static const Argument *match_argument(
  CharV string,
  CharV *endout
);

static bool match_patern(
  CharV string,
  CharV patern,
  CharV *endout
);

static const char version[] =
  "CK Compiler "CKC_VERSION"\n"
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

static const Argument arguments[] = {
  { .type = A_INPUT_DIR,  .patern = CV_NTS("--include"), .param = 1 },
  { .type = A_VERSION,    .patern = CV_NTS("--version"), .param = 0 },
  { .type = A_OUT_FILE,   .patern = CV_NTS("--output"),  .param = 1 },
  { .type = A_HELP,       .patern = CV_NTS("--help"),    .param = 0 },
  { .type = A_INPUT_DIR,  .patern = CV_NTS("-i*"),       .param = 1 },
  { .type = A_OUT_FILE,   .patern = CV_NTS("-o*"),       .param = 1 },
  { .type = A_COM_ONLY,   .patern = CV_NTS("-c"),        .param = 0 },
  { .type = A_HELP,       .patern = CV_NTS("-h"),        .param = 0 },
  { .type = A_PP_ONLY,    .patern = CV_NTS("-p"),        .param = 0 },
  { .type = A_VERSION,    .patern = CV_NTS("-v"),        .param = 0 },
  { .type = A_INPUT_FILE, .patern = CV_NTS("*"),         .param = 1 },
};

#define ARGUMENTS_SIZE (sizeof(arguments)/sizeof(arguments[0]))

Ckc ckc_new_args(CharVV args) {
  Ckc ckc = {
    .valid = 1,
    .version = 0,
    .help = 0,
    .pp_only = 0,
    .com_only = 0,
    .ofile = cv_mk(5, "a.out"),
    .ifiles = cva_new(0),
    .idirs = cva_new(0)
  };
  foreach_argument(&ckc, args, count_arg);
  if (ckc.ifiles.size == 0) {
    fprintf(stderr, "%s\n", "error: no input files");
    ckc.valid = 0;
    return ckc;
  }
  ckc.ifiles = cva_new(ckc.ifiles.size);
  ckc.idirs = cva_new(ckc.idirs.size);
  if (!ckc.idirs.valid || !ckc.ifiles.valid) {
    cva_delete(ckc.ifiles);
    cva_delete(ckc.idirs);
    ckc.valid = 0;
    return ckc;
  }
  ckc.ifiles.size = 0;
  ckc.idirs.size = 0;
  foreach_argument(&ckc, args, save_arg);
  return ckc;
}

void ckc_delete(Ckc ckc) {
  if (!ckc.valid) {
    return;
  }
  cva_delete(ckc.ifiles);
  cva_delete(ckc.idirs);
}

CharV ckc_help() {
  return cv_mk(sizeof(help), help);
}

CharV ckc_version() {
  return cv_mk(sizeof(version), version);
}

void foreach_argument(Ckc *ckc, CharVV args, ArgumentHandler func) {
  CharV param;
  for (size_t i = 1; i < args.size; i++) {
    const Argument *arg = match_argument(args.at[i], &param);
    if (arg->param && param.size == 0) {
      if (++i < args.size) {
        param = args.at[i];
      }
    }
    func(ckc, arg->type, param);
  }
}

void count_arg(Ckc *ckc, ArgumentType type, CharV param) {
  switch (type) {
  case A_HELP:       ckc->help = 1;      break;
  case A_VERSION:    ckc->version = 1;   break;
  case A_PP_ONLY:    ckc->pp_only = 1;   break;
  case A_COM_ONLY:   ckc->com_only = 1;  break;
  case A_OUT_FILE:   ckc->ofile = param; break;
  case A_INPUT_DIR:  ckc->idirs.size++;  break;
  case A_INPUT_FILE: ckc->ifiles.size++; break;
  default: break;
  }
}

void save_arg(Ckc *ckc, ArgumentType type, CharV param) {
  switch (type) {
  case A_INPUT_DIR:
    ckc->idirs.at[ckc->idirs.size] = param;
    ckc->idirs.size++;
    break;
  case A_INPUT_FILE:
    ckc->ifiles.at[ckc->ifiles.size] = param;
    ckc->ifiles.size++;
    break;
  default: break;
  }
}

const Argument *match_argument(CharV string, CharV *endout) {
  for (size_t i = 0; i < ARGUMENTS_SIZE; i++) {
    if (match_patern(string, arguments[i].patern, endout)) {
      return &arguments[i];
    }
  }
  return &arguments[ARGUMENTS_SIZE - 1]; /* input file? */
}

bool match_patern(CharV string, CharV patern, CharV *endout) {
  if (patern.at[patern.size - 1] != '*') {
    *endout = (CharV) CV_NTS("");
    return cv_eq(string, patern);
  }
  CharV solid_patern = cv_rcut(patern, 1);
  *endout = cv_cut(string, solid_patern.size);
  return cv_eq(cv_get(string, solid_patern.size), solid_patern);
}
