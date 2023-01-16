/* main.c (C source)
 * The entry point of the program
 */

#include<stdio.h>
#include<ctype.h>
#include"./argstream.h"
#include"./extraio.h"

/* Spaces between any "information unit" */
#define GAP_LEN (2)
/* Length of dash "-" and option alpha together */
#define SHORT_OPT_LEN (2)
/* Length of dash "--" before option name */
#define LONG_OPT_PREFIX_LEN (2)
/* Maximum length of the option name */
#define MAX_OPT_NAME_LEN (16)

/* Print alpha then name then info for each option */
static void print_help(size_t optc, const Opt *optv);
/* Print information about the name, version, origin, and legal status
 * of the program
 */
static void print_version();
/* Look for options such as help or version
 * and print the relevant information
 */
static bool handle_info_options(Args *a, size_t optc, const Opt *optv);

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
  size_t optc = sizeof(optv)/sizeof(Opt);

  Args *a = aopen(argc, argv, optc, optv);

  bool handled = handle_info_options(a, optc, optv);

  // TODO: exit the program if any info option has been handled
  (void)handled;

  aclose(a);

  return 0;
}

void print_help(size_t optc, const Opt *optv) {
  static const Sv message = SV_SZ(
    "Usage: ckc [options] file...\n"
    "Options:\n"
  );
  fputsv(message, stdout);
  for (size_t i = 0; i < optc; i++) {
    fputc_x(' ', stdout, GAP_LEN);
    if (isgraph(optv[i].alpha)) {
      putchar('-');
      putchar(optv[i].alpha);
    } else {
      fputc_x(' ', stdout, SHORT_OPT_LEN);
    }
    fputc_x(' ', stdout, GAP_LEN);
    if (!sv_mty(optv[i].name)) {
      fputc_x('-', stdout, LONG_OPT_PREFIX_LEN);
      fputsv_la(optv[i].name, stdout, MAX_OPT_NAME_LEN);
    } else {
      fputc_x(' ', stdout, LONG_OPT_PREFIX_LEN + MAX_OPT_NAME_LEN);
    }
    fputc_x(' ', stdout, GAP_LEN);
    fputsv(optv[i].info, stdout);
    putchar('\n');
  }
}

void print_version() {
  // TODO: correct the message
  static const Sv message = SV_SZ(
    "ckc 0.0.0\n"
    "Copyright (C) 2023 psycoder-x\n"
    "License: MIT <https://spdx.org/licenses/MIT.html>\n"
  );
  fputsv(message, stdout);
}

bool handle_info_options(Args *a, size_t optc, const Opt *optv) {
  arewind(a);
  while (!aeos(a) && !aerror(a)) {
    const Arg *arg = aget(a);
    if (arg->alpha == 'h') {
      print_help(optc, optv);
      return true;
    } else if (arg->alpha == 'v') {
      print_version();
      return true;
    }
  }
  return false;
}
