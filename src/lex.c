#include "lex.h"

#include "stdlib.h"
/* included to allocate memory */

typedef struct Lexer {
  bool valid;
  const FileData *file;
  const char *cur;
  const char *end;
} Lexer;

typedef struct Keyword {
  TokenType type;
  CharV string;
} Keyword;

static const Keyword keywords[] = {
  { .string = CV_NTS("and"), .type = TT_AND },
  { .string = CV_NTS("define"), .type = TT_DEFINE },
  { .string = CV_NTS("else"), .type = TT_ELSE },
  { .string = CV_NTS("end"), .type = TT_END },
  { .string = CV_NTS("if"), .type = TT_IF },
  { .string = CV_NTS("ifdef"), .type = TT_IFDEF },
  { .string = CV_NTS("ifndef"), .type = TT_IFNDEF },
  { .string = CV_NTS("include"), .type = TT_INCLUDE },
  { .string = CV_NTS("not"), .type = TT_NOT },
  { .string = CV_NTS("or"), .type = TT_OR },
  { .string = CV_NTS("ptr"), .type = TT_PTR },
  { .string = CV_NTS("ret"), .type = TT_RET },
  { .string = CV_NTS("undef"), .type = TT_UNDEF },
  { .string = CV_NTS("while"), .type = TT_WHILE },
};

static Token get_token(
  Lexer *lex
);

static TokenL tl_realloc(
  TokenL list,
  size_t len_request
);

static void tok_rcopy(
  Token *dst,
  const Token *src,
  size_t count
);

static Token get_tok_range(
  Lexer *lex,
  TokenType type,
  size_t size
);

static bool is_id(
  char ch
);

static bool skip_comment(
  Lexer *lex
);

static Token get_alnum(
  Lexer *lex,
  TokenType type
);

static Token get_quoted(
  Lexer *lex,
  TokenType type
);

Token tok_mk(TokenType type, CharV value, const FileData *file) {
  return (Token) {
    .type = type,
    .value = value,
    .file = file
  };
}

TokenL tl_new() {
#define TL_DEF_CAP (2)
  TokenL list;
  list.size = 0;
  list.capacity = TL_DEF_CAP;
  list.at = calloc(TL_DEF_CAP, sizeof(Token));
  list.valid = (list.at != NULL);
  if (!list.valid) {
    fprintf(stderr, "%s%s\n",
      __func__, ": error: fail to allocate memory");
  }
  return list;
}

TokenL tl_new_lex(const FileData *file) {
  Lexer lex = (Lexer) {
    .valid = 1,
    .file = file,
    .cur = file->content.at,
    .end = file->content.at + file->content.size
  };
  TokenL list = tl_new();
  while (list.valid) {
    Token token = get_token(&lex);
    if (!lex.valid) {
      tl_delete(list);
      list.valid = 0;
      return list;
    }
    if (token.type == TT_EOF) {
      break;
    }
    list = tl_add(list, token);
  }
  return list;
}

TokenL tl_add(TokenL list, Token token) {
  return tl_add_range(list, tv_mk(1, &token));
}

TokenL tl_add_range(TokenL list, TokenV tokens) {
  return tl_insert_range(list, tokens, list.size);
}

TokenL tl_insert_range(TokenL list, TokenV tokens, size_t i) {
  if (i > list.size) {
    free(list.at);
    fprintf(stderr, "%s%s\n",
      __func__, ": error: the index is out of range");
    list.valid = 0;
    return list;
  }
  size_t j = i + tokens.size;
  size_t k = list.size - i;
  list = tl_realloc(list, list.size + tokens.size);
  if (!list.valid) {
    return list;
  }
  list.size += tokens.size;
  tok_rcopy(list.at + j, list.at + i, k);
  tok_rcopy(list.at + i, tokens.at, tokens.size);
  return list;
}

TokenV tl_view(TokenL list) {
  if (!list.valid) {
    return tv_mk(0, NULL);
  }
  return tv_mk(list.size, list.at);
}

TokenL tl_shrink_to_fit(TokenL list) {
  Token *new_ptr = realloc(list.at, list.size * sizeof(Token));
  if (new_ptr == NULL) {
    return list;
  }
  list.at = new_ptr;
  list.capacity = list.size;
  return list;
}

void tl_delete(TokenL list) {
  if (list.valid) {
    free(list.at);
  }
}

TokenL tl_realloc(TokenL list, size_t len_request) {
  if (list.capacity >= len_request) {
    return list;
  }
  list.capacity = len_request + len_request / 2; /* x1.5 */
  Token *new_ptr = realloc(list.at, list.capacity * sizeof(Token));
  if (new_ptr == NULL) {
    list.valid = 0;
    free(list.at);
    fprintf(stderr, "%s%s\n",
      __func__, ": error: fail to reallocate memory");
    return list;
  }
  list.at = new_ptr;
  return list;
}

TokenV tv_mk(size_t size, const Token *pointer) {
  return (TokenV) {
    .size = size,
    .at = pointer
  };
}

void tok_rcopy(Token *dst, const Token *src, size_t count) {
  dst += count;
  src += count;
  while (count != 0) {
    dst--;
    src--;
    count--;
    *dst = *src;
  }
}

Token get_token(Lexer *lex) {
  /* skip space */
  while (lex->cur < lex->end && c_isspace(*lex->cur)) {
    lex->cur++;
  }
  /* no chars left? */
  if (lex->cur >= lex->end) {
    return tok_mk(TT_EOF, cv_mk(0, lex->cur), lex->file);
  }
  /* match */
  switch (*lex->cur) {
  case '{': return get_tok_range(lex, TT_BRACE_BEG, 1);
  case '}': return get_tok_range(lex, TT_BRACE_END, 1);
  case '(': return get_tok_range(lex, TT_PAREN_BEG, 1);
  case ')': return get_tok_range(lex, TT_PAREN_END, 1);
  case '[': return get_tok_range(lex, TT_BRACK_BEG, 1);
  case ']': return get_tok_range(lex, TT_BRACK_END, 1);
  case ';': return get_tok_range(lex, TT_SEMICOLON, 1);
  case ',': return get_tok_range(lex, TT_COMMA, 1);
  case '.': return get_tok_range(lex, TT_DOT, 1);
  case '~': return get_tok_range(lex, TT_BIT_NOT, 1);
  case '|': return get_tok_range(lex, TT_BIT_OR, 1);
  case '&': return get_tok_range(lex, TT_BIT_AND, 1);
  case '+': return get_tok_range(lex, TT_ADD, 1);
  case '-': return get_tok_range(lex, TT_SUB, 1);
  case '*': return get_tok_range(lex, TT_MUL, 1);
  case '>': return get_tok_range(lex, TT_GT, 1);
  case '<': return get_tok_range(lex, TT_LT, 1);
  case '=':
    if (*(lex->cur + 1) == '=') {
      return get_tok_range(lex, TT_EQ, 2);
    }
    else {
      return get_tok_range(lex, TT_ASSIGN, 1);
    }
  case '/':
    if (*(lex->cur + 1) == ':') {
      if (!skip_comment(lex)) {
        return tok_mk(TT_EOF, cv_mk(1, lex->cur), lex->file);
      }
      return get_token(lex);
    }
    else {
      return get_tok_range(lex, TT_DIV, 1);
    }
  case '\'': return get_quoted(lex, TT_CHAR_LIT);
  case '\"': return get_quoted(lex, TT_STR_LIT);
  default:
    if (c_isalpha(*lex->cur) || *lex->cur == '_') {
      return get_alnum(lex, TT_ID);
    }
    if (c_isdigit(*lex->cur)) {
      return get_alnum(lex, TT_INT_LIT);
    }
  }
  lex->valid = 0;
  Token utoken = tok_mk(0, cv_mk(1, lex->cur), lex->file);
  Context ctx = ctx_mk_token(utoken);
  ctx_write_position(ctx, stderr);
  fprintf(stderr, "%s\n", "error: unknown token");
  ctx_write_line_view(ctx, stderr);
  return utoken;
}

Token get_tok_range(Lexer *lex, TokenType type, size_t size) {
  Token t = tok_mk(type, cv_mk(size, lex->cur), lex->file);
  lex->cur += size;
  return t;
}

bool skip_comment(Lexer *lex) {
  const char *start = lex->cur;
  for (char before = '/'; ; before = *lex->cur) {
    lex->cur++;
    if (lex->cur >= lex->end) {
      lex->valid = 0;
      CharV string = cv_mk(lex->end - start, start);
      Context ctx = ctx_mk_token(tok_mk(0, string, lex->file));
      ctx_write_position(ctx, stderr);
      fprintf(stderr, "%s\n", "error: endless comment");
      ctx_write_line_view(ctx, stderr);
      return false;
    }
    if (before == ':' && *lex->cur == '/') {
      break;
    }
  }
  lex->cur++;
  return true;
}

Context ctx_mk_token(Token token) {
  Context ctx = (Context) {
    .file = token.file,
    .line = 1,
    .col = 1,
    .range = token.value,
    .line_view = (CharV) CV_NTS("context not found!")
  };
  CharV text = ca_view(token.file->content);
  const char *line_ptr = text.at;
  for (size_t i = 0; i < text.size; i++) {
    if (text.at + i == token.value.at) {
      size_t end_size = line_ptr - text.at;
      CharV string = cv_mk(text.size - end_size, line_ptr);
      const char *line_sep = cv_chr(string, '\n');
      if (line_sep != NULL) {
        string = cv_mk(line_sep - string.at, string.at);
      }
      ctx.line_view = string;
      return ctx;
    }
    if (text.at[i] == '\n') {
      ctx.col = 1;
      ctx.line++;
      line_ptr = text.at + i + 1;
    }
    else if (text.at[i] != '\r') {
      ctx.col++;
    }
  }
  return ctx;
}

void ctx_write_position(Context ctx, FILE *stream) {
  cv_write(ca_view(ctx.file->path), stream);
  fprintf(stream, ":%i:%i: ", ctx.line, ctx.col);
}

void ctx_write_line_view(Context ctx, FILE *stream) {
  fprintf(stream, "%4i | ", ctx.line);
  cv_write(ctx.line_view, stream);
  fprintf(stream, "\n     | ");
  for (size_t i = 1; i < ctx.col; i++) {
    fputc(' ', stream);
  }
  fputc('^', stream);
  size_t lim = ctx.line_view.size + ctx.line_view.at - ctx.range.at;
  lim--; /* '\n' not counts */
  lim = (lim < ctx.range.size) ? lim : ctx.range.size;
  for (size_t i = 1; i < lim; i++) {
    fputc('~', stream);
  }
  fputc('\n', stream);
}

Token get_alnum(Lexer *lex, TokenType type) {
  const char *start = lex->cur;
  while (lex->cur < lex->end && is_id(*lex->cur)) {
    lex->cur++;
  }
  CharV value = cv_mk(lex->cur - start, start);
  if (type == TT_ID) {
    size_t size = sizeof(keywords)/sizeof(keywords[0]);
    for (size_t i = 0; i < size; i++) {
      if (cv_eq(value, keywords[i].string)) {
        type = keywords[i].type;
        break;
      }
    }
  }
  return tok_mk(type, value, lex->file);
}

Token get_quoted(Lexer *lex, TokenType type) {
  const char *start = lex->cur;
  char quote = *lex->cur;
  for (bool esc = 0; ; ) {
    lex->cur++;
    if (lex->cur >= lex->end) {
      CharV string = cv_mk(lex->end - start, start);
      Context ctx = ctx_mk_token(tok_mk(0, string, lex->file));
      ctx_write_position(ctx, stderr);
      fprintf(stderr, "%s%c\n", "error: missing quote ", quote);
      ctx_write_line_view(ctx, stderr);
      lex->valid = 0;
      return tok_mk(0, (CharV) CV_NTS(""), lex->file);
    }
    if (!esc) {
      if (*lex->cur == quote) {
        break;
      }
      if (*lex->cur == '\\') {
        esc = 1;
      }
    }
    else {
      esc = 0;
    }
  }
  lex->cur++;
  CharV value = cv_mk(lex->cur - start, start);
  return tok_mk(type, value, lex->file);
}

bool is_id(char ch) {
  return c_isdigit(ch) || c_isalpha(ch) || ch == '_';
}
