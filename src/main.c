#include "str.h"
#include "stdio.h"

int main(int argc, char **argv) {
  size_t args_size = argc;
  Str args[argc];
  for (size_t i = 0; i < args_size; i++) {
    args[i] = str_mk(nts_len(argv[i]), argv[i]);
  }
  /* test */
  for (size_t i = 0; i < args_size; i++) {
    fwrite(args[i].at, 1, args[i].size, stdout);
    fputc('\n', stdout);
  }
  /* end test */
  return 0;
}
