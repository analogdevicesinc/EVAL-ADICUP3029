#if !defined(ARROW_TELEMETRY_API_H_)
#define ARROW_TELEMETRY_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/device.h>

typedef struct _telemetry_data_info {
  char *deviceHid;
  char *name;
  char *type;
  time_t timestamp;
  int floatValue;
  struct _telemetry_data_info *next;
} telemetry_data_info_t;

typedef struct _telemetry_response_data_list_ {
  int size;
  int page;
  int totalSize;
  int totalPages;
  telemetry_data_info_t *data;
} telemetry_response_data_list_t;

// initialize the data list object for telemetry requests
int telemetry_response_data_list_init(telemetry_response_data_list_t *data,
                                      int size,
                                      int page,
                                      int tsize,
                                      int tpage);
// terminate the data list object
int telemetry_response_data_list_free(telemetry_response_data_list_t *data);

// add this information into a data list object
int add_telemetry_data_info(telemetry_response_data_list_t *data,
                            const char *deviceHid,
                            const char *name,
                            const char *type,
                            time_t timestamp,
                            int flval);

// create telemetry data to the cloud
int arrow_send_telemetry(arrow_device_t *device, void *data);
// create telemetry data batch
int arrow_telemetry_batch_create(arrow_device_t *device, void *data, int size);
// find telemetry data by an application hid
int arrow_telemetry_find_by_application_hid(const char *hid, int n, ...);
// find telemetry data by a device hid
int arrow_telemetry_find_by_device_hid(const char *hid, telemetry_response_data_list_t *data, int n, ...);
// telemetry data by a node hid
int arrow_telemetry_find_by_node_hid(const char *hid, int n, ...);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_TELEMETRY_API_H_
