#include"token.h"
#include"ckc.h"
#include"string.h"
#include"ctype.h"

#define TOK(TYPE) (ts->ttype = TYPE)

#define NEXT (ts->curchar = fgetc(ts->stream))

static token_type tok_idlike(tokens *ts);

static token_type tok_number(tokens *ts);

static token_type tok_string(tokens *ts);

static token_type tok_character(tokens *ts);

static void skip_comment(tokens *ts);

static int escape(int ch);

const char * const token_strings[] = {
#define t(tok, str) str ,
  TOKEN_LIST
#undef t
};

const char * const token_type_strings[] = {
#define t(tok, str) #tok ,
  TOKEN_LIST
#undef t
};

void tok_begin(tokens *ts, FILE *stream, const char *file) {
  ts->col = 1;
  ts->line = 1;
  ts->size = 0;
  ts->stream = stream;
  ts->file = file;
  ts->ttype = TOKEN_INVALID;
  ts->curchar = fgetc(ts->stream);
  ts->tval[0] = '\0';
}

token_type tok_get(tokens *ts) {
  ts->col += ts->size;
  /*
   * skip space
   */
  while (isspace(ts->curchar)) {
    if (ts->curchar == '\n') {
      ts->line++;
      ts->col = 1;
    }
    else if (ts->curchar != '\r') {
      ts->col++;
    }
    NEXT;
  }
  /*
   * no chars left?
   */
  if (ts->curchar == EOF) {
    return TOK(TOKEN_END_OF_STREAM);
  }
  /*
   * match
   */
  ts->size = 1;
  switch (ts->curchar) {
  case '{': NEXT; return TOK(TOKEN_BRACE_BEG);
  case '}': NEXT; return TOK(TOKEN_BRACE_END);
  case '(': NEXT; return TOK(TOKEN_PAREN_BEG);
  case ')': NEXT; return TOK(TOKEN_PAREN_END);
  case '[': NEXT; return TOK(TOKEN_BRACK_BEG);
  case ']': NEXT; return TOK(TOKEN_BRACK_END);
  case ';': NEXT; return TOK(TOKEN_SEMICOLON);
  case ',': NEXT; return TOK(TOKEN_COMMA);
  case '.': NEXT; return TOK(TOKEN_DOT);
  case '~': NEXT; return TOK(TOKEN_BIT_NOT);
  case '|': NEXT; return TOK(TOKEN_BIT_OR);
  case '&': NEXT; return TOK(TOKEN_BIT_AND);
  case '+': NEXT; return TOK(TOKEN_ADD);
  case '-': NEXT; return TOK(TOKEN_SUB);
  case '*': NEXT; return TOK(TOKEN_MUL);
  case '=':
    NEXT;
    switch (ts->curchar) {
    case '=': ts->size = 2; NEXT; return TOK(TOKEN_EQ);
    default: return TOK(TOKEN_ASSIGN);
    }
  case '/':
    NEXT;
    switch (ts->curchar) {
    case ':': skip_comment(ts); return tok_get(ts);
    default: return TOK(TOKEN_DIV);
    }
  case '\'': return tok_character(ts);
  case '\"': return tok_string(ts);
  default:
    if (isalpha(ts->curchar) || ts->curchar == '_') {
      return tok_idlike(ts);
    }
    if (isdigit(ts->curchar)) {
      return tok_number(ts);
    }
  }
  /*
   * something wrong
   */
  lerror(0, "invalid token", ts->file, ts->line, ts->col);
  return TOK(TOKEN_INVALID);
}

token_type tok_idlike(tokens *ts) {
  char *str = ts->tval;
  char *end = ts->tval + TOKEN_VALUE_BUF_SIZE;
  while ((isalnum(ts->curchar) || ts->curchar == '_') && str != end) {
    *str = ts->curchar;
    str++;
    NEXT;
  }
  if (str == end) {
    lerror(0, "identifier is too long", ts->file, ts->line, ts->col);
  }
  *str = '\0';
  ts->size = str - ts->tval;
  /*
   * keywords
   */
  for (token_type t = TOKEN_AND; t <= TOKEN_WHILE; t++) {
    if (strcmp(ts->tval, token_strings[t]) == 0) {
      return t;
    }
  }
  return TOKEN_IDENTIFIER;
}

token_type tok_number(tokens *ts) {
  char *str = ts->tval;
  char *end = ts->tval + TOKEN_VALUE_BUF_SIZE;
  while (isdigit(ts->curchar) && str != end) {
    *str = ts->curchar;
    str++;
    NEXT;
  }
  if (str == end) {
    lerror(0, "number is too long", ts->file, ts->line, ts->col);
  }
  *str = '\0';
  ts->size = str - ts->tval;
  return TOKEN_INTEGER;
}

token_type tok_string(tokens *ts) {
  char *str = ts->tval;
  char *end = ts->tval + TOKEN_VALUE_BUF_SIZE;
  ts->size = 1;
  for (char esc = 0; str != end; ) {
    NEXT;
    ts->size++;
    if (ts->curchar == EOF) {
      lerror(0, "endless string", ts->file, ts->line, ts->col);
    }
    if (!esc) {
      if (ts->curchar == '\\') {
        esc = 1;
      }
      else if (ts->curchar == '\"') {
        NEXT;
        break;
      }
      else {
        *str = ts->curchar;
        str++;
      }
    }
    else {
      esc = 0;
      *str = escape(ts->curchar);
      str++;
    }
  }
  if (str == end) {
    lerror(0, "string is too long", ts->file, ts->line, ts->col);
  }
  *str = '\0';
  return TOKEN_STRING;
}

token_type tok_character(tokens *ts) {
  NEXT;
  ts->tval[1] = '\0';
  if (ts->curchar == EOF) {
    lerror(0, "endless character", ts->file, ts->line, ts->col);
  }
  if (ts->curchar != '\\') {
    ts->tval[0] = ts->curchar;
    ts->size = 3;
  }
  else {
    NEXT;
    if (ts->curchar == EOF) {
      lerror(0, "endless character", ts->file, ts->line, ts->col);
    }
    ts->tval[0] = escape(ts->curchar);
    ts->size = 4;
  }
  NEXT;
  if (ts->curchar != '\'') {
    lerror(0, "missing \' character", ts->file, ts->line, ts->col);
  }
  NEXT;
  return TOKEN_CHARACTER;
}

void skip_comment(tokens *ts) {
  int sline = ts->line;
  int scol = ts->col;
  ts->size = 0;
  ts->col += 2;
  for (int before = ':'; ; before = ts->curchar) {
    NEXT;
    if (ts->curchar == EOF) {
      lerror(0, "endless comment", ts->file, sline, scol);
    }
    if (before == ':' && ts->curchar == '/') {
      ts->col++;
      break;
    }
    if (ts->curchar == '\n') {
      ts->col = 1;
      ts->line++;
    }
    else if (ts->curchar != '\r') {
      ts->col++;
    }
  }
  NEXT;
}

int escape(int ch) {
  switch (ch) {
  case ' ': return '\\';
  case 'n': return '\n';
  case 't': return '\t';
  case 'r': return '\r';
  case 'f': return '\f';
  case 'v': return '\v';
  case 'a': return '\a';
  case 'b': return '\b';
  case '0': return '\0';
  case '\"': return '\"';
  case '\'': return '\"';
  case '\\': return '\\';
  default: return ch;
  }
}
