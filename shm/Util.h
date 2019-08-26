#ifndef __SHM_UTIL_H
#define __SHM_UTIL_H
#include <cstdlib>
#include <cassert>
typedef struct list {
	int val;
	struct list *next;
} List_t;

bool list_empty(List_t *list) {
	return list->next == NULL;
}

int list_front(List_t *list) {
	return list->next->val;
}

void list_push(List_t *list, int val) {
	List_t *to_insert= (List_t *)malloc(sizeof(List_t));
	assert(to_insert);
	to_insert->val = val;
	to_insert->next = list->next;
	list->next = to_insert;
}

void list_pop(List_t *list) {
	List_t *to_remove = list->next;
	list->next = to_remove->next;
	free(to_remove);
}

#endif
