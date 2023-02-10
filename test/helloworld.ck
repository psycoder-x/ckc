include "io.h"
/: Hello, World! program :/
uint(uint argc, ptr Str argv) main {
  cv_write("Hello, World!\n", stdout);
  ret 0;
}
