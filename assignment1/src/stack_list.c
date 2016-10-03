#include <list.h>
#include <stack.h>

typedef struct list_type stack_type;

stack_type s;

int init_stack(){
	init_list(&s);
	return 0;
}

int push(void* element){
	insert_head(&s, element);
	return 0;
}

void* pop(){
	return extract_head(&s);
}

int size(){
	return list_size(&s);
}
