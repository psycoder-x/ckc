#ifndef TOKEN_H
#define TOKEN_H

#include"stdio.h"

#define TOKEN_VALUE_BUF_SIZE (1024)

#define TOKEN_LIST \
  t(BRACE_BEG, "{") \
  t(BRACE_END, "}") \
  t(PAREN_BEG, "(") \
  t(PAREN_END, ")") \
  t(BRACK_BEG, "[") \
  t(BRACK_END, "]") \
  t(SEMICOLON, ";") \
  t(COMMA, ",") \
  t(DOT, ".") \
  t(BIT_NOT, "~") \
  t(BIT_OR, "|") \
  t(BIT_AND, "&") \
  t(ADD, "+") \
  t(SUB, "-") \
  t(MUL, "*") \
  t(DIV, "/") \
  t(GT, ">") \
  t(LT, "<") \
  t(ASSIGN, "=") \
  t(EQ, "==") \
  t(INVALID, "") \
  t(END_OF_STREAM, "") \
  t(IDENTIFIER, "") \
  t(INTEGER, "") \
  t(STRING, "") \
  t(CHARACTER, "") \
  t(AND, "and") \
  t(BYTE, "byte") \
  t(CHAR, "char") \
  t(DATA, "data") \
  t(ELSE, "else") \
  t(IF, "if") \
  t(INCLUDE, "include") \
  t(MODULE, "module") \
  t(NOT, "not") \
  t(OR, "or") \
  t(PTR, "ptr") \
  t(RETURN, "ret") \
  t(VOID, "void") \
  t(UINT, "uint") \
  t(WHILE, "while")

typedef enum token_type {
#define t(tok, str) TOKEN_##tok ,
  TOKEN_LIST
#undef t
  TOKEN_NUMBER
} token_type;

/* Token stream */
typedef struct tokens {
  const char *file;
  int curchar;
  int col;
  int line;
  FILE *stream;
  token_type ttype;
  int size;
  char tval[TOKEN_VALUE_BUF_SIZE];
} tokens;

extern const char * const token_strings[];

extern const char * const token_type_strings[];

void tok_begin(tokens *ts, FILE *stream, const char *file);

token_type tok_get(tokens *ts);

#endif /* TOKEN_H */
