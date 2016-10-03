#include <list.h>

typedef struct list_type queue_type;

queue_type q;

int init_queue(){
	init_list(&q);
	return 0;
}

int queue(void *element){
	insert_head(&q, element);
	return 0;
}

void* dequeue(){
	return extract_tail(&q);
}

int size(){
	return list_size(&q);
}
