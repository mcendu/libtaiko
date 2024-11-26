// SPDX-License-Identifier: BSD-2-Clause
#include "io.h"

#include "alloc.h"
#include "taiko.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

struct taiko_file_ {
  taiko_allocator *alloc;
  void *stream;
  taiko_read_fn *read;
  taiko_write_fn *write;
  taiko_close_fn *close;
  taiko_printf_fn *printf;
};

taiko_file *taiko_file_open_(taiko_allocator *alloc, void *stream,
                             taiko_read_fn *read, taiko_write_fn *write,
                             taiko_close_fn *close, taiko_printf_fn *printf) {
  taiko_file *f = taiko_malloc_(alloc, sizeof(taiko_file));
  if (!f)
    return NULL;

  f->alloc = alloc;
  f->stream = stream;
  f->read = read;
  f->write = write;
  f->close = close;
  f->printf = printf;
  return f;
}

taiko_file *taiko_file_open_path_(const char *path, const char *mode) {
  FILE *f = fopen(path, mode);
  if (!f)
    return NULL;

  taiko_file *result =
      taiko_file_open_(&taiko_default_allocator_, f, (taiko_read_fn *)fread,
                       (taiko_write_fn *)fwrite, (taiko_close_fn *)fclose,
                       (taiko_printf_fn *)vfprintf);
  if (!result) {
    fclose(f);
    return NULL;
  }

  return result;
}

taiko_file *taiko_file_open_stdio_(FILE *file) {
  return taiko_file_open_(&taiko_default_allocator_, file,
                          (taiko_read_fn *)fread, (taiko_write_fn *)fwrite,
                          NULL, (taiko_printf_fn *)vfprintf);
}

taiko_file *taiko_file_open_null_(taiko_allocator *alloc) {
  if (!alloc)
    alloc = &taiko_default_allocator_;

  return taiko_file_open_(alloc, NULL, NULL, NULL, NULL, NULL);
}

void taiko_file_close_(taiko_file *file) {
  if (file->close)
    file->close(file->stream);
  taiko_free_(file->alloc, file);
}

size_t taiko_file_read_(taiko_file *file, void *dst, size_t size) {
  if (file->read)
    return file->read(dst, 1, size, file->stream);
  return 0;
}

size_t taiko_file_write_(taiko_file *file, const void *src, size_t size) {
  if (file->write)
    return file->write(src, 1, size, file->stream);
  return 0;
}

int taiko_file_printf_(taiko_file *file, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int result = taiko_file_vprintf_(file, format, ap);
  va_end(ap);

  return result;
}

int taiko_file_vprintf_(taiko_file *file, const char *format, va_list arg) {
  return file->printf(file->stream, format, arg);
}
