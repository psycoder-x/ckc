#ifndef PP_H
#define PP_H

#include "lex.h"
#include "file.h"

typedef struct FileDataA {
  bool valid;
  /* if false, an error was printed and you need to exit */  
  size_t size;
  /* number of files */
  FileData *at;
  /* array of files */
} FileDataA;
/* array of files */

typedef struct TokenA {
  bool valid;
  /* if false, an error was printed and you need to exit */  
  size_t size;
  /* number of tokens */
  Token *at;
  /* array of tokens */
} TokenA;
/* array of tokens */

typedef struct CodeFile {
  bool valid;
  /* if false, an error was printed and you need to exit */
  TokenA tokens;
  /* ready to compile */
  FileDataA sources;
  /* source files */
} CodeFile;
/* file after preprocessing */

TokenV ta_view(
  TokenA array
);

CodeFile cf_new(
  CharV file,
  /* file by this name will be preprocessed */
  CharV local,
  /* context directory */
  CharVV idirs
  /* include directories */
);

void cf_delete(
  CodeFile file
  /* CodeFile returned from a "new" function */
);
/* frees the memory used by CodeFile */

#endif /* PP_H */
