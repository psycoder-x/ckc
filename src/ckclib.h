#ifndef CKCLIB_H
#define CKCLIB_H

#include"stddef.h"
#include"stdlib.h"

/* global error */
void gerr(const char *msg);
/* local error in file (f - file) */
void ferr(const char *msg, const char *f);
/* local error in line (f - file, l - line) */
void lerr(const char *msg, const char *f, int l);
/* local error in column (f - file, l - line, c - column) */
void cerr(const char *msg, const char *f, int l, int c);
/* malloc that checks for errors */
void *smalloc(size_t bytes);
/* calloc that checks for errors */
void *scalloc(size_t num, size_t size);
/* realloc that checks for errors */
void *srealloc(void *ptr, size_t bytes);

#endif /* CKCLIB_H */
