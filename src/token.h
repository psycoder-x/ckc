#ifndef TOKEN_H
#define TOKEN_H

#define TOKEN_LIST \
  t(INVALID, "") \
  t(IDENTIFIER, "") \
  t(INTEGER, "") \
  t(STRING, "") \
  t(CHARACTER, "") \
  t(WHILE, "while") \
  t(IF, "if") \
  t(ELSE, "else") \
  t(RETURN, "ret") \
  t(VOID, "void") \
  t(PTR, "ptr") \
  t(BYTE, "byte") \
  t(CHAR, "char") \
  t(UINT, "uint") \
  t(DATA, "data") \
  t(NOT, "not") \
  t(AND, "and") \
  t(OR, "or") \
  t(MODULE, "module") \
  t(INCLUDE, "include") \
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
  t(ADD, "+") \
  t(SUB, "-") \
  t(MUL, "*") \
  t(DIV, "/") \
  t(GT, ">") \
  t(LT, "<") \
  t(EQ, "==") \
  t(ASSIGN, "=")

typedef enum token_type {
#define t(tok, str) TOKEN_##tok ,
  TOKEN_LIST
#undef t
  TOKEN_NUMBER
} token_type;

extern const char * const token_strings[];

extern const char * const token_type_strings[];

#endif /* TOKEN_H */
