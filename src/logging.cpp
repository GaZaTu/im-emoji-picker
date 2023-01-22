#include "logging.hpp"
#include <cstdio>
#include <time.h>
#include <stdarg.h>

extern "C" {
void log_printf(const char* format, ...) {
#ifndef NDEBUG
  static FILE* log_file = nullptr;
  if (!log_file) {
    log_file = fopen("/tmp/dank-emoji-picker.log", "a");
  }

  fprintf(log_file, "(%ld) ", time(nullptr));

  va_list args;
  va_start(args, format);
  vfprintf(log_file, format, args);
  va_end(args);

  fflush(log_file);
#endif
}
}
