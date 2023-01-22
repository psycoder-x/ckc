#include"token.h"

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
