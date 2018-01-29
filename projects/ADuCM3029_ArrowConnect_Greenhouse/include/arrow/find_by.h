#if !defined(ARROW_FIND_BY_H_)
#define ARROW_FIND_BY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdarg.h>

enum FindBy {
  f_userHid = 0,
  f_uid,
  f_type,
  f_gatewayHid,
  f_createdBefore,
  f_createdAfter,
  f_updatedBefore,
  f_updatedAfter,
  f_enabled,
  f_page,
  f_size,
  createdDateFrom,
  createdDateTo,
  sortField,
  sortDirection,
  statuses,
  systemNames,
  // telemetry
  fromTimestamp,
  toTimestamp,
  telemetryNames,
  FindBy_count
};

typedef struct _find_by {
  int key;
  const char *value;
  struct _find_by *next;
#if defined(__cplusplus)
  _find_by(int k, const char *val) : key(k), value(val), next(NULL) {}
#endif
} find_by_t;

#if defined(__cplusplus)
#define find_by(x, y) find_by_t(x, (const char*)y)
#else
#define find_by(x, y) (find_by_t){ .key=x, .name=y, .next=NULL }
#endif

const char *get_find_by_name(int num);
void add_find_param(find_by_t **first, find_by_t *val);

#define COLLECT_FIND_BY(params, n) \
  do { \
    find_by_t val; \
    va_list args; \
    va_start(args, n); \
    int i = 0; \
    for (i=0; i < n; i++) { \
      val = va_arg(args, find_by_t); \
      add_find_param(&params, &val); \
    } \
    va_end(args); \
  } while(0)

// FIXME if value is property, rm p_stack
#define ADD_FIND_BY_TO_REQ(params, request) \
  if ( params ) { \
    do { \
      if ( params->key < FindBy_count ) { \
        http_request_add_query(request, \
                p_const(get_find_by_name(params->key)), \
                p_stack(params->value)); \
      } \
      params = params->next; \
    } while( params ); \
  }

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_FIND_BY_H_
