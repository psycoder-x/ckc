#ifndef FILE_H
#define FILE_H

#include"stdio.h"
#include"stddef.h"

typedef struct file_data {
  char *name;
  char *content;
  size_t length;
} file_data;

file_data file_read_by_name(const char *filename);

file_data file_read(FILE *stream, const char *filename);

void file_del(file_data file);

#endif /* FILE_H */
