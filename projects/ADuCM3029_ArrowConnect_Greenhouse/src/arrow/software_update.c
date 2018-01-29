#include "arrow/software_update.h"
#include <debug.h>
#include <arrow/events.h>

#if defined(NO_SOFTWARE_UPDATE)
typedef void __dummy__;
#else
__update_cb __attribute__((weak)) __update;

int ev_GatewaySoftwareUpdate(void *_ev, JsonNode *_parameters) {
  SSP_PARAMETER_NOT_USED(_ev);
  DBG("start software update processing");
//  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  JsonNode *tmp = json_find_member(_parameters, "url");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  DBG("update url: %s", tmp->string_);

  return arrow_gateway_software_update(tmp->string_);
}

int __attribute__((weak)) arrow_gateway_software_update(const char *url) {
  if ( __update ) return __update(url);
  return -1;
}

int arrow_gateway_software_update_set_cb(__update_cb cb) {
  __update = cb;
  return 0;
}
#endif
