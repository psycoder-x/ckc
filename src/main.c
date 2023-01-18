/* main.c (C source)
 * The entry point of the program
 */

#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"./argstream.h"
#include"./extraio.h"
#include"./preinit.h"
#include"./init.h"

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
  /* Init context */
  static InitCtx initx;
  Args *a = aopen(argc, argv, optc, optv);
  preinit(&preinitx, a, optc, optv);
  if (preinitx.info_mode) {
    aclose(a);
    return EXIT_SUCCESS;
  }
  init(&initx, a);
  aclose(a);
  /* example */
  printf("Input files:\n");
  for (size_t i = 0; i < initx.input_filenames.size; i++) {
    fputc_x(' ', stdout, 3);
    fputsv(sva_at(initx.input_filenames, i), stdout);
    putchar('\n');
  }
  printf("Output file:\n");
  fputc_x(' ', stdout, 3);
  fputsv(initx.output_filename, stdout);
  putchar('\n');
  /* example end */
  // TODO: load input files to RAM
  // TODO: preprocessor
  init_free(&initx);
  return EXIT_SUCCESS;
}
