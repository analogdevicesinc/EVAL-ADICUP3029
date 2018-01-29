#include "arrow/find_by.h"
#include <arrow/mem.h>

static const char *find_by_name[] = {
  "userHid",
  "uid",
  "type",
  "gatewayHid",
  "createdBefore",
  "createdAfter",
  "updatedBefore",
  "updatedAfter",
  "enabled",
  "_page",
  "_size",
  "createdDateFrom",
  "createdDateTo",
  "sortField",
  "sortDirection",
  "statuses",
  "systemNames",
  "fromTimestamp",
  "toTimestamp",
  "telemetryNames"
};

const char *get_find_by_name(int num) {
  if ( num < FindBy_count ) {
    return find_by_name[num];
  }
  return 0;
}

void add_find_param(find_by_t **first, find_by_t *val) {
  find_by_t *tmp = malloc(sizeof(find_by_t));
  tmp->key = val->key;
  tmp->value = val->value;
  tmp->next = val->next;
  if ( ! *first ) *first = tmp;
  else {
    find_by_t *t_first = *first;
    while( t_first->next ) t_first = t_first->next;
    t_first->next = tmp;
  }
}
