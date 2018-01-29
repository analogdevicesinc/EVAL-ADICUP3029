#if !defined(ARROW_NODE_TYPE_H_)
#define ARROW_NODE_TYPE_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _arrow_node_type {
  char *description;
  int enabled;
  char *name;
  char *hid;
} arrow_node_type_t;

// list existing node types
int arrow_node_type_list(void);
// create new node type
int arrow_node_type_create(arrow_node_type_t *node);
// update existing node type
int arrow_node_type_update(arrow_node_type_t *node);


#if defined(__cplusplus)
}
#endif

#endif  // ARROW_NODE_TYPE_H_
