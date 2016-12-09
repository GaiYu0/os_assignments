BIN=bin
INC=inc
OBJ=obj
SRC=src

all: clean client server
array:
	gcc -o ${OBJ}/array ${SRC}/dynamic_array.c -c -I${INC}
file_lock:
	gcc -o ${OBJ}/file_lock ${SRC}/file_lock.c -c -I${INC} -lpthread
utility:
	gcc -o ${OBJ}/utility ${SRC}/utility.c -c -I${INC}
client: utility
	gcc -o ${OBJ}/client ${SRC}/client.c -c -I${INC} -lpthread
	gcc -o ${BIN}/client ${OBJ}/client ${OBJ}/utility -I${INC} -lpthread
server:	clean array utility
	gcc -o ${OBJ}/server ${SRC}/server.c -c -lpthread -I${INC} -D_REENTRANT
	gcc -o ${BIN}/server ${OBJ}/server ${OBJ}/array ${OBJ}/utility -lpthread -I${INC}
test: array file_lock
	gcc -o ${OBJ}/test test.c -c -I${INC} -lpthread
	gcc -o a.out ${OBJ}/test ${OBJ}/array ${OBJ}/file_lock -lpthread
clean:
	rm -f ${BIN}/* ${OBJ}/*
