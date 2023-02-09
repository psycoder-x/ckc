#include "file.h"

#include "stdio.h"
/* included to read files */

static CharA path_cat(
  CharV dir,
  CharV name
);

static CharV path_name(
  CharV path
);
/* returns name of the file  */

static CharV path_dir(
  CharV path
);
/* returns the directory where the file is located */

static const char *path_last_sep(
  CharV path
);

static bool find(
  CharV filename,
  CharV localdir,
  CharVV idirs,
  CharA *path_out,
  FILE **stream_out
);

static bool local_find(
  CharV dir,
  CharV name,
  CharA *path_out,
  FILE **stream_out
);

FileData fd_new(CharV filename, CharV localdir, CharVV idirs) {
  FileData fd;
  FILE *stream;
  /* find */
  if (!find(filename, localdir, idirs, &fd.path, &stream)) {
    fd.valid = 0;
    return fd;
  }
  /* read */
  fseek(stream, 0, SEEK_END);
  size_t chars = ftell(stream);
  fseek(stream, 0, SEEK_SET);
  fd.content = ca_new(chars + 1);
  if (!fd.content.valid) {
    ca_delete(fd.path);
    fclose(stream);
    fd.valid = 0;
    return fd;
  }
  size_t readn = fread(fd.content.at, sizeof(char), chars, stream);
  fclose(stream);
  /* complete */
  if (readn != chars) {
    ca_delete(fd.path);
    ca_delete(fd.content);
    fd.valid = 0;
    return fd;
  }
  fd.content.at[chars] = '\0'; /* C-compatible */
  CharV path = ca_view(fd.path);
  fd.name = path_name(path);
  fd.dir = path_dir(path);
  fd.valid = 1;
  return fd;
}

void fd_delete(FileData file) {
  if (file.valid) {
    ca_delete(file.path);
    ca_delete(file.content);
  }
}

CharA path_cat(CharV dir, CharV name) {
  return ca_new_cat(dir, name);
}

bool find(
  CharV filename, CharV localdir, CharVV idirs,
  CharA *path_out, FILE **stream_out
) {
  static const CharV empty = CV_NTS("");
  if (!local_find(empty, filename, path_out, stream_out)) {
    return false;
  }
  if (*stream_out != NULL) {
    return true;
  }
  if (localdir.size != 0) {
    ca_delete(*path_out);
    if (!local_find(localdir, filename, path_out, stream_out)) {
      return false;
    }
    if (*stream_out != NULL) {
      return true;
    }
  }
  for (size_t i = 0; i < idirs.size; i++) {
    ca_delete(*path_out);
    if (!local_find(idirs.at[i], filename, path_out, stream_out)) {
      return false;
    }
    if (*stream_out != NULL) {
      return true;
    }
  }
  ca_delete(*path_out);
  cv_write(filename, stderr);
  fprintf(stderr, "%s\n", ": error: file not found");
  return false;
}

bool local_find(
  CharV dir, CharV name,
  CharA *path_out, FILE **stream_out
) {
  *path_out = path_cat(dir, name);
  if (!path_out->valid) {
    return false;
  }
  *stream_out = fopen(path_out->at, "rb");
  return true;
}

CharV path_name(CharV path) {
  const char *sep = path_last_sep(path);
  if (sep == NULL) {
    return path;
  }
  size_t dir_size = sep - path.at + 1;
  return cv_mk(path.size - dir_size, path.at + dir_size);
}

CharV path_dir(CharV path) {
  const char *sep = path_last_sep(path);
  if (sep == NULL) {
    return (CharV) CV_NTS("");
  }
  size_t dir_size = sep - path.at + 1;
  return cv_mk(dir_size, path.at);
}

const char *path_last_sep(CharV path) {
  const char *sep = cv_rchr(path, '/');
  const char *alt_sep = cv_rchr(path, '\\');
  if (sep == NULL) {
    return alt_sep;
  }
  if (alt_sep == NULL) {
    return sep;
  }
  return (sep > alt_sep) ? sep : alt_sep;
}
