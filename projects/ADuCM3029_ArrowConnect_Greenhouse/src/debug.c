#include "debug.h"

#include <arrow/mem.h>

#define DBG_LINE_SIZE 160

char dbg_buffer[DBG_LINE_SIZE] __attribute__((weak));

#if defined(__USE_STD__)
#include <stdarg.h>
__attribute__((weak)) void dbg_line(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  *dbg_buffer = 0x0;
  vsnprintf(dbg_buffer, DBG_LINE_SIZE-2, fmt, args);
  strcat(dbg_buffer, "\r\n");
  printf(dbg_buffer);
  va_end(args);
}
#else
__attribute__((weak)) void dbg_line(const char *fmt, ...) {
  (void)(fmt);
  // dammy printf; do nothing
}
#endif

__attribute__((weak)) void hex_dump(const char *data, int size) {
  size -= size%8;
  int i=0;
  for (i=0; i<size/8; i++) {
    DBG("%02x %02x %02x %02x %02x %02x %02x %02x",
        data[0+i*8], data[1+i*8],
        data[2+i*8], data[3+i*8],
        data[4+i*8], data[5+i*8],
        data[6+i*8], data[7+i*8]);
  }
}
