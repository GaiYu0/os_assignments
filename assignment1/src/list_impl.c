#include <list.h>
#include <stdlib.h>
#include <stdio.h>


void init_list(struct list_type *l) {
	l->head = NULL;
	l->tail = NULL;
}
	
	
void insert_head(struct list_type *l, void* element) {
	cell* new_cell = malloc(sizeof(cell));
	new_cell->content = element;
	new_cell->previous = NULL;
	new_cell->next = l->head;
	if (l->head != NULL)
		l->head->previous = new_cell;
	l->head = new_cell;
	if (l->tail == NULL)
			l->tail = l->head;
}


void* extract_head(struct list_type *l) {
	/* TODO */
	if (l->head == NULL)
		return NULL;
	cell *head = l->head;
	l->head = head->next;
	void *content = head->content;
	free(head);
	return content;
}


void* extract_tail(struct list_type *l) {
	/* TODO */
	if (l->tail == NULL)
		return NULL;
	cell *tail = l->tail;
	l->tail = tail->previous;
	void *content = tail->content;
	free(tail);
	return content;
}


int list_size(struct list_type *l) {
	/* TODO */
	if (l->head == NULL) return 0;
	if (l->head == l->tail) return 1;
	int size;
	cell *iterator;
	for(size=1, iterator=l->head; iterator->next != l->tail; ++size, iterator = iterator->next);
	return size + 1;
}
