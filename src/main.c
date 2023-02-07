#include "ckc.h"
#include "str.h"
#include "stdio.h"

void print_cvv(CharVV arr, FILE *stream) {
  for (size_t i = 0, n = 1; i < arr.size; i++, n++) {
    fprintf(stream, "%i%s", n, ". ");
    cv_write(arr.at[i], stream);
    fputc('\n', stream);
  }
}

int main(int argc, char **argv) {
  CharV args_varr[argc];
  CharVV args = cvv_mk(argc, args_varr);
  for (size_t i = 0; i < args.size; i++) {
    args_varr[i] = cv_mk(nts_len(argv[i]), argv[i]);
  }
  Ckc ckc = ckc_new_args(args);
  if (!ckc.valid) {
    return 1;
  }
  /* test */
  const char *sbool[2] = { "false", "true" };
  fprintf(stdout, "%s\n%s\n", "  help:", sbool[ckc.help]);
  fprintf(stdout, "%s\n%s\n", "  version:", sbool[ckc.version]);
  fprintf(stdout, "%s\n%s\n", "  pp_only:", sbool[ckc.pp_only]);
  fprintf(stdout, "%s\n%s\n", "  com_only:", sbool[ckc.com_only]);
  fprintf(stdout, "%s\n", "  ofile:");
  cv_write(ckc.ofile, stdout);
  fputc('\n', stdout);
  fprintf(stdout, "%s\n", "  ifiles:");
  print_cvv(cvv_cva(ckc.ifiles), stdout);
  fprintf(stdout, "%s\n", "  idirs:");
  print_cvv(cvv_cva(ckc.idirs), stdout);
  /* end test */
  ckc_delete(ckc);
  return 0;
}
