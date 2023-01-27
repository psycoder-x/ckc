#include"ckclib.h"
#include"stdlib.h"
#include"stdio.h"

void gerr(const char *msg) {
  static const char *const fmt = "%s%s\n";
  fprintf(stderr, fmt, "error: ", msg);
  exit(EXIT_FAILURE);
}

void ferr(const char *msg, const char *f) {
  static const char *const fmt = "%s%s%s\n";
  fprintf(stderr, fmt, f, ": error: ", msg);
  exit(EXIT_FAILURE);
}

void lerr(const char *msg, const char *f, int l) {
  static const char *const fmt = "%s%c%i%s%s\n";
  fprintf(stderr, fmt, f, ':', l, ": error: ", msg);
  exit(EXIT_FAILURE);
}

void cerr(const char *msg, const char *f, int l, int c) {
  static const char *const fmt = "%s%c%i%c%i%s%s\n";
  fprintf(stderr, fmt, f, ':', l, ':', c, ": error: ", msg);
  exit(EXIT_FAILURE);
}

void *smalloc(size_t bytes) {
  void *new_ptr = malloc(bytes);
  if (new_ptr == NULL) {
    gerr("fail to allocate memory");
  }
  return new_ptr;
}

void *scalloc(size_t num, size_t size) {
  void *new_ptr = calloc(num, size);
  if (new_ptr == NULL) {
    gerr("fail to allocate memory");
  }
  return new_ptr;
}

void *srealloc(void *ptr, size_t bytes) {
  void *new_ptr = realloc(ptr, bytes);
  if (new_ptr == NULL) {
    gerr("fail to reallocate memory");
  }
  return new_ptr;
}
