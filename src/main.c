#include "ckc.h"
#include "str.h"
#include "file.h"
#include "lex.h"
#include "pp.h"
#include "stdio.h"

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
  if (ckc.help) {
    cv_write(ckc_help(), stdout);
    ckc_delete(ckc);
    return 0;
  }
  if (ckc.version) {
    cv_write(ckc_version(), stdout);
    ckc_delete(ckc);
    return 0;
  }
  CharV compiler_dir = path_dir(args.at[0]);
  CharVV idirs = cvv_mk(ckc.idirs.size, ckc.idirs.at);
  for (size_t i = 0; i < ckc.ifiles.size; i++) {
    CodeFile file = cf_new(ckc.ifiles.at[i], compiler_dir, idirs);
    if (!file.valid) {
      ckc_delete(ckc);
      return 1;
    }
    /* test */
    for (size_t t = 0; t < file.tokens.size; t++) {
      fprintf(stdout, "%2i | ", file.tokens.at[t].type);
      cv_write(file.tokens.at[t].value, stdout);
      fputc('\n', stdout);
    }
    /* end test */
    cf_delete(file);
  }
  ckc_delete(ckc);
  return 0;
}
