#include "time/watchdog.h"

int __attribute__((weak)) wdt_start(void)  {
  return 0; // nothing to do
}

int __attribute__((weak)) wdt_feed(void) {
  return 0; // nothing to do
}

void __attribute__((weak)) wdt_stop(void) {
  return; // nothing to do
}
