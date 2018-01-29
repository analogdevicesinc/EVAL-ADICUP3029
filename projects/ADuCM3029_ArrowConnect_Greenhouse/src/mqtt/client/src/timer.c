#include "mqtt/client/network.h"
#include <time/time.h>

void TimerInit(TimerInterval* timer) {
    timer->end_time = (struct timeval){0, 0};
}

char TimerIsExpired(TimerInterval* timer) {
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    return (int)res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(TimerInterval* timer, unsigned int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {(time_t)(timeout / 1000), (suseconds_t)(timeout % 1000) * 1000};
    timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(TimerInterval* timer, unsigned int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {(time_t)timeout, 0};
    timeradd(&now, &interval, &timer->end_time);
}


int TimerLeftMS(TimerInterval* timer) {
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    return ((int)res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}
