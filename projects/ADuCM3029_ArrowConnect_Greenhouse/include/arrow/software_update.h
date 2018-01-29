#if !defined(ARROW_SOFTWARE_UPDATE_H_)
#define ARROW_SOFTWARE_UPDATE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <json/json.h>

typedef int (*__update_cb)(const char *url);

// GatewaySoftwareUpdate event handler
int ev_GatewaySoftwareUpdate(void *_ev, JsonNode *_parameters);

// weak function just execute the update callback
// it's possible to reimplement this function in your own space
int arrow_gateway_software_update(const char *url);

// set the callback; url pass as an argument
int arrow_gateway_software_update_set_cb(__update_cb);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_SOFTWARE_UPDATE_H_
