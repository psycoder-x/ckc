#include "pp.h"

#include "stdlib.h"

#define FDL_CAP 3

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
static bool pp_global_next(Prexer *pre);
static bool pp_include(Prexer *pre);
static bool pp_ifdef(Prexer *pre, bool sample);
static bool pp_ignore(Prexer *pre, Token root);
static bool pp_define(Prexer *pre);
static bool pp_undef(Prexer *pre);
static bool pp_try_macro(Prexer *pre);

CodeFile cf_new(CharV file, CharV local, CharVV idirs) {
  /* prepare */
  TokenL tokens = tl_new();
  FileDataL sources = fdl_new();
  Prexer prexer = (Prexer) {
    .valid = 1,
    .toks = &tokens,
    .srcs = &sources,
    .idirs = &idirs
  };
  if (!tokens.valid || !sources.valid) {
    tl_delete(tokens);
    fdl_delete(sources);
    return (CodeFile) { .valid = 0 };
  }
  /* preprocess */
  if (!cf_read(&prexer, file, local)) {
    tl_delete(tokens);
    fdl_delete(sources);
    return (CodeFile) { .valid = 0 };
  }
  /* construct the result */
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
  while (pre->cur < pre->end) {
    if (!pp_global_next(pre)) {
      return false;
    }
  }
  return true;
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

bool pp_global_next(Prexer *pre) {
  switch (pre->cur->type) {
  case TT_INCLUDE: return pp_include(pre);
  case TT_IFDEF: return pp_ifdef(pre, true);
  case TT_IFNDEF: return pp_ifdef(pre, false);
  case TT_DEFINE: return pp_define(pre);
  case TT_UNDEF: return pp_undef(pre);
  case TT_END: {
    Context ctx = ctx_mk_token(*pre->cur);
    ctx_write_position(ctx, stderr);
    fprintf(stderr, "error: unexpected 'end'\n");
    ctx_write_line_view(ctx, stderr);
    return false;
  };
  case TT_ID: return pp_try_macro(pre);
  default: break;
  }
  *pre->toks = tl_add(*pre->toks, *pre->cur);
  pre->cur++;
  return pre->toks->valid;
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
  (void)macro_name;
  bool defined = !sample; /* TODO: pp_find_def() != -1 */
  if (defined != sample) {
    return pp_ignore(pre, *(pre->cur - 1));
  }
  fprintf(stderr, "error: true-branch is not implemented yet\n");
  return false; /* TODO: implement true-branch */
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
  Context ctx = ctx_mk_token(*pre->cur);
  ctx_write_position(ctx, stderr);
  fprintf(stderr, "error: 'define' is not implemented yet\n");
  ctx_write_line_view(ctx, stderr);
  return false; /* TODO: implement define */
}

bool pp_undef(Prexer *pre) {
  Context ctx = ctx_mk_token(*pre->cur);
  ctx_write_position(ctx, stderr);
  fprintf(stderr, "error: 'undef' is not implemented yet\n");
  ctx_write_line_view(ctx, stderr);
  return false; /* TODO: implement undef */
}

bool pp_try_macro(Prexer *pre) {
  /* if fail: */
  *pre->toks = tl_add(*pre->toks, *pre->cur);
  pre->cur++;
  return pre->toks->valid;
}
