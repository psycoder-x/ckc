#ifndef LEX_H
#define LEX_H

#include "str.h"

#include "file.h"

typedef enum TokenType {
  TT_EOF,
  /* complex */
  TT_ID,
  TT_INT_LIT,
  TT_CHAR_LIT,
  TT_STR_LIT,
  /* punctuator */
  TT_BRACE_BEG,
  TT_BRACE_END,
  TT_PAREN_BEG,
  TT_PAREN_END,
  TT_BRACK_BEG,
  TT_BRACK_END,
  TT_SEMICOLON,
  TT_COMMA,
  TT_DOT,
  TT_BIT_NOT,
  TT_BIT_OR,
  TT_BIT_AND,
  TT_ADD,
  TT_SUB,
  TT_MUL,
  TT_DIV,
  TT_GT,
  TT_LT,
  TT_ASSIGN,
  TT_EQ,
  /* keywords */
  TT_AND,
  TT_DEFINE,
  TT_ELSE,
  TT_END,
  TT_IF,
  TT_IFDEF,
  TT_IFNDEF,
  TT_INCLUDE,
  TT_NOT,
  TT_OR,
  TT_PTR,
  TT_RET,
  TT_UNDEF,
  TT_WHILE,
} TokenType;

typedef struct Token {
  TokenType type;
  /* meaning of the token */

  CharV value;
  /* string representation of the token */

  const FileData *file;
  /* the token is a part of this file */
} Token;

typedef struct TokenL {
  bool valid;
  /* if false, an error was printed and you need to exit */  

  size_t size;
  /* number of tokens */

  size_t capacity;
  /* allocated memory */

  Token *at;
  /* array of tokens */
} TokenL;
/* list of tokens */

typedef struct TokenV {
  size_t size;
  const Token *at;
} TokenV;

typedef struct Context {
  size_t line;
  size_t col;
  const FileData *file;
  CharV line_view;
  CharV range;
} Context;

Token tok_mk(
  TokenType type,
  CharV value,
  const FileData *file
);

TokenL tl_new();
/* returns an empty list of tokens */

TokenL tl_new_lex(
  const FileData *file
  /* file to tokenize */
);
/* analyze <file>'s content and returns tokens */

TokenL tl_add(
  TokenL list,
  Token token
);
/* add token to <list> */

TokenL tl_add_range(
  TokenL list,
  TokenV tokens
  /* cannot be view of tokens in this <list> */
);
/* add another tokens to <list> */

TokenL tl_insert_range(
  TokenL list,
  TokenV tokens,
  /* cannot be view of tokens in this <list> */
  size_t i
  /* starting from index <i> */
);
/* insert another tokens into <list> */

TokenV tl_view(
  TokenL list
);
/* returns a view of the list */

void tl_delete(
  TokenL list
  /* list returned from a "new" function */
);
/* frees the memory used by <list> */

TokenV tv_mk(
  size_t size,
  const Token *pointer
);
/* returns a view of tokens */

Context ctx_mk_token(
  Token token
);
/* returns context of the token */

void ctx_write_position(
  Context ctx,
  FILE *stream
);

void ctx_write_line_view(
  Context ctx,
  FILE *stream
);


#endif /* !LEX_H */
