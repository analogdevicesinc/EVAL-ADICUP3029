#if !defined(ARROW_DEVICE_COMMAND_H_)
#define ARROW_DEVICE_COMMAND_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <json/json.h>

typedef enum {
    failed,
    received,
    succeeded
} cmd_type;

typedef enum {
    CMD_OK,
    CMD_NO_HANDLER = -1,
    CMD_ERROR = -2
} handler_result;


typedef int (*fp)(const char *);

typedef struct __cmd_handler {
  char *name;
  fp callback;
  struct __cmd_handler *next;
} cmd_handler;

// Find and execute 'name' command handler for this 'payload'
int command_handler(const char *name,
                    JsonNode *payload,
                    JsonNode **error);

// Is there any command handler was added
int has_cmd_handler(void);

// Add a new command handler ( set the callback )
int add_cmd_handler(const char *name, fp callback);

// erase all command handlers
void free_cmd_handler(void);

// DeviceCommand event handler
int ev_DeviceCommand(void *ev, JsonNode *node);

// send the answer by DeviceCommand: failed, received or succeeded (cmd_type)
int arrow_send_event_ans(const char *hid, cmd_type ev, const char *payload);

#if defined(__cplusplus)
}
#endif

#endif // ARROW_DEVICE_COMMAND_H_
