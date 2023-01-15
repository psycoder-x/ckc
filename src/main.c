/* main.c (C source)
 * The entry point of the program
 */

#include<stdio.h>
#include"./argstream.h"

int main(int argc, char **argv) {
  /* Available options */
  static Opt optv[] = {
    {
      .alpha = 'h', .name = SV_SZ("help"), .param = false,
      .info = SV_SZ("Display this information.")
    },
    {
      .alpha = 'v', .name = SV_SZ("version"), .param = false,
      .info = SV_SZ("Display compiler version information.")
    },
    {
      .alpha = 'o', .name = SV_SZ("output"), .param = true,
      .info = SV_SZ("Place the output into <file>.")
    },
  };

  /* Temporary example */
  Args *a = aopen(argc, argv, sizeof(optv)/sizeof(Opt), optv);
  while (!aeos(a) && !aerror(a)) {
    const Arg *arg = aget(a);
    if (arg->alpha != '\0') {
      printf("-%c ", arg->alpha);
    }
    if (!sv_mty(arg->value)) {
      putchar('<');
      fwrite(arg->value.data, arg->value.size, 1, stdout);
      putchar('>');
    }
    putchar('\n');
  }
  long n = atell(a);
  printf("atell() = %d\n", n);
  aclose(a);
  /* Example end */

  // TODO: handle help option and stop the program
  // TODO: handle version option and stop the program

  return 0;
}
