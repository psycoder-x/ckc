#include "ckc.h"

#include "stdio.h"
/* included to print errors */

#define CKC_VERSION "v0.0.0"

typedef enum ArgType {
  ARG_VERSION,
  ARG_HELP,
  ARG_PP_ONLY,
  ARG_COM_ONLY,
  ARG_OUTPUT_FILE,
  ARG_INPUT_DIRECTORY,
  ARG_INPUT_FILE,
  ARG_UNKNOWN,
} ArgType;

typedef struct Option {
  bool prm;
  /* requires a parameter */

  CharV lng;
  /* long name */

  char shrt;
  /* single character name */
} Option;

typedef bool(*ArgHandler)(void *ctx, ArgType type, CharV value);

static bool argument(
  CharVV args, /* read access to all arguments */
  size_t *i, /* index to the next argument */
  ArgHandler func, /* function with context */
  void *ctx /* context for the function */
);

static bool long_option(
  CharVV args, /* read access to all arguments */
  size_t *i, /* index to the next argument */
  ArgHandler func, /* function with context */
  void *ctx, /* context for the function */
  CharV opt
);

static CharV req_param(
  CharVV args, /* read access to all arguments */
  size_t *i, /* index to the current argument */
  CharV param_mb /* characters after option */
);

static CharV get_next(
  CharVV args, /* read access to all arguments */
  size_t *i /* index to the next argument */
);

static bool count_arg(void *ctx, ArgType type, CharV value);
static bool save_arg(void *ctx, ArgType type, CharV value);

static const char version[] =
  "CK Compiler "CKC_VERSION"\n"
  "Copyright (C) 2023 psycoder-x\n"
  "License: MIT <https://spdx.org/licenses/MIT.html>\n"
  ;

static const char help[] =
  "Usage: ckc [options...] [-o outfile] [-c|-p] file(s)...\n"
  "  -c --conly         Compile only; do not assemble or link\n"
  "  -h --help          Display this information\n"
  "  -i --include <dir> Add include path <dir>\n"
  "  -o --output <file> Place the output into <file>\n"
  "  -p --ponly         Preprocessor only\n"
  "  -v --version       Display compiler version information\n"
  ;

static const Option options[] = {
  { .prm = 0, .shrt = 'v', .lng = CV_NTS("version") },
  { .prm = 0, .shrt = 'h', .lng = CV_NTS("help") },
  { .prm = 0, .shrt = 'p', .lng = CV_NTS("ponly") },
  { .prm = 0, .shrt = 'c', .lng = CV_NTS("conly") },
  { .prm = 1, .shrt = 'o', .lng = CV_NTS("output") },
  { .prm = 1, .shrt = 'i', .lng = CV_NTS("include") },
};

static const size_t options_size = sizeof(options)/sizeof(options[0]);

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
  for (size_t i = 0; i < args.size;) {
    if (!argument(args, &i, count_arg, &ckc)) {
      ckc.valid = 0;
      return ckc;
    }
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
  for (size_t i = 0; i < args.size;) {
    if (!argument(args, &i, save_arg, &ckc)) {
      ckc.valid = 0;
      return ckc;
    }
  }
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

bool argument(CharVV args, size_t *i, ArgHandler func, void *ctx) {
  CharV base = get_next(args, i);
  if (base.size == 0) {
    // fprintf(stderr, "%s\n", "the empty argument");
    return true;
  }
  if (base.at[0] != '-') {
    func(ctx, ARG_INPUT_FILE, base);
    return true;
  }
  if (base.size == 1) {
    fprintf(stderr, "%s\n", "the empty option \'-\'");
    return false;
  }
  CharV option = cv_cut(base, 1);
  CharV end = cv_cut(option, 1);
  switch (option.at[0]) {
  case '-':
    return long_option(args, i, func, ctx, end);
  case 'c':
    return func(ctx, ARG_COM_ONLY, end);
  case 'h':
    return func(ctx, ARG_HELP, end);
  case 'i':
    return func(ctx, ARG_INPUT_DIRECTORY, req_param(args, i, end));
  case 'o':
    return func(ctx, ARG_OUTPUT_FILE, req_param(args, i, end));
  case 'p':
    return func(ctx, ARG_PP_ONLY, end);
  case 'v':
    return func(ctx, ARG_VERSION, end);
  default:
    return func(ctx, ARG_INPUT_FILE, base);
  }
  return true;
}

bool long_option(
  CharVV args, size_t *i, ArgHandler func, void *ctx, CharV opt
) {
  size_t oi = 0;
  for (; oi < options_size; oi++) {
    if (cv_eq(opt, options[oi].lng)) {
      break;
    }
  }
  if (oi >= options_size) {
    fprintf(stderr, "%s", "error: ");
    cv_write(opt, stderr);
    fprintf(stderr, "%s\n", ": unknown option");
    return false;
  }
  if (options[oi].prm) {
    return func(ctx, oi, get_next(args, i));
  }
  else {
    return func(ctx, oi, opt);
  }
}

CharV req_param(CharVV args, size_t *i, CharV param_mb) {
  if (param_mb.size == 0) {
    return get_next(args, i);
  }
  return param_mb;
}

CharV get_next(CharVV args, size_t *i) {
  ++*i;
  if (*i >= args.size) {
    return cv_mk(0, "");
  }
  return args.at[*i];
}

bool count_arg(void *ctx, ArgType type, CharV value) {
  Ckc *ckc = ctx;
  switch (type) {
  case ARG_HELP: ckc->help = 1; break;
  case ARG_VERSION: ckc->version = 1; break;
  case ARG_PP_ONLY: ckc->pp_only = 1; break;
  case ARG_COM_ONLY: ckc->com_only = 1; break;
  case ARG_OUTPUT_FILE: ckc->ofile = value; break;
  case ARG_INPUT_DIRECTORY: ckc->idirs.size++; break;
  case ARG_INPUT_FILE: ckc->ifiles.size++; break;
  default:
    fprintf(stderr, "%s%i%s\n",
      "error: ", type, ": unknown option type");
    return false;
  }
  return true;
}

bool save_arg(void *ctx, ArgType type, CharV value) {
  Ckc *ckc = ctx;
  switch (type) {
  case ARG_INPUT_DIRECTORY:
    ckc->idirs.at[ckc->idirs.size] = value;
    ckc->idirs.size++;
    break;
  case ARG_INPUT_FILE:
    ckc->ifiles.at[ckc->ifiles.size] = value;
    ckc->ifiles.size++;
    break;
  default:
    /* errors only in the count_arg */
    return true;
  }
  return true;
}
