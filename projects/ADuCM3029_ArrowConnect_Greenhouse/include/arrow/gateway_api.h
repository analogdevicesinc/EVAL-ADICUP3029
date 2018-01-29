#if !defined(ARROW_GATEWAY_API_H_)
#define ARROW_GATEWAY_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/find_by.h>
#include <arrow/gateway.h>
#include <arrow/device.h>

// register new gateway
int arrow_register_gateway(arrow_gateway_t *gateway);
// download gateway configuration
int arrow_gateway_config(arrow_gateway_t *gateway, arrow_gateway_config_t *config);
// send the heartbeat request
int arrow_gateway_heartbeat(arrow_gateway_t *gateway);
// send the checkin request
int arrow_gateway_checkin(arrow_gateway_t *gateway);
// find gateway by hid
int arrow_gateway_find(const char *hid);
// find gateway by other any parameters
int arrow_gateway_find_by(int n, ...);
// list gateway audit logs
int arrow_gateway_logs_list(arrow_gateway_t *gateway, int n, ...);
// list gateway devices
int arrow_gateway_devices_list(const char *hid);
// send command and payload to gateway and device
int arrow_gateway_device_send_command(const char *gHid, const char *dHid, const char *cmd, const char *payload);
// update existing gateway
int arrow_gateway_update(arrow_gateway_t *gateway);
// send the error request
int arrow_gateway_error(arrow_gateway_t *gateway, const char *error);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_GATEWAY_API_H_
