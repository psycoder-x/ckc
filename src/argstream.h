/* argstream.h (C header)
 * Stream interface to read command-line arguments.
 * 
 */

#ifndef ARGSTREAM_H_INCLUDED
#define ARGSTREAM_H_INCLUDED 1

//****************************************************************
// Dependencies

#include<stdbool.h>
#include"./strview.h"

//****************************************************************
// Data types

/* Argument Stream (opaque type) */
typedef struct Args Args;

/* Option */
typedef struct Opt {
  /* A character that matches a short option, such as "-h" */
  char alpha;
  /* A string that matches a long option, such as "--help" */
  Sv name;
  /* This option requires a parameter */
  bool param;
  /* A description of what this option does */
  Sv info;
} Opt;

/* Argument */
typedef struct Arg {
  /* The alpha of the option used to load this argument. 
   * If the alpha is the null character, there is no option
   */
  char alpha;
  /* The value of the parameter, or an empty string if none exists.
   * Or the value of the argument if there is no option
   */
  Sv value;
} Arg;

//****************************************************************
// Functions and function-macros

/* Open a stream to read arguments */
Args      *aopen   (int argc, char **argv, size_t n, const Opt *opts);
/* Move the stream position indicator to the beginning */
void       arewind (Args *args);
/* Get the next argument or NULL if end of stream */
const Arg *aget    (Args *args);
/* Returns the current stream position indicator */
size_t     atell   (Args *args);
/* Moves the stream position indicator to a specific location */
void       aseek   (Args *args, size_t offset, bool from_end);
/* Close a stream */
void       aclose  (Args *args);
/* Check for the end-of-stream */
bool       aeos    (Args *args);
/* Check for a stream error */
bool       aerror  (Args *args);

#endif /* ARGSTREAM_H_INCLUDED */
