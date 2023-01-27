#include"lexer.h"
#include"ckclib.h"
#include"string.h"
#include"ctype.h"
#include"stdbool.h"

#include"stdio.h"

typedef struct token_stream {
  const file_data *file;
  size_t index;
  size_t line;
  size_t col;
} token_stream;

static void tl_realloc(token_list *tl, size_t len_request);

static bool ts_end(token_stream *ts);

static char ts_cur(token_stream *ts);

static token ts_get(token_stream *ts);

static char ts_next(token_stream *ts);

static void ts_skip_comment(token_stream *ts, token_pos loc);

static token ts_alnum(token_stream *ts, token_pos loc, token_type t);

static token ts_q(token_stream *ts, token_pos l, token_type t, char q);

/********************************************************************/
/* lex                                                              */
/********************************************************************/

token_list lex_file(file_data file) {
  token_stream ts = (token_stream) {
    .file = &file,
    .index = 0,
    .line = 1,
    .col = 1,
  };
  token_list tl = tl_new();
  for (;;) {
    token tok = ts_get(&ts);
    if (tok.type == TOK_EOS) {
      break;
    }
    tl_add(&tl, tv_new(1, &tok));
  }
  return tl;
}

bool ts_end(token_stream *ts) {
  return ts->index >= ts->file->length;
}

char ts_cur(token_stream *ts) {
  if (ts_end(ts)) {
    return '\0';
  }
  return ts->file->content[ts->index];
}

char ts_next(token_stream *ts) {
  ts->index++;
  char ch = ts_cur(ts);
  if (ch == '\n') {
    ts->col = 0;
    ts->line++;
  }
  else if (ch != '\r') {
    ts->col++;
  }
  return ch;
}

token ts_get(token_stream *ts) {
  char ch = ts_cur(ts);
  /*
   * skip space
   */
  while (!ts_end(ts) && isspace(ch)) {
    ch = ts_next(ts);
  }
  token_pos loc = tok_pos_new(ts->file->name, ts->line, ts->col);
  /*
   * no chars left?
   */
  if (ts_end(ts)) {
    return tok_new(TOK_EOS, loc);
  }
  /*
   * match
   */
  switch (ch) {
  case '{': ts_next(ts); return tok_new(TOK_BRACE_BEG, loc);
  case '}': ts_next(ts); return tok_new(TOK_BRACE_END, loc);
  case '(': ts_next(ts); return tok_new(TOK_PAREN_BEG, loc);
  case ')': ts_next(ts); return tok_new(TOK_PAREN_END, loc);
  case '[': ts_next(ts); return tok_new(TOK_BRACK_BEG, loc);
  case ']': ts_next(ts); return tok_new(TOK_BRACK_END, loc);
  case ';': ts_next(ts); return tok_new(TOK_SEMICOLON, loc);
  case ',': ts_next(ts); return tok_new(TOK_COMMA, loc);
  case '.': ts_next(ts); return tok_new(TOK_DOT, loc);
  case '~': ts_next(ts); return tok_new(TOK_BIT_NOT, loc);
  case '|': ts_next(ts); return tok_new(TOK_BIT_OR, loc);
  case '&': ts_next(ts); return tok_new(TOK_BIT_AND, loc);
  case '+': ts_next(ts); return tok_new(TOK_ADD, loc);
  case '-': ts_next(ts); return tok_new(TOK_SUB, loc);
  case '*': ts_next(ts); return tok_new(TOK_MUL, loc);
  case '>': ts_next(ts); return tok_new(TOK_GT, loc);
  case '<': ts_next(ts); return tok_new(TOK_LT, loc);
  //case '#': ts_next(ts); return tok_new(TOK_SHARP, loc);
  case '=':
    switch (ts_next(ts)) {
    case '=': ts_next(ts); return tok_new(TOK_EQ, loc);
    default: return tok_new(TOK_ASSIGN, loc);
    }
  case '/':
    switch (ts_next(ts)) {
    case ':': ts_skip_comment(ts, loc); return ts_get(ts);
    default: return tok_new(TOK_DIV, loc);
    }
  case '\'': return ts_q(ts, loc, TOK_CHAR, '\'');
  case '\"': return ts_q(ts, loc, TOK_STRING, '\"');
  default:
    if (isalpha(ch) || ch == '_') {
      return ts_alnum(ts, loc, TOK_ID);
    }
    if (isdigit(ch)) {
      return ts_alnum(ts, loc, TOK_NUM);
    }
  }
  /*
   * something wrong
   */
  cerr("invalid token", ts->file->name, ts->line, ts->col);
  return tok_new(TOK_INVALID, loc);
}

void ts_skip_comment(token_stream *ts, token_pos loc) {
  char ch = ts_cur(ts);
  for (char before = '/'; ; before = ch) {
    ch = ts_next(ts);
    if (ts_end(ts)) {
      cerr("endless comment", loc.file, loc.line, loc.column);
    }
    if (before == ':' && ch == '/') {
      break;
    }
  }
  ts_next(ts);
}

token ts_alnum(token_stream *ts, token_pos loc, token_type t) {
  const char *str = &ts->file->content[ts->index];
  size_t len = 0;
  char ch = ts_cur(ts);
  while (!ts_end(ts) && (isalnum(ch) || ch == '_')) {
    len++;
    ch = ts_next(ts);
  }
  /*
   * keywords
   */
  if (t == TOK_ID) {
    for (token_type t = TOK_AND; t <= TOK_WHILE; t++) {
      if (strncmp(str, token_strings[t], len) == 0) {
        return tok_new(t, loc);
      }
    }
  }
  return tok_new_str(t, len, str, loc);
}

token ts_q(token_stream *ts, token_pos l, token_type t, char q) {
  const char *str = &ts->file->content[ts->index];
  size_t len = 1;
  for (bool esc = 0; ; ) {
    char ch = ts_next(ts);
    len++;
    if (ts_end(ts)) {
      cerr("missing quote", ts->file->name, ts->line, ts->col);
    }
    if (!esc) {
      if (ch == q) {
        break;
      }
      if (ch == '\\') {
        esc = 1;
      }
    }
    else {
      esc = 0;
    }
  }
  ts_next(ts);
  return tok_new_str(t, len, str, l);
}

/********************************************************************/
/* token list                                                       */
/********************************************************************/

token_list tl_new() {
  return (token_list) {
    .len = 0,
    .cap = 2,
    .arr = smalloc(2 * sizeof(token))
  };
}

void tl_del(token_list tl) {
  for (size_t i = 0; i < tl.len; i++) {
    tok_del(tl.arr[i]);
  }
  free(tl.arr);
}

void tl_add(token_list *tl, token_view tv) {
  tl_insert(tl, tv, tl->len);
}

void tl_insert(token_list *tl, token_view tv, size_t i) {
  if (i > tl->len) {
    ferr("the index is out of range", "lexer");
  }
  size_t j = i + tv.len;
  size_t k = tl->len - i;
  tl_realloc(tl, tl->len + tv.len);
  tl->len += tv.len;
  memmove(&tl->arr[j], &tl->arr[i], k * sizeof(token));
  memmove(&tl->arr[i], &tv.arr[0], tv.len * sizeof(token));
}

void tl_remove(token_list *tl, size_t i, size_t n) {
  if (i >= tl->len) {
    ferr("the index is out of range", "lexer");
  }
  if (i + n > tl->len) {
    ferr("the size is out of range", "lexer");
  }
  /*
   * free removed tokens
   */
  for (size_t k = i; k < i + n; k++) {
    tok_del(tl->arr[k]);
  }
  /*
   * shift tokens
   */
  size_t j = i + n; /* the index of the end */
  size_t k = tl->len - j; /* the size of the end */
  memmove(&tl->arr[i], &tl->arr[j], k * sizeof(token));
  tl->len -= n;
}

token_view tl_view(token_list tl) {
  return tv_new(tl.len, tl.arr);
}

size_t tl_len(token_list tl) {
  return tl.len;
}

void tl_realloc(token_list *tl, size_t len_request) {
  if (tl->cap >= len_request) {
    return;
  }
  tl->cap = len_request + len_request / 2; /* x1.5 */
  tl->arr = srealloc(tl->arr, tl->cap * sizeof(token));
}

/********************************************************************/
/* token view                                                       */
/********************************************************************/

token_view tv_new(size_t len, const token *arr) {
  return (token_view) {
    .len = len,
    .arr = arr
  };
}

size_t tv_len(token_view tv) {
  return tv.len;
}

token tv_get(token_view tv, size_t i) {
  if (i >= tv.len) {
    ferr("the index is out of range", "lexer");
  }
  return tv.arr[i];
}
