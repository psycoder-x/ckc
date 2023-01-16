/* argstream.c (C source)
 * Implementation for argstream.h
 */

//****************************************************************
// Dependencies

#include"./argstream.h"
#include<stdlib.h>

//****************************************************************
// Data types

/* Argument Stream (complete type) */
struct Args {
  size_t  cap;
  size_t  size;
  size_t  pos;
  Arg    *data;
};

//****************************************************************
// Static functions and private function-macros

/* Get the next string from argv (increment index first).
 * If there are no strings left then an empty string is returned
 */
static Sv anext(int argc, char **argv, int *i);
/* Find an option by an alpha or a name.
 * Return the option pointer or NULL if not found 
 */
static const Opt *afind(size_t optc, const Opt *optv, char ch, Sv str);
/* Read one argument from argv (starting at *index + 1).
 * Save the option alpha if a long or short option appears.
 * And add a parameter to an option if required.
 * If the argument is not an option, just store its value.
 * Returns true if a subsequent call to this function is possible
 */
static bool aread(int argc, char **argv, size_t optc, const Opt *optv,
                  int *index, Arg *arg);

//****************************************************************
// Functions and function-macros

Sv anext(int argc, char **argv, int *i) {
  *i += 1;
  if (*i < argc) {
    return sv_sz(argv[*i]);
  } else {
    return SV_NULL;
  }
}

const Opt *afind(size_t optc, const Opt *optv, char ch, Sv str) {
  if (sv_len(str) == 0) {
    for (size_t i = 0; i < optc; i++) {
      if (optv[i].alpha == ch) {
        return &optv[i];
      }
    }
  } else {
    for (size_t i = 0; i < optc; i++) {
      if (sv_equ(optv[i].name, str)) {
        return &optv[i];
      }
    }
  }
  return NULL;
}

bool aread(int argc, char **argv, size_t optc, const Opt *optv,
           int *index, Arg *arg) {
  Sv tok = anext(argc, argv, index);
  arg->alpha = '\0';
  arg->value = tok;
  size_t tlen = sv_len(tok);
  if (tlen == 0) {
    return false;
  }
  if (tlen == 1) {
    return true;
  }
  char first = sv_at(tok, 0);
  if (first != '-') {
    return true;
  }
  char alpha = sv_at(tok, 1);
  size_t olen = (alpha == '-') ? tlen : 2;
  const Opt *o = afind(optc, optv, alpha, sv_cut(sv_get(tok, olen), 2));
  if (o == NULL) {
    return true;
  }
  arg->alpha = o->alpha;
  if (!o->param) {
    arg->value = SV_NULL;
    return true;
  }
  if (olen == tlen) {
    arg->value = anext(argc, argv, index);
  } else {
    arg->value = sv_cut(tok, olen);
  }
  return true;
}

Args *aopen(int argc, char **argv, size_t optc, const Opt *optv) {
  if (argc <= 0) {
    return NULL;
  }
  Args *args = malloc(sizeof(Args));
  if (args == NULL) {
    return NULL;
  }
  args->cap = (size_t)argc;
  args->size = 0;
  args->pos = 0;
  args->data = calloc(args->cap, sizeof(Arg));
  if (args->data == NULL) {
    return args;
  }
  int i = 0;
  while (aread(argc, argv, optc, optv, &i, &args->data[args->size])) {
    args->size++;
  }
  return args;
}

void arewind(Args *args) {
  if (args == NULL) {
    return;
  }
  args->pos = 0;
}

const Arg *aget(Args *args) {
  if (aeos(args)) {
    return NULL;
  }
  if (args->data == NULL) {
    return NULL;
  }
  const Arg *a = &args->data[args->pos];
  args->pos++;
  return a;
}

size_t atell(Args *args) {
  if (args == NULL) {
    return 0;
  }
  return args->pos;
}

void aseek(Args *args, size_t offset, bool from_end) {
  if (args == NULL) {
    return;
  }
  offset = (offset > args->size) ? args->size : offset;
  args->pos = (!from_end) ? offset : args->size - offset;
}

void aclose(Args *args) {
  if (args != NULL) {
    free(args->data);
  }
  free(args);
}

bool aeos(Args *args) {
  if (args == NULL) {
    return true;
  }
  return args->pos >= args->size;
}

bool aerror(Args *args) {
  if (args == NULL) {
    return true;
  }
  return args->data == NULL;
}
