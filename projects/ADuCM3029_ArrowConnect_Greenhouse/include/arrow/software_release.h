#if !defined(ARROW_SOFTWARE_RELEASE_H_)
#define ARROW_SOFTWARE_RELEASE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/gateway.h>
#include <arrow/device.h>

typedef int (*__release_cb)(const char *url,
                           const char *chsum,
                           const char *from,
                           const char *to);

typedef int (*__download_payload_cb)(property_t *,const char *,int);
typedef int (*__download_complete_cb)(property_t *);

typedef struct _release_sched_ {
  property_t trans_hid;
  const char *schedule_hid;
  const char *release_hid;
} release_sched_t;

void create_release_schedule(release_sched_t *rs, const char *shed_hid, const char *rel_hid);
void free_release_schedule(release_sched_t *rs);

int arrow_gateway_software_releases_trans(arrow_gateway_t *gate, release_sched_t *rs);
int arrow_device_software_releases_trans(arrow_device_t *gate, release_sched_t *rs);

// mark software release transaction as failed
int arrow_software_releases_trans_fail(const char *hid, const char *error);
// mark software release transaction as received
int arrow_software_releases_trans_received(const char *hid);
// mark software release transaction as succeeded
int arrow_software_releases_trans_success(const char *hid);

// start software release transaction
int arrow_software_releases_trans_start(const char *hid);

// DeviceSoftwareRelease event handler
int ev_DeviceSoftwareRelease(void *_ev, JsonNode *_parameters);

int arrow_software_release_download(const char *token, const char *tr_hid);

// set software release download callback
// will be executed when download complete
int arrow_software_release_dowload_set_cb(__download_payload_cb pcb, __download_complete_cb ccb);

int arrow_software_release(const char *token,
                           const char *chsum,
                           const char *from,
                           const char *to);

int arrow_software_release_set_cb(__release_cb cb);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_SOFTWARE_RELEASE_H_
