/* preinit.c (C source)
 * Implementation for preinit.h
 */

//****************************************************************
// Dependencies

#include"./preinit.h"
#include"./strview.h"
#include"./extraio.h"
#include<stdio.h>
#include<ctype.h>

//****************************************************************
// Static functions and function-macros

/* Print alpha then name then info for each option */
static void print_help(size_t optc, const Opt *optv);
/* Print information about the name, version, 
 * origin, and legal status of the program
 */
static void print_version();
/* Look for options such as help or version
 * and print the relevant information.
 * Return true if there is at least one such option
 */
static bool handle_info_options(Args *a, size_t optc, const Opt *optv);

//****************************************************************
// Static constant-macros

/* Spaces between any "information unit" */
#define GAP_LEN (2)
/* Length of dash "-" and option alpha together */
#define SHORT_OPT_LEN (2)
/* Length of dash "--" before option name */
#define LONG_OPT_PREFIX_LEN (2)
/* Maximum length of the option name */
#define MAX_OPT_NAME_LEN (16)

//****************************************************************
// Functions and function-macros

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

void preinit(PreinitCtx *ctx, Args *a, size_t optc, const Opt *optv) {
  ctx->info_mode = handle_info_options(a, optc, optv);
}
