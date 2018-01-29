#if !defined(ARROW_EVENTS_H_)
#define ARROW_EVENTS_H_

typedef struct {
  char *gateway_hid;
  char *device_hid;
  char *name;
  int encrypted;
  char *cmd;
  char *payload;
} mqtt_event_t;

int process_event(const char *str);

#define MAX_PARAM_LINE 20
#define MAX_PARAM_LINE_SIZE 256

#endif // ARROW_EVENTS_H_
