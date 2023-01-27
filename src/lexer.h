#ifndef LEXER_H
#define LEXER_H

#include"file.h"
#include"token.h"

typedef struct token_list {
  size_t len;
  size_t cap;
  token *arr;
} token_list;

typedef struct token_view {
  size_t len;
  const token *arr;
} token_view;

token_list lex_file(file_data file);

token_list tl_new();
void tl_del(token_list tl);
void tl_add(token_list *tl, token_view tv);
void tl_insert(token_list *tl, token_view tv, size_t i);
void tl_remove(token_list *tl, size_t i, size_t n);
token_view tl_view(token_list tl);
size_t tl_len(token_list tl);

token_view tv_new(size_t len, const token *arr);
size_t tv_len(token_view tv);
token tv_get(token_view tv, size_t i);

#endif /* LEXER_H */
