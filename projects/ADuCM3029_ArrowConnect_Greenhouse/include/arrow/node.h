#if !defined(ARROW_NODE_H_)
#define ARROW_NODE_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _arrow_node {
  char *description;
  int enabled;
  char *name;
  char *nodeTypeHid;
  char *parentNodeHid;
  char *hid;
} arrow_node_t;

// list existing nodes
int arrow_node_list(void);
// create new node
int arrow_node_create(arrow_node_t *node);
// update existing node
int arrow_node_update(arrow_node_t *node);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_NODE_H_
