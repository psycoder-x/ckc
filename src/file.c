#include"file.h"
#include"ckclib.h"
#include"stdbool.h"
#include"string.h"

static file_data readc(FILE *stream, const char *fname, bool close);

file_data file_read_by_name(const char *filename) {
  FILE *stream = fopen(filename, "rb");
  if (stream == NULL) {
    ferr("cannot open the file", filename);
  }
  return readc(stream, filename, true);
}

file_data file_read(FILE *stream, const char *filename) {
  return readc(stream, filename, false);
}

void file_del(file_data file) {
  free(file.name);
  free(file.content);
}

file_data readc(FILE *stream, const char *fname, bool close) {
  size_t len = strlen(fname) + 1;
  file_data file;
  file.name = smalloc(len);
  memcpy(file.name, fname, len);
  fseek(stream, 0, SEEK_END);
  file.length = ftell(stream);
  fseek(stream, 0, SEEK_SET);
  file.content = smalloc(file.length);
  size_t readn = fread(file.content, 1, file.length, stream);
  if (close) {
    fclose(stream);
  }
  if (readn != file.length) {
    if (!close) {
      fclose(stream);
    }
    ferr("cannot read the file", fname);
  }
  return file;
}
