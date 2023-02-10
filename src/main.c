#include "ckc.h"
#include "str.h"
#include "file.h"
#include "lex.h"
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
  /* test */
  CharV local = (CharV) CV_NTS("./");
  CharVV idirs = cvv_mk(ckc.idirs.size, ckc.idirs.at);
  for (size_t i = 0; i < ckc.ifiles.size; i++) {
    FileData fd = fd_new(ckc.ifiles.at[i], local, idirs);
    if (!fd.valid) {
      ckc_delete(ckc);
      return 1;
    }
    fprintf(stdout, "%s", "file: ");
    cv_write(ca_view(fd.path), stdout);
    fprintf(stdout, "\n%s\n", "tokens:");
    TokenL tokens = tl_new_lex(&fd);
    if (!tokens.valid) {
      fd_delete(fd);
      ckc_delete(ckc);
      return 1;
    }
    for (size_t t = 0; t < tokens.size; t++) {
      fprintf(stdout, "%2i | ", tokens.at[t].type);
      cv_write(tokens.at[t].value, stdout);
      fputc('\n', stdout);
    }
    fd_delete(fd);
  }
  /* end test */
  ckc_delete(ckc);
  return 0;
}
