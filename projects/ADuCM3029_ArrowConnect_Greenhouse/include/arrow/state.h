#if !defined(ARROW_STATE_H_)
#define ARROW_STATE_H_

#include <arrow/device.h>

// set a new device state
void add_state(const char *name, const char *value);

// get device state information
int arrow_get_state(arrow_device_t *device);
int arrow_post_state_request(arrow_device_t *device);
int arrow_post_state_update(arrow_device_t *device);

// allow to use
int arrow_state_mqtt_run(arrow_device_t *device);
int arrow_state_mqtt_is_running(void);
int arrow_state_mqtt_stop(void);

// DeviceStateRequest event handler
int ev_DeviceStateRequest(void *, JsonNode *_parameters);

int state_handler(char *str);

#endif // ARROW_STATE_H_
