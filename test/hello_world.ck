/: hello-world program :/
include"io.hk"
(uint argc, ptr str argv)uint main {
  prints(cout, "Hello, World!");
  putlf(cout); /: line feed improvisation :/
  ret 0;
}
