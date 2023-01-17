/* main.c (C source)
 * The entry point of the program
 */

#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"./argstream.h"
#include"./extraio.h"
#include"./preinit.h"

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
  /* Number of options */
  size_t optc = sizeof(optv) / sizeof(Opt);
  /* Preinit context */
  static PreinitCtx preinitx;

  Args *a = aopen(argc, argv, optc, optv);

  preinit(&preinitx, a, optc, optv);

  if (preinitx.info_mode) {
    aclose(a);
    return EXIT_SUCCESS;
  }

  // TODO: program initialization

  aclose(a);

  return EXIT_SUCCESS;
}
