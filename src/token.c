#include"token.h"
#include"ckclib.h"
#include"string.h"

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

token_pos tok_pos_new(const char *file, size_t line, size_t column) {
  return (token_pos) {
    .file = file,
    .line = line,
    .column = column
  };
}

token tok_new(token_type t, token_pos l) {
  return (token) {
    .type = t,
    .loc = l,
    .string = NULL,
    .length = 0
  };
}

token tok_new_str(token_type t, size_t n, const char *s, token_pos l) {
  token tok = (token) {
    .type = t,
    .loc = l,
    .string = smalloc(n + 1),
    .length = n + 1
  };
  memcpy(tok.string, s, n);
  tok.string[n] = '\0';
  return tok;
}

void tok_del(token t) {
  free(t.string);
}
