#ifndef FILE_H
#define FILE_H

#include "str.h"

#include "stdbool.h"
/* included for bool */

typedef struct FileData {
  bool valid;
  /* if false, an error was printed and you need to exit */

  CharA path;
  /* path to the file */

  CharV dir;
  /* path to the directory where the file is located */

  CharV name;
  /* name of the file (aka <path> without <dir>) */

  CharA content;
  /* characters from the file */
} FileData;
/* file stored in RAM */

FileData fd_new(
  CharV filename,
  CharV localdir,
  CharVV idirs
);
/* 
searches for a file by...
1. <filename>
2. <localdir> + <filename>
3. for each <dir> in <idirs>: <dir> + <filename>
returns valid file data if the file was found and loaded successfully */

void fd_delete(
  FileData file /* a file returned from a "new" function */
);
/* frees the memory used by <file> */

#endif /* !FILE_H */
