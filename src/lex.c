#include "lex.h"

#include "stdlib.h"
/* included to allocate memory */

typedef struct Lexer {
  bool valid;
  const FileData *file;
  const char *cur;
  const char *limit;
} Lexer;

typedef struct Context {
  size_t line;
  size_t col;
  const FileData *file;
  CharV line_view;
} Context;

typedef struct Keyword {
  TokenType type;
  CharV string;
} Keyword;

static const Keyword keywords[] = {
  { .string = CV_NTS("and"), .type = TT_AND },
  { .string = CV_NTS("byte"), .type = TT_BYTE },
  { .string = CV_NTS("char"), .type = TT_CHAR },
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
  { .string = CV_NTS("void"), .type = TT_VOID },
  { .string = CV_NTS("uint"), .type = TT_UINT },
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

static bool is_space(
  char ch
);

static bool is_alpha(
  char ch
);

static bool is_digit(
  char ch
);

static bool is_id(
  char ch
);

static bool skip_comment(
  Lexer *lex
);

static Context get_context(
  const FileData *file,
  const char *pointer
);

static Token get_alnum(
  Lexer *lex,
  TokenType type
);

static Token get_quoted(
  Lexer *lex,
  TokenType type
);

static void ctx_write_position(
  Context ctx,
  FILE *stream
);

static void ctx_write_line_view(
  Context ctx,
  FILE *stream
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
    .limit = file->content.at + file->content.size
  };
  TokenL list = tl_new();
  if (!list.valid) {
    return list;
  }
  for (;;) {
    Token token = get_token(&lex);
    if (!lex.valid) {
      tl_delete(list);
      list.valid = 0;
      return list;
    }
    list = tl_add(list, token);
    if (token.type == TT_EOF || !list.valid) {
      return list;
    }
  }
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
  while (lex->cur < lex->limit && is_space(*lex->cur)) {
    lex->cur++;
  }
  /* no chars left? */
  if (lex->cur >= lex->limit) {
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
    if (is_alpha(*lex->cur) || *lex->cur == '_') {
      return get_alnum(lex, TT_ID);
    }
    if (is_digit(*lex->cur)) {
      return get_alnum(lex, TT_INT_LIT);
    }
  }
  lex->valid = 0;
  Context ctx = get_context(lex->file, lex->cur);
  ctx_write_position(ctx, stderr);
  fprintf(stderr, "%s\n", "error: unknown token");
  ctx_write_line_view(ctx, stderr);
  return tok_mk(TT_EOF, cv_mk(1, lex->cur), lex->file);
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
    if (lex->cur >= lex->limit) {
      lex->valid = 0;
      Context ctx = get_context(lex->file, start);
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

Context get_context(const FileData *file, const char *pointer) {
  Context ctx;
  ctx.file = file;
  ctx.line = 0;
  ctx.col = 0;
  ctx.line_view = (CharV) CV_NTS("context was missed!");
  CharV text = ca_view(file->content);
  size_t col = 1;
  size_t line = 1;
  const char *line_start = text.at;
  for (size_t i = 0; i < text.size; i++) {
    if (&text.at[i] == pointer) {
      ctx.line = line;
      ctx.col = col;
      size_t line_size = line_start - text.at;
      CharV string = cv_mk(text.size - line_size, line_start);
      const char *line_sep = cv_chr(string, '\n');
      if (line_sep != NULL) {
        string = cv_mk(line_sep - string.at, string.at);
      }
      ctx.line_view = string;
      return ctx;
    }
    if (text.at[i] == '\n') {
      col = 1;
      line++;
      line_start = &text.at[i + 1];
    }
    else if (text.at[i] != '\r') {
      col++;
    }
  }
  return ctx;
}

void ctx_write_position(Context ctx, FILE *stream) {
  cv_write(ca_view(ctx.file->path), stream);
  fprintf(stream, ":%i:%i: ", ctx.line, ctx.col);
}

void ctx_write_line_view(Context ctx, FILE *stream) {
  cv_write(ctx.line_view, stream);
  fputc('\n', stream);
  for (size_t i = 1; i < ctx.col; i++) {
    fputc(' ', stream);
  }
  fputc('^', stream);
  fputc('\n', stream);
}

Token get_alnum(Lexer *lex, TokenType type) {
  const char *start = lex->cur;
  while (lex->cur < lex->limit && is_id(*lex->cur)) {
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
    if (lex->cur >= lex->limit) {
      Context ctx = get_context(lex->file, start);
      ctx_write_position(ctx, stderr);
      fprintf(stderr, "%s%c\n", "error: missing quote ", quote);
      ctx_write_line_view(ctx, stderr);
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

bool is_space(char ch) {
  return ch == ' ' || ch == '\t' || ch == '\0'
    || ch == '\v' || ch == '\n' || ch == '\r';
}

bool is_alpha(char ch) {
  return ('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z')
    || (unsigned char)ch > 127;
}

bool is_digit(char ch) {
  return '0' <= ch && ch <= '9';
}

bool is_id(char ch) {
  return is_digit(ch) || is_alpha(ch) || ch == '_';
}
