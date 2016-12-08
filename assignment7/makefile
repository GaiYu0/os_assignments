BIN=bin
INC=inc
OBJ=obj
SRC=src

all: clean client server
array:
	gcc -o ${OBJ}/array ${SRC}/dynamic_array.c -c -I${INC}
utility:
	gcc -o ${OBJ}/utility ${SRC}/utility.c -c -I${INC}
client: utility
	gcc -o ${OBJ}/client ${SRC}/client.c -c -I${INC}
	gcc -o ${BIN}/client ${OBJ}/client ${OBJ}/utility -I${INC}
server:	clean array utility
	gcc -o ${OBJ}/server ${SRC}/server.c -c -lpthread -I${INC} -D_REENTRANT
	gcc -o ${BIN}/server ${OBJ}/server ${OBJ}/array ${OBJ}/utility -lpthread -I${INC}
clean:
	rm -f ${BIN}/* ${OBJ}/*
