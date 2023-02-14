#include "pp.h"

#include "stdlib.h"

#define FDL_CAP 3
#define ML_CAP 2

typedef struct Macro {
  Token name;
  /* identifier */
  TokenA value;
  /* tokens defined by name */
} Macro;
/* defined macro */

typedef struct MacroL {
  bool valid;
  /* if false, an error was printed and you need to exit */  
  size_t size;
  /* number of defines */
  size_t capacity;
  /* allocated memory */
  Macro *at;
  /* array of macros */
} MacroL;
/* list of macros */

typedef struct FileDataL {
  bool valid;
  /* if false, an error was printed and you need to exit */  
  size_t size;
  /* number of files */
  size_t capacity;
  /* allocated memory */
  FileData *at;
  /* array of files */
} FileDataL;
/* list of files */

typedef struct Prexer {
  bool valid;
  TokenL *toks;
  FileDataL *srcs;
  MacroL *mcrs;
  CharVV *idirs;
  const FileData *parent;
  const Token *cur;
  const Token *end;
} Prexer;
/* preprocessing context */

static void ta_delete(TokenA array);
static void fda_delete(FileDataA array);
static FileDataL fdl_new();
static void fdl_delete(FileDataL list);
static FileDataL fdl_shrink_to_fit(FileDataL list);
static bool cf_read(Prexer *pre, CharV file, CharV local);
static void fdl_add(FileDataL *list, FileData file);
static bool pp_include(Prexer *pre);
static bool pp_ifdef(Prexer *pre, bool sample);
static bool pp_ignore(Prexer *pre, Token root);
static bool pp_define(Prexer *pre);
static bool pp_undef(Prexer *pre);
static bool pp_try_macro(Prexer *pre);
static bool pp_end(Prexer *pre, bool expected);
static bool pp_block(Prexer *pre, bool toend);
static bool pp_error(Prexer *pre);

static Macro m_new(Token name, TokenL list);
static void m_delete(Macro macro);
static MacroL ml_new();
static void ml_delete(MacroL list);
static void ml_add(MacroL *list, Macro macro);
static size_t ml_find(MacroL *list, CharV name);
static void ml_remove(MacroL *list, size_t i);

CodeFile cf_new(CharV file, CharV local, CharVV idirs) {
  /* prepare */
  TokenL tokens = tl_new();
  FileDataL sources = fdl_new();
  MacroL macros = ml_new();
  Prexer prexer = (Prexer) {
    .valid = 1,
    .toks = &tokens,
    .srcs = &sources,
    .mcrs = &macros,
    .idirs = &idirs
  };
  if (!tokens.valid || !sources.valid || !macros.valid) {
    tl_delete(tokens);
    fdl_delete(sources);
    ml_delete(macros);
    return (CodeFile) { .valid = 0 };
  }
  /* preprocess */
  if (!cf_read(&prexer, file, local)) {
    tl_delete(tokens);
    fdl_delete(sources);
    ml_delete(macros);
    return (CodeFile) { .valid = 0 };
  }
  /* construct the result */
  ml_delete(macros);
  tokens = tl_shrink_to_fit(tokens);
  sources = fdl_shrink_to_fit(sources);
  CodeFile code_file = (CodeFile) {
    .valid = 1,
    .tokens = (TokenA) {
      .at = tokens.at,
      .size = tokens.size,
      .valid = 1
    },
    .sources = (FileDataA) {
      .at = sources.at,
      .size = sources.size,
      .valid = 1
    }
  };
  return code_file;
}

void cf_delete(CodeFile file) {
  if (file.valid) {
    ta_delete(file.tokens);
    fda_delete(file.sources);
  }
}

void ta_delete(TokenA array) {
  if (array.valid) {
    free(array.at);
  }
}

void fda_delete(FileDataA array) {
  if (array.valid) {
    for (size_t i = 0; i < array.size; i++) {
      fd_delete(array.at[i]);
    }
    free(array.at);
  }
}

FileDataL fdl_new() {
  FileDataL list = (FileDataL) {
    .size = 0,
    .capacity = FDL_CAP,
    .at = calloc(FDL_CAP, sizeof(FileData))
  };
  list.valid = (list.at != NULL);
  return list;
}

void fdl_delete(FileDataL list) {
  if (list.valid) {
    for (size_t i = 0; i < list.size; i++) {
      fd_delete(list.at[i]);
    }
    free(list.at);
  }
}

FileDataL fdl_shrink_to_fit(FileDataL list) {
  FileData *new_ptr = realloc(list.at, list.size * sizeof(FileData));
  if (new_ptr == NULL) {
    return list;
  }
  list.at = new_ptr;
  list.capacity = list.size;
  return list;
}

bool cf_read(Prexer *pre, CharV file, CharV local) {
  FileData source = fd_new(file, local, *pre->idirs);
  fdl_add(pre->srcs, source);
  if (!source.valid || !pre->srcs->valid) {
    return false;
  }
  TokenL tokens = tl_new_lex(&source);
  if (!tokens.valid) {
    return false;
  }
  pre->parent = &source;
  pre->cur = tokens.at;
  pre->end = tokens.at + tokens.size;
  bool ok = pp_block(pre, false);
  tl_delete(tokens);
  return ok;
}

void fdl_add(FileDataL *list, FileData file) {
  if (list->size == list->capacity) {
    size_t cap = list->capacity = list->size + 3;
    FileData *new_ptr = realloc(list->at, cap * sizeof(FileData));
    if (new_ptr == NULL) {
      fprintf(stderr, "%s%s\n", 
        __func__, ": fail to reallocate memory");
      fdl_delete(*list);
      fd_delete(file);
      list->valid = 0;
      return;
    }
  }
  list->at[list->size] = file;
  list->size++;
}

bool pp_block(Prexer *pre, bool toend) {
  bool ok = true;
  while (ok && pre->cur < pre->end) {
    switch (pre->cur->type) {
    case TT_ID:       ok = pp_try_macro(pre); break;
    case TT_INCLUDE:  ok = pp_include(pre); break;
    case TT_IFDEF:    ok = pp_ifdef(pre, true); break;
    case TT_IFNDEF:   ok = pp_ifdef(pre, false); break;
    case TT_DEFINE:   ok = pp_define(pre); break;
    case TT_UNDEF:    ok = pp_undef(pre); break;
    case TT_ERROR:    ok = pp_error(pre); break;
    case TT_END:      return pp_end(pre, toend);
    default:
      *pre->toks = tl_add(*pre->toks, *pre->cur);
      pre->cur++;
      ok = pre->toks->valid;
      break;
    }
  }
  return ok;
}

bool pp_error(Prexer *pre) {
  /* always invalid */
  Context ctx = ctx_mk_token(*pre->cur);
  ctx_write_position(ctx, stderr);
  fprintf(stderr, "error: ");
  cv_write(ctx.line_view, stderr);
  fputc('\n', stderr);
  ctx_write_line_view(ctx, stderr);
  return false;
}

bool pp_end(Prexer *pre, bool expected) {
  if (!expected) {
    Context ctx = ctx_mk_token(*pre->cur);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: unexpected 'end'\n");
    ctx_write_line_view(ctx, stderr);
  }
  pre->cur++;
  return expected;
}

bool pp_include(Prexer *pre) {
  pre->cur++;
  if (pre->cur >= pre->end) {
    Context ctx = ctx_mk_token(*(pre->cur - 1));
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: string expected, got nothing\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  if (pre->cur->type != TT_STR_LIT) {
    Context ctx = ctx_mk_token(*pre->cur);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: string expected, got not a string\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  CharV filename = cv_cut(cv_rcut(pre->cur->value, 1), 1);
  Prexer sub_pre = *pre;
  if (!cf_read(&sub_pre, filename, pre->parent->dir)) {
    Context ctx = ctx_mk_token(*pre->cur);
    fprintf(stderr, "in ");
    ctx_write_position(ctx, stderr);
    fputc('\n', stderr);
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  pre->cur++;
  return true;
}

bool pp_ifdef(Prexer *pre, bool sample) {
  pre->cur++;
  if (pre->cur >= pre->end) {
    Context ctx = ctx_mk_token(*(pre->cur - 1));
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: identifier expected, got nothing\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  if (pre->cur->type != TT_ID) {
    Context ctx = ctx_mk_token(*pre->cur);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, 
      "error: identifier expected, got not an identifier\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  CharV macro_name = pre->cur->value;
  pre->cur++;
  bool defined = (ml_find(pre->mcrs, macro_name) < pre->mcrs->size);
  if (defined != sample) {
    return pp_ignore(pre, *(pre->cur - 1));
  }
  else {
    return pp_block(pre, true);
  }
}

bool pp_ignore(Prexer *pre, Token root) {
  size_t end_need = 1;
  while (pre->cur < pre->end && end_need != 0) {
    if (pre->cur->type == TT_END) {
      end_need--;
    }
    else if (pre->cur->type == TT_IFDEF
      || pre->cur->type == TT_IFNDEF
      || pre->cur->type == TT_DEFINE) {
      end_need++;
    }
    pre->cur++;
  }
  if (end_need != 0) {
    Context ctx = ctx_mk_token(root);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: unterminated block\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  return true;
}

bool pp_define(Prexer *pre) {
  pre->cur++;
  if (pre->cur >= pre->end) {
    Context ctx = ctx_mk_token(*(pre->cur - 1));
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: identifier expected, got nothing\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  if (pre->cur->type != TT_ID) {
    Context ctx = ctx_mk_token(*pre->cur);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, 
      "error: identifier expected, got not an identifier\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  Token name = *pre->cur;
  size_t i = ml_find(pre->mcrs, name.value);
  /* redefinition */
  if (i < pre->mcrs->size) {
    Context ctx = ctx_mk_token(*pre->cur);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: \"");
    cv_write(name.value, stderr);
    fprintf(stderr, "\" redefined\n");
    ctx_write_line_view(ctx, stderr);
    Context ctx_note = ctx_mk_token(pre->mcrs->at[i].name);
    ctx_write_position(ctx_note, stderr);
    fprintf(stderr,
      "note: this is the location of the previous definition\n");
    ctx_write_line_view(ctx_note, stderr);
    return false;
  }
  /* definition */
  cv_write(name.value, stdout);
  fputc('\n', stdout);
  TokenL value = tl_new();
  if (!value.valid) {
    return false;
  }
  pre->cur++;
  Prexer sub_pre = *pre;
  sub_pre.toks = &value;
  if (!pp_block(&sub_pre, true)) {
    tl_delete(value);
    return false;
  }
  Macro macro = m_new(name, value);
  ml_add(pre->mcrs, macro);
  pre->cur = sub_pre.cur;
  return pre->mcrs->valid;
}

bool pp_undef(Prexer *pre) {
  pre->cur++;
  if (pre->cur >= pre->end) {
    Context ctx = ctx_mk_token(*(pre->cur - 1));
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: identifier expected, got nothing\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  if (pre->cur->type != TT_ID) {
    Context ctx = ctx_mk_token(*pre->cur);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, 
      "error: identifier expected, got not an identifier\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  }
  size_t i = ml_find(pre->mcrs, pre->cur->value);
  pre->cur++;
  if (i < pre->mcrs->size) {
    ml_remove(pre->mcrs, i);
  }
  return pre->mcrs->valid;
}

bool pp_try_macro(Prexer *pre) {
  size_t i = ml_find(pre->mcrs, pre->cur->value);
  /* if fail: */
  if (i >= pre->mcrs->size) {
    *pre->toks = tl_add(*pre->toks, *pre->cur);
    pre->cur++;
    return pre->toks->valid;
  }
  Macro macro = pre->mcrs->at[i];
  *pre->toks = tl_add_range(*pre->toks, ta_view(macro.value));
  pre->cur++;
  return pre->toks->valid;
}

Macro m_new(Token name, TokenL list) {
  tl_shrink_to_fit(list);
  Macro macro = (Macro) {
    .value = (TokenA) {
      .at = list.at,
      .size = list.size,
      .valid = 1
    },
    .name = name
  };
  return macro;
}

void m_delete(Macro macro) {
  ta_delete(macro.value);
}

MacroL ml_new() {
  MacroL list = (MacroL) {
    .size = 0,
    .capacity = ML_CAP,
    .at = calloc(ML_CAP, sizeof(Macro))
  };
  list.valid = (list.at != NULL);
  return list;
}

void ml_delete(MacroL list) {
  if (list.valid) {
    for (size_t i = 0; i < list.size; i++) {
      m_delete(list.at[i]);
    }
    free(list.at);
  }
}

void ml_add(MacroL *list, Macro macro) {
  if (list->size == list->capacity) {
    size_t cap = list->capacity = list->size + 10;
    Macro *new_ptr = realloc(list->at, cap * sizeof(Macro));
    if (new_ptr == NULL) {
      fprintf(stderr, "%s%s\n", 
        __func__, ": fail to reallocate memory");
      ml_delete(*list);
      m_delete(macro);
      list->valid = 0;
      return;
    }
  }
  list->at[list->size] = macro;
  list->size++;
}

size_t ml_find(MacroL *list, CharV name) {
  for (size_t i = 0; i < list->size; i++) {
    if (cv_eq(list->at[i].name.value, name)) {
      return i;
    }
  }
  return list->size;
}

void ml_remove(MacroL *list, size_t i) {
  if (i >= list->size) {
    ml_delete(*list);
    fprintf(stderr, "%s%s%i%s\n", 
        __func__, ": index ", i, " is out of range");
    list->valid = 0;
    return;
  }
  m_delete(list->at[i]);
  for (size_t j = i + 1; j < list->size; j++, i++) {
    list->at[i] = list->at[j];
  }
  list->size--;
}

TokenV ta_view(TokenA array) {
  return (TokenV) {
    .at = array.at,
    .size = (array.valid) ? array.size : 0
  };
}

