include "std/io.hk"
/: Hello, World! program :/
(Uint argc, ptr Str argv) Uint
main {
  cv_write("Hello, World!\n", OUT);
  ret 0;
}
