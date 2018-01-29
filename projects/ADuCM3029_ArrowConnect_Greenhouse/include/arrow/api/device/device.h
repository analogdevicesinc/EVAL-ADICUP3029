#if !defined(ARROW_DEVICE_API_H_)
#define ARROW_DEVICE_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/find_by.h>
#include <arrow/gateway.h>
#include <arrow/device.h>

// create or update this device
int arrow_register_device(arrow_gateway_t *gateway, arrow_device_t *device);
// find device information by some parameters, 'n' is the number of the find arguments
int arrow_device_find_by(int n, ...);
// find device information by HID
int arrow_device_find_by_hid(const char *hid);
// update existing device
int arrow_update_device(arrow_gateway_t *gateway, arrow_device_t *device);
// list historical device events
int arrow_list_device_events(arrow_device_t *device, int n, ...);
// list device audit logs
int arrow_list_device_logs(arrow_device_t *device, int n, ...);
// error request
int arrow_error_device(arrow_device_t *device, const char *error);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_DEVICE_API_H_
