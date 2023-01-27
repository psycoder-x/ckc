#ifndef TOKEN_H
#define TOKEN_H

#include"stddef.h"

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
  t(SHARP, "#") \
  t(ASSIGN, "=") \
  t(EQ, "==") \
  t(INVALID, "") \
  t(EOS, "") \
  t(ID, "") \
  t(NUM, "") \
  t(STRING, "") \
  t(CHARACTER, "") \
  t(AND, "and") \
  t(BYTE, "byte") \
  t(CHAR, "char") \
  t(DATA, "data") \
  t(DEFINE, "define") \
  t(ELSE, "else") \
  t(END, "end") \
  t(IF, "if") \
  t(IFDEF, "ifdef") \
  t(IFNDEF, "ifndef") \
  t(INCLUDE, "include") \
  t(NOT, "not") \
  t(OR, "or") \
  t(PTR, "ptr") \
  t(RETURN, "ret") \
  t(VOID, "void") \
  t(UINT, "uint") \
  t(UNDEF, "undef") \
  t(WHILE, "while")

typedef enum token_type {
#define t(tok, str) TOK_##tok ,
  TOKEN_LIST
#undef t
  TOK_COUNT
} token_type;

/* token location */
typedef struct token_pos {
  const char *file;
  size_t line;
  size_t column;
} token_pos;

/* Token */
typedef struct token {
  token_type type;
  size_t length;
  char *string;
  token_pos loc;
} token;

extern const char * const token_strings[];

extern const char * const token_type_strings[];

token_pos tok_pos_new(const char *file, size_t line, size_t column);

token tok_new(token_type t, token_pos l);

token tok_new_str(token_type t, size_t n, const char *s, token_pos l);

void tok_del(token t);

#endif /* TOKEN_H */
