#include "ckc.h"
#include "str.h"
#include "file.h"
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
    fprintf(stdout, "%s", "path: ");
    cv_write(ca_view(fd.path), stdout);
    fprintf(stdout, "\n%s", "path.name: ");
    cv_write(fd.name, stdout);
    fprintf(stdout, "\n%s", "path.dir: ");
    cv_write(fd.dir, stdout);
    fprintf(stdout, "\n%s\n", "content:");
    cv_write(ca_view(fd.content), stdout);
    fputc('\n', stdout);
    fd_delete(fd);
  }
  /* end test */
  ckc_delete(ckc);
  return 0;
}
