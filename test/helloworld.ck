include "io.h"
/: Hello, World! program :/
Uint(Uint argc, ptr Str argv) main {
  cv_write("Hello, World!\n", stdout);
  ret 0;
}
