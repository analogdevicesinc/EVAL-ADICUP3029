#if !defined(_ARROW_TIME_WATCHDOG_H_)
#define _ARROW_TIME_WATCHDOG_H_

#if defined(__cplusplus)
extern "C" {
#endif

int	wdt_start(void);
int	wdt_feed(void);
void wdt_stop(void);

#if defined(__cplusplus)
}
#endif

#endif //_ARROW_TIME_WATCHDOG_H_
