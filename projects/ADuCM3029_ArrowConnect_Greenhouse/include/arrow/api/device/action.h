#if !defined(ARROW_DEVICE_ACTION_H_)
#define ARROW_DEVICE_ACTION_H_

#include <arrow/device.h>

typedef struct _dev_action_model {
  char *criteria;
  char *description;
  int enabled;
  int expiration;
  int index;
  char *systemName;
} dev_action_model_t;

// create a new device action for a specific device type
int arrow_create_device_action(arrow_device_t *dev, dev_action_model_t *model);
// delete a device action from a specific device type
int arrow_delete_device_action(arrow_device_t *dev, dev_action_model_t *model);
// list existing device actions for a device
int arrow_list_device_action(arrow_device_t *dev);
// list available action types
int arrow_list_action_type(void);
// update an existing device action for a specific device type
int arrow_update_device_action(arrow_device_t *dev, dev_action_model_t *model);

#endif  // ARROW_DEVICE_ACTION_H_
